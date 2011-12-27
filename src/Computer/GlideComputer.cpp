/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "GlideComputer.hpp"
#include "ComputerSettings.hpp"
#include "NMEA/Info.hpp"
#include "NMEA/Derived.hpp"
#include "ConditionMonitor/ConditionMonitors.hpp"
#include "TeamCodeCalculation.hpp"
#include "PeriodClock.hpp"
#include "GlideComputerInterface.hpp"
#include "Input/InputQueue.hpp"
#include "ComputerSettings.hpp"
#include "Math/Earth.hpp"
#include "Logger/Logger.hpp"
#include "Engine/Waypoint/Waypoints.hpp"
#include "LocalTime.hpp"

static PeriodClock last_team_code_update;

/**
 * Constructor of the GlideComputer class
 * @return
 */
GlideComputer::GlideComputer(const Waypoints &_way_points,
                             Airspaces &_airspace_database,
                             ProtectedTaskManager &task,
                             GlideComputerTaskEvents& events):
  GlideComputerAirData(_way_points),
  GlideComputerTask(task, _airspace_database),
  warning_computer(_airspace_database),
  waypoints(_way_points),
  team_code_ref_id(-1)
{
  events.SetComputer(*this);
  idle_clock.update();
}

/**
 * Resets the GlideComputer data
 * @param full Reset all data?
 */
void
GlideComputer::ResetFlight(const bool full)
{
  GlideComputerBlackboard::ResetFlight(full);
  GlideComputerAirData::ResetFlight(full);
  GlideComputerTask::ResetFlight(full);
  GlideComputerStats::ResetFlight(full);

  cu_computer.Reset();
  warning_computer.Reset(Basic(), Calculated());
}

/**
 * Initializes the GlideComputer
 */
void
GlideComputer::Initialise()
{
  ResetFlight(true);
}

/**
 * Is called by the CalculationThread and processes the received GPS data in Basic()
 */
bool
GlideComputer::ProcessGPS()
{
  const MoreData &basic = Basic();
  DerivedInfo &calculated = SetCalculated();

  calculated.date_time_local = basic.date_time_utc + GetUTCOffset();

  calculated.Expire(basic.clock);

  // Process basic information
  ProcessBasic();

  // Process basic task information
  ProcessBasicTask(basic, LastBasic(),
                   calculated, LastCalculated(),
                   GetComputerSettings());
  ProcessMoreTask(basic, calculated, LastCalculated(),
                  GetComputerSettings());

  // Check if everything is okay with the gps time and process it
  if (!FlightTimes())
    return false;

  if (!time_retreated())
    GlideComputerTask::ProcessAutoTask(basic, calculated, LastCalculated());

  // Process extended information
  ProcessVertical();

  if (!time_retreated())
    GlideComputerStats::ProcessClimbEvents(calculated, LastCalculated());

  // Calculate the team code
  CalculateOwnTeamCode();

  // Calculate the bearing and range of the teammate
  CalculateTeammateBearingRange();

  // Calculate the bearing and range of the teammate
  // (if teammate is a FLARM target)
  CheckTraffic();

  vegavoice.Update(basic, Calculated(), GetComputerSettings());

  // update basic trace history
  if (time_advanced())
    calculated.trace_history.append(basic);

  // Update the ConditionMonitors
  ConditionMonitorsUpdate(*this);

  return idle_clock.check_update(500);
}

/**
 * Process slow calculations. Called by the CalculationThread.
 */
void
GlideComputer::ProcessIdle(bool exhaustive)
{
  // Log GPS fixes for internal usage
  // (snail trail, stats, olc, ...)
  DoLogging(Basic(), LastBasic(), Calculated(), GetComputerSettings());

  GlideComputerTask::ProcessIdle(Basic(), SetCalculated(), GetComputerSettings(),
                                 exhaustive);

  if (time_advanced())
    warning_computer.Update(GetComputerSettings(), Basic(), LastBasic(),
                            Calculated(), SetCalculated().airspace_warnings);
}

bool
GlideComputer::DetermineTeamCodeRefLocation()
{
  const ComputerSettings &settings_computer = GetComputerSettings();

  if (settings_computer.team_code_reference_waypoint < 0)
    return false;

  if (settings_computer.team_code_reference_waypoint == team_code_ref_id)
    return team_code_ref_found;

  team_code_ref_id = settings_computer.team_code_reference_waypoint;
  const Waypoint *wp = waypoints.LookupId(team_code_ref_id);
  if (wp == NULL)
    return team_code_ref_found = false;

  team_code_ref_location = wp->location;
  return team_code_ref_found = true;
}

/**
 * Calculates the own TeamCode and saves it to Calculated
 */
void
GlideComputer::CalculateOwnTeamCode()
{
  // No reference waypoint for teamcode calculation chosen -> cancel
  if (!DetermineTeamCodeRefLocation())
    return;

  // Only calculate every 10sec otherwise cancel calculation
  if (!last_team_code_update.check_update(10000))
    return;

  // Get bearing and distance to the reference waypoint
  const GeoVector v = team_code_ref_location.DistanceBearing(Basic().location);

  // Save teamcode to Calculated
  SetCalculated().own_teammate_code.Update(v.bearing, v.distance);
}

static void
ComputeFlarmTeam(const GeoPoint &location, const GeoPoint &reference_location,
                 const FlarmState &flarm, const FlarmId target_id,
                 TeamInfo &teamcode_info)
{
  if (!flarm.available) {
    teamcode_info.flarm_teammate_code_current = false;
    return;
  }

  const FlarmTraffic *traffic = flarm.FindTraffic(target_id);
  if (traffic == NULL || !traffic->location_available) {
    teamcode_info.flarm_teammate_code_current = false;
    return;
  }

  // Set Teammate location to FLARM contact location
  teamcode_info.teammate_location = traffic->location;
  teamcode_info.teammate_vector = location.DistanceBearing(traffic->location);
  teamcode_info.teammate_available = true;

  // Calculate distance and bearing from teammate to reference waypoint

  GeoVector v = reference_location.DistanceBearing(traffic->location);

  // Calculate TeamCode and save it in Calculated
  teamcode_info.flarm_teammate_code.Update(v.bearing, v.distance);
  teamcode_info.flarm_teammate_code_available = true;
  teamcode_info.flarm_teammate_code_current = true;
}

static void
ComputeTeamCode(const GeoPoint &location, const GeoPoint &reference_location,
                const TeamCode &team_code,
                TeamInfo &teamcode_info)
{
  // Calculate bearing and distance to teammate
  teamcode_info.teammate_location = team_code.GetLocation(reference_location);
  teamcode_info.teammate_vector =
    location.DistanceBearing(teamcode_info.teammate_location);
  teamcode_info.teammate_available = true;
}

void
GlideComputer::CalculateTeammateBearingRange()
{
  const ComputerSettings &settings_computer = GetComputerSettings();
  const NMEAInfo &basic = Basic();
  TeamInfo &teamcode_info = SetCalculated();

  // No reference waypoint for teamcode calculation chosen -> cancel
  if (!DetermineTeamCodeRefLocation())
    return;

  if (settings_computer.team_flarm_tracking) {
    ComputeFlarmTeam(basic.location, team_code_ref_location,
                     basic.flarm, settings_computer.team_flarm_id,
                     teamcode_info);
    CheckTeammateRange();
  } else if (settings_computer.team_code_valid) {
    teamcode_info.flarm_teammate_code_available = false;

    ComputeTeamCode(basic.location, team_code_ref_location,
                    settings_computer.team_code,
                    teamcode_info);
    CheckTeammateRange();
  } else {
    teamcode_info.teammate_available = false;
    teamcode_info.flarm_teammate_code_available = false;
  }
}

void
GlideComputer::CheckTeammateRange()
{
  static bool InTeamSector = false;

  // Hysteresis for GlideComputerEvent
  // If (closer than 100m to the teammates last position and "event" not reset)
  if (Calculated().teammate_vector.distance < fixed(100) &&
      InTeamSector == false) {
    InTeamSector = true;
    // Raise GCE_TEAM_POS_REACHED event
    InputEvents::processGlideComputer(GCE_TEAM_POS_REACHED);
  } else if (Calculated().teammate_vector.distance > fixed(300)) {
    // Reset "event" when distance is greater than 300m again
    InTeamSector = false;
  }
}

void
GlideComputer::OnTakeoff()
{
  GlideComputerAirData::OnTakeoff();
  InputEvents::processGlideComputer(GCE_TAKEOFF);
}

void
GlideComputer::OnLanding()
{
  GlideComputerAirData::OnLanding();
  InputEvents::processGlideComputer(GCE_LANDING);
}

void
GlideComputer::OnSwitchClimbMode(bool isclimb, bool left)
{
  GlideComputerAirData::OnSwitchClimbMode(isclimb, left);

  if (isclimb) {
    InputEvents::processGlideComputer(GCE_FLIGHTMODE_CLIMB);
  } else {
    InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);
  }
}

void
GlideComputer::CheckTraffic()
{
  const NMEAInfo &basic = Basic();
  const NMEAInfo &last_basic = LastBasic();

  if (!basic.flarm.available || !last_basic.flarm.available)
    return;

  if (basic.flarm.rx && last_basic.flarm.rx == 0)
    // traffic has appeared..
    InputEvents::processGlideComputer(GCE_FLARM_TRAFFIC);

  if (basic.flarm.rx == 0 && last_basic.flarm.rx)
    // traffic has disappeared..
    InputEvents::processGlideComputer(GCE_FLARM_NOTRAFFIC);

  if (basic.flarm.new_traffic)
    // new traffic has appeared
    InputEvents::processGlideComputer(GCE_FLARM_NEWTRAFFIC);
}

void 
GlideComputer::OnStartTask()
{
  GlideComputerBlackboard::StartTask();
  GlideComputerStats::StartTask();

  if (logger != NULL)
    logger->LogStartEvent(Basic());
}

void 
GlideComputer::OnFinishTask()
{
  SaveFinish();

  if (logger != NULL)
    logger->LogFinishEvent(Basic());
}

void
GlideComputer::OnTransitionEnter()
{
  GlideComputerStats::SetFastLogging();
}


void
GlideComputer::SetTerrain(RasterTerrain* _terrain)
{
  GlideComputerAirData::SetTerrain(_terrain);
  GlideComputerTask::SetTerrain(_terrain);
}
