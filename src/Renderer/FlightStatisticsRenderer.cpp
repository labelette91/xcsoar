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

#include "FlightStatisticsRenderer.hpp"
#include "FlightStatistics.hpp"
#include "Util/Macros.hpp"
#include "Look/MapLook.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Fonts.hpp"
#include "Screen/Graphics.hpp"
#include "Screen/Layout.hpp"
#include "Math/FastMath.h"
#include "Math/Earth.hpp"
#include "Math/Constants.h"
#include "NMEA/Info.hpp"
#include "NMEA/Derived.hpp"
#include "Units/UnitsFormatter.hpp"
#include "Units/Units.hpp"
#include "Terrain/RasterTerrain.hpp"
#include "Wind/WindStore.hpp"
#include "Language/Language.hpp"
#include "ComputerSettings.hpp"
#include "MapSettings.hpp"
#include "Navigation/Geometry/GeoVector.hpp"
#include "Task/TaskPoints/AATPoint.hpp"
#include "Task/TaskPoints/ASTPoint.hpp"
#include "GlideSolvers/GlidePolar.hpp"
#include "Projection/ChartProjection.hpp"
#include "Renderer/TaskRenderer.hpp"
#include "Renderer/RenderTaskPoint.hpp"
#include "Renderer/OZRenderer.hpp"
#include "Renderer/AircraftRenderer.hpp"
#include "Screen/Chart.hpp"
#include "Computer/TraceComputer.hpp"

#include <algorithm>

#include <stdio.h>

using std::min;
using std::max;

FlightStatisticsRenderer::FlightStatisticsRenderer(const FlightStatistics &_flight_statistics,
                                                   const ChartLook &_chart_look,
                                                   const MapLook &_map_look)
  :fs(_flight_statistics),
   chart_look(_chart_look),
   map_look(_map_look),
   trail_renderer(map_look.trail) {}

static bool
IsTaskLegVisible(const OrderedTaskPoint &tp)
{
  switch (tp.GetType()) {
  case TaskPoint::START:
    return tp.HasExited();

  case TaskPoint::FINISH:
  case TaskPoint::AAT:
  case TaskPoint::AST:
    return tp.HasEntered();

  case TaskPoint::UNORDERED:
  case TaskPoint::ROUTE:
    break;
  }

  /* not reachable */
  assert(false);
  return false;
}

static void DrawLegs(Chart& chart,
                     const TaskManager &task_manager,
                     const NMEAInfo& basic,
                     const DerivedInfo& calculated,
                     const bool task_relative)
{
  if (!calculated.common_stats.task_started)
    return;

  const fixed start_time = task_relative
    ? basic.time - calculated.common_stats.task_time_elapsed
    : calculated.flight.takeoff_time;

  const OrderedTask &task = task_manager.GetOrderedTask();
  for (unsigned i = 0, n = task.TaskSize(); i < n; ++i) {
    const OrderedTaskPoint &tp = *task.GetTaskPoint(i);
    if (!IsTaskLegVisible(tp))
      continue;

    fixed x = tp.GetEnteredState().time - start_time;
    if (!negative(x)) {
      x /= 3600;
      chart.DrawLine(x, chart.getYmin(), x, chart.getYmax(),
                     ChartLook::STYLE_REDTHICK);
    }
  }
}

void
FlightStatisticsRenderer::RenderBarographSpark(Canvas &canvas,
                                               const PixelRect rc,
                                               bool inverse,
                                               const NMEAInfo &nmea_info,
    const DerivedInfo &derived_info, const ProtectedTaskManager *_task) const
{
  ScopeLock lock(fs.mutexStats);
  Chart chart(chart_look, canvas, rc);
  chart.PaddingBottom = 0;
  chart.PaddingLeft = 0;

  if (fs.Altitude.sum_n < 2)
    return;

  chart.ScaleXFromData(fs.Altitude);
  chart.ScaleYFromData(fs.Altitude);
  chart.ScaleYFromValue(fixed_zero);

  if (_task != NULL) {
    ProtectedTaskManager::Lease task(*_task);
    canvas.SelectHollowBrush();
    DrawLegs(chart, task, nmea_info, derived_info, false);
  }

  canvas.SelectNullPen();
  canvas.Select(Graphics::hbGround);

  chart.DrawFilledLineGraph(fs.Altitude_Terrain);

  Pen pen(2, inverse ? COLOR_WHITE : COLOR_BLACK);
  chart.DrawLineGraph(fs.Altitude, pen);
}

void
FlightStatisticsRenderer::RenderBarograph(Canvas &canvas, const PixelRect rc,
                                  const NMEAInfo &nmea_info,
                                  const DerivedInfo &derived_info,
                                  const ProtectedTaskManager *_task) const
{
  Chart chart(chart_look, canvas, rc);

  if (fs.Altitude.sum_n < 2) {
    chart.DrawNoData();
    return;
  }

  chart.ScaleXFromData(fs.Altitude);
  chart.ScaleYFromData(fs.Altitude);
  chart.ScaleYFromValue(fixed_zero);
  chart.ScaleXFromValue(fs.Altitude.x_min + fixed_one); // in case no data
  chart.ScaleXFromValue(fs.Altitude.x_min);

  if (_task != NULL) {
    ProtectedTaskManager::Lease task(*_task);
    DrawLegs(chart, task, nmea_info, derived_info, false);
  }

  canvas.SelectNullPen();
  canvas.Select(Graphics::hbGround);

  chart.DrawFilledLineGraph(fs.Altitude_Terrain);
  canvas.SelectWhitePen();
  canvas.SelectWhiteBrush();

  chart.DrawXGrid(fixed_half, fs.Altitude.x_min,
                  ChartLook::STYLE_THINDASHPAPER,
                  fixed_half, true);
  chart.DrawYGrid(Units::ToSysAltitude(fixed(1000)),
                  fixed_zero, ChartLook::STYLE_THINDASHPAPER, fixed(1000), true);
  chart.DrawLineGraph(fs.Altitude, ChartLook::STYLE_MEDIUMBLACK);

  chart.DrawTrend(fs.Altitude_Base, ChartLook::STYLE_BLUETHIN);
  chart.DrawTrend(fs.Altitude_Ceiling, ChartLook::STYLE_BLUETHIN);

  chart.DrawXLabel(_T("t (hr)"));
  chart.DrawYLabel(_T("h"));
}

void
FlightStatisticsRenderer::RenderSpeed(Canvas &canvas, const PixelRect rc,
                              const NMEAInfo &nmea_info,
                              const DerivedInfo &derived_info,
                              const TaskManager &task) const
{
  Chart chart(chart_look, canvas, rc);

  if ((fs.Task_Speed.sum_n < 2) || !task.CheckOrderedTask()) {
    chart.DrawNoData();
    return;
  }

  chart.ScaleXFromData(fs.Task_Speed);
  chart.ScaleYFromData(fs.Task_Speed);
  chart.ScaleYFromValue(fixed_zero);
  chart.ScaleXFromValue(fs.Task_Speed.x_min + fixed_one); // in case no data
  chart.ScaleXFromValue(fs.Task_Speed.x_min);

  DrawLegs(chart, task, nmea_info, derived_info, true);

  chart.DrawXGrid(fixed_half, fs.Task_Speed.x_min,
                  ChartLook::STYLE_THINDASHPAPER, fixed_half, true);
  chart.DrawYGrid(Units::ToSysTaskSpeed(fixed_ten),
                  fixed_zero, ChartLook::STYLE_THINDASHPAPER, fixed(10), true);
  chart.DrawLineGraph(fs.Task_Speed, ChartLook::STYLE_MEDIUMBLACK);
  chart.DrawTrend(fs.Task_Speed, ChartLook::STYLE_BLUETHIN);

  chart.DrawXLabel(_T("t (hr)"));
  chart.DrawYLabel(_T("V"));
}

void
FlightStatisticsRenderer::RenderClimb(Canvas &canvas, const PixelRect rc,
                              const GlidePolar& glide_polar) const
{
  Chart chart(chart_look, canvas, rc);

  if (fs.ThermalAverage.sum_n < 1) {
    chart.DrawNoData();
    return;
  }

  fixed MACCREADY = glide_polar.GetMC();

  chart.ScaleYFromData(fs.ThermalAverage);
  chart.ScaleYFromValue(MACCREADY + fixed_half);
  chart.ScaleYFromValue(fixed_zero);

  chart.ScaleXFromValue(fixed_minus_one);
  chart.ScaleXFromValue(fixed(fs.ThermalAverage.sum_n));

  chart.DrawYGrid(Units::ToSysVSpeed(fixed_one), fixed_zero,
                  ChartLook::STYLE_THINDASHPAPER, fixed_one, true);
  chart.DrawBarChart(fs.ThermalAverage);

  chart.DrawLine(fixed_zero, MACCREADY, fixed(fs.ThermalAverage.sum_n), MACCREADY,
                 ChartLook::STYLE_REDTHICK);

  chart.DrawLabel(_T("MC"),
                  max(fixed_half, fixed(fs.ThermalAverage.sum_n) - fixed_one),
                  MACCREADY);

  chart.DrawTrendN(fs.ThermalAverage, ChartLook::STYLE_BLUETHIN);

  chart.DrawXLabel(_T("n"));
  chart.DrawYLabel(_T("w"));
}

void
FlightStatisticsRenderer::RenderGlidePolar(Canvas &canvas, const PixelRect rc,
                                           const ClimbHistory &climb_history,
                                   const ComputerSettings &settings_computer,
                                   const GlidePolar& glide_polar) const
{
  Chart chart(chart_look, canvas, rc);
  Pen blue_pen(2, COLOR_BLUE);

  chart.ScaleYFromValue(fixed_zero);
  chart.ScaleYFromValue(-glide_polar.GetSMax() * fixed(1.1));
  chart.ScaleXFromValue(glide_polar.GetVMin() * fixed(0.8));
  chart.ScaleXFromValue(glide_polar.GetVMax() + fixed_two);

  chart.DrawXGrid(Units::ToSysSpeed(fixed_ten), fixed_zero,
                  ChartLook::STYLE_THINDASHPAPER, fixed_ten, true);
  chart.DrawYGrid(Units::ToSysVSpeed(fixed_one),
                  fixed_zero, ChartLook::STYLE_THINDASHPAPER, fixed_one, true);

  fixed sinkrate0, sinkrate1;
  fixed v0 = fixed_zero, v1;
  bool v0valid = false;
  unsigned i0 = 0;

  const unsigned vmin = (unsigned)glide_polar.GetVMin();
  const unsigned vmax = (unsigned)glide_polar.GetVMax();
  for (unsigned i = vmin; i <= vmax; ++i) {
    sinkrate0 = -glide_polar.SinkRate(fixed(i));
    sinkrate1 = -glide_polar.SinkRate(fixed(i + 1));
    chart.DrawLine(fixed(i), sinkrate0, fixed(i + 1), sinkrate1,
                   ChartLook::STYLE_MEDIUMBLACK);

    if (climb_history.Check(i)) {
      v1 = climb_history.Get(i);

      if (v0valid)
        chart.DrawLine(fixed(i0), v0, fixed(i), v1, blue_pen);

      v0 = v1;
      i0 = i;
      v0valid = true;
    }
  }

  fixed MACCREADY = glide_polar.GetMC();
  fixed sb = -glide_polar.GetSBestLD();
  fixed ff = (sb - MACCREADY) / glide_polar.GetVBestLD();

  chart.DrawLine(fixed_zero, MACCREADY, glide_polar.GetVMax(),
                 MACCREADY + ff * glide_polar.GetVMax(),
                 ChartLook::STYLE_REDTHICK);

  chart.DrawXLabel(_T("V"));
  chart.DrawYLabel(_T("w"));

  StaticString<80> text;
  canvas.SetBackgroundTransparent();

  text.Format(_T("%s: %d kg"), _("Mass"),
              (int)glide_polar.GetTotalMass());
  canvas.text(rc.left + Layout::Scale(30), rc.bottom - Layout::Scale(55), text);

  fixed wl = glide_polar.GetWingLoading();
  if ( wl != fixed_zero )
  {
    text.Format(_T("%s: %.1f kg/m2"), _("Wing loading"), (double)wl);

    canvas.text(rc.left + Layout::Scale(30), rc.bottom - Layout::Scale(40), text);
  }
}

void
FlightStatisticsRenderer::RenderOLC(Canvas &canvas, const PixelRect rc,
                            const NMEAInfo &nmea_info, 
                            const DerivedInfo &calculated,
                            const ComputerSettings &settings_computer,
                            const MapSettings &settings_map,
                            const ContestStatistics &contest,
                                    const TraceComputer &trace_computer) const
{
  if (!trail_renderer.LoadTrace(trace_computer)) {
    Chart chart(chart_look, canvas, rc);
    chart.DrawNoData();
    return;
  }

  ChartProjection proj(rc, trail_renderer.GetBounds(nmea_info.location));

  RasterPoint aircraft_pos = proj.GeoToScreen(nmea_info.location);
  AircraftRenderer::Draw(canvas, settings_map, map_look.aircraft,
                         calculated.heading, aircraft_pos);

  trail_renderer.Draw(canvas, proj);

  for (unsigned i=0; i< 3; ++i) {
    if (contest.GetResult(i).IsDefined()) {
      canvas.Select(map_look.contest_pens[i]);
      trail_renderer.Draw(canvas, proj, contest.GetSolution(i));
    }
  }
}

void
FlightStatisticsRenderer::CaptionOLC(TCHAR *sTmp,
                                     const TaskBehaviour &task_behaviour,
                                     const DerivedInfo &derived) const
{
  if (task_behaviour.contest == OLC_Plus) {
    const ContestResult& result =
        derived.contest_stats.GetResult(2);

    const ContestResult& result_classic =
        derived.contest_stats.GetResult(0);

    const ContestResult& result_fai =
        derived.contest_stats.GetResult(1);

    TCHAR timetext1[100];
    Units::TimeToTextHHMMSigned(timetext1, (int)result.time);
    TCHAR distance_classic[100];
    Units::FormatUserDistance(result_classic.distance, distance_classic, 100);
    TCHAR distance_fai[100];
    Units::FormatUserDistance(result_fai.distance, distance_fai, 100);
    TCHAR speed[100];
    Units::FormatUserTaskSpeed(result.speed, speed, ARRAY_SIZE(speed));
    _stprintf(sTmp,
              (Layout::landscape
               ? _T("%s:\r\n%s\r\n%s (FAI)\r\n%s:\r\n%.1f %s\r\n%s: %s\r\n%s: %s\r\n")
               : _T("%s: %s\r\n%s (FAI)\r\n%s: %.1f %s\r\n%s: %s\r\n%s: %s\r\n")),
              _("Distance"), distance_classic, distance_fai,
              _("Score"), (double)result.score, _("pts"),
              _("Time"), timetext1,
              _("Speed"), speed);
  } else if (task_behaviour.contest == OLC_DHVXC ||
             task_behaviour.contest == OLC_XContest) {
    const ContestResult& result_free =
        derived.contest_stats.GetResult(0);

    const ContestResult& result_triangle =
        derived.contest_stats.GetResult(1);

    TCHAR timetext1[100];
    Units::TimeToTextHHMMSigned(timetext1, (int)result_free.time);
    TCHAR distance[100];
    Units::FormatUserDistance(result_free.distance, distance, 100);
    TCHAR distance_fai[100];
    Units::FormatUserDistance(result_triangle.distance, distance_fai, 100);
    TCHAR speed[100];
    Units::FormatUserTaskSpeed(result_free.speed, speed, ARRAY_SIZE(speed));
    _stprintf(sTmp,
              (Layout::landscape
               ? _T("%s:\r\n%s (Free)\r\n%s (Triangle)\r\n%s:\r\n%.1f %s\r\n%s: %s\r\n%s: %s\r\n")
               : _T("%s: %s (Free)\r\n%s (Triangle)\r\n%s: %.1f %s\r\n%s: %s\r\n%s: %s\r\n")),
              _("Distance"), distance, distance_fai,
              _("Score"), (double)result_free.score, _("pts"),
              _("Time"), timetext1,
              _("Speed"), speed);
  } else {
    unsigned result_index;
    switch (task_behaviour.contest) {
      case OLC_League:
        result_index = 0;
        break;
      default:
        result_index = -1;
        break;
    }

    const ContestResult& result_olc =
        derived.contest_stats.GetResult(result_index);

    TCHAR timetext1[100];
    Units::TimeToTextHHMMSigned(timetext1, (int)result_olc.time);
    TCHAR distance[100];
    Units::FormatUserDistance(result_olc.distance, distance, 100);
    TCHAR speed[100];
    Units::FormatUserTaskSpeed(result_olc.speed, speed, ARRAY_SIZE(speed));
    _stprintf(sTmp,
              (Layout::landscape
               ? _T("%s:\r\n%s\r\n%s:\r\n%.1f %s\r\n%s: %s\r\n%s: %s\r\n")
               : _T("%s: %s\r\n%s: %.1f %s\r\n%s: %s\r\n%s: %s\r\n")),
              _("Distance"), distance,
              _("Score"), (double)result_olc.score, _("pts"),
              _("Time"), timetext1,
              _("Speed"), speed);
  }
}

void
FlightStatisticsRenderer::RenderTask(Canvas &canvas, const PixelRect rc,
                             const NMEAInfo &nmea_info, 
                             const DerivedInfo &calculated,
                             const ComputerSettings &settings_computer,
                             const MapSettings &settings_map,
                                     const ProtectedTaskManager &_task_manager,
                                     const TraceComputer *trace_computer) const
{
  Chart chart(chart_look, canvas, rc);

  ChartProjection proj;

  {
    ProtectedTaskManager::Lease task_manager(_task_manager);
    const OrderedTask &task = task_manager->GetOrderedTask();

    if (!task.CheckTask()) {
      chart.DrawNoData();
      return;
    }

    proj.Set(rc, task, nmea_info.location);

    OZRenderer ozv(map_look.task, map_look.airspace, settings_map.airspace);
    RenderTaskPoint tpv(canvas, proj, map_look.task,
                        task.GetTaskProjection(),
                        ozv, false, RenderTaskPoint::ALL, nmea_info.location);
    ::TaskRenderer dv(tpv, proj.GetScreenBounds());
    dv.Draw(task);
  }

  if (trace_computer != NULL)
    trail_renderer.Draw(canvas, *trace_computer, proj, 0);

  RasterPoint aircraft_pos = proj.GeoToScreen(nmea_info.location);
  AircraftRenderer::Draw(canvas, settings_map, map_look.aircraft,
                         calculated.heading, aircraft_pos);
}


void
FlightStatisticsRenderer::RenderWind(Canvas &canvas, const PixelRect rc,
                             const NMEAInfo &nmea_info,
                             const WindStore &wind_store) const
{
  int numsteps = 10;
  int i;
  fixed h;
  Vector wind;
  bool found = true;
  fixed mag;

  LeastSquares windstats_mag;
  Chart chart(chart_look, canvas, rc);

  if (fs.Altitude_Ceiling.y_max - fs.Altitude_Ceiling.y_min <= fixed_ten) {
    chart.DrawNoData();
    return;
  }

  for (i = 0; i < numsteps; i++) {
    h = fixed(fs.Altitude_Ceiling.y_max - fs.Altitude_Base.y_min) * i /
        (numsteps - 1) + fixed(fs.Altitude_Base.y_min);

    wind = wind_store.GetWind(nmea_info.time, h, found);
    mag = hypot(wind.x, wind.y);

    windstats_mag.LeastSquaresUpdate(mag, h);
  }

  chart.ScaleXFromData(windstats_mag);
  chart.ScaleXFromValue(fixed_zero);
  chart.ScaleXFromValue(fixed_ten);

  chart.ScaleYFromData(windstats_mag);

  chart.DrawXGrid(Units::ToSysSpeed(fixed(5)), fixed_zero,
                  ChartLook::STYLE_THINDASHPAPER, fixed(5), true);
  chart.DrawYGrid(Units::ToSysAltitude(fixed(1000)),
                  fixed_zero,
                  ChartLook::STYLE_THINDASHPAPER, fixed(1000), true);
  chart.DrawLineGraph(windstats_mag, ChartLook::STYLE_MEDIUMBLACK);

#define WINDVECTORMAG 25

  numsteps = (int)((rc.bottom - rc.top) / WINDVECTORMAG) - 1;

  // draw direction vectors
  fixed hfact;
  for (i = 0; i < numsteps; i++) {
    hfact = fixed(i + 1) / (numsteps + 1);
    h = fixed(fs.Altitude_Ceiling.y_max - fs.Altitude_Base.y_min) * hfact +
        fixed(fs.Altitude_Base.y_min);

    wind = wind_store.GetWind(nmea_info.time, h, found);
    if (windstats_mag.x_max == fixed_zero)
      windstats_mag.x_max = fixed_one; // prevent /0 problems
    wind.x /= fixed(windstats_mag.x_max);
    wind.y /= fixed(windstats_mag.x_max);
    mag = hypot(wind.x, wind.y);
    if (negative(mag))
      continue;

    Angle angle = Angle::Radians(atan2(-wind.x, wind.y));

    chart.DrawArrow((chart.getXmin() + chart.getXmax()) / 2, h,
                    mag * WINDVECTORMAG, angle, ChartLook::STYLE_MEDIUMBLACK);
  }

  chart.DrawXLabel(_T("w"));
  chart.DrawYLabel(_T("h"));
}

void
FlightStatisticsRenderer::CaptionBarograph(TCHAR *sTmp)
{
  ScopeLock lock(fs.mutexStats);
  if (fs.Altitude_Ceiling.sum_n < 2) {
    sTmp[0] = _T('\0');
  } else if (fs.Altitude_Ceiling.sum_n < 4) {
    _stprintf(sTmp, _T("%s:\r\n  %.0f-%.0f %s"),
              _("Working band"),
              (double)Units::ToUserAltitude(fixed(fs.Altitude_Base.y_ave)),
              (double)Units::ToUserAltitude(fixed(fs.Altitude_Ceiling.y_ave)),
              Units::GetAltitudeName());
  } else {
    _stprintf(sTmp, _T("%s:\r\n  %.0f-%.0f %s\r\n\r\n%s:\r\n  %.0f %s/hr"),
              _("Working band"),
              (double)Units::ToUserAltitude(fixed(fs.Altitude_Base.y_ave)),
              (double)Units::ToUserAltitude(fixed(fs.Altitude_Ceiling.y_ave)),
              Units::GetAltitudeName(),
              _("Ceiling trend"),
              (double)Units::ToUserAltitude(fixed(fs.Altitude_Ceiling.m)),
              Units::GetAltitudeName());
  }
}

void
FlightStatisticsRenderer::CaptionClimb(TCHAR* sTmp)
{
  ScopeLock lock(fs.mutexStats);
  if (fs.ThermalAverage.sum_n == 0) {
    sTmp[0] = _T('\0');
  } else if (fs.ThermalAverage.sum_n == 1) {
    _stprintf(sTmp, _T("%s:\r\n  %3.1f %s"),
              _("Avg. climb"),
              (double)Units::ToUserVSpeed(fixed(fs.ThermalAverage.y_ave)),
              Units::GetVerticalSpeedName());
  } else {
    _stprintf(sTmp, _T("%s:\r\n  %3.1f %s\r\n\r\n%s:\r\n  %3.2f %s"),
              _("Avg. climb"),
              (double)Units::ToUserVSpeed(fixed(fs.ThermalAverage.y_ave)),
              Units::GetVerticalSpeedName(),
              _("Climb trend"),
              (double)Units::ToUserVSpeed(fixed(fs.ThermalAverage.m)),
              Units::GetVerticalSpeedName());
  }
}

void
FlightStatisticsRenderer::CaptionPolar(TCHAR *sTmp, const GlidePolar& glide_polar) const
{
  _stprintf(sTmp, Layout::landscape ?
                  _T("%s:\r\n  %d\r\n  at %d %s\r\n\r\n%s:\r\n  %3.2f %s\r\n  at %d %s") :
                  _T("%s:\r\n  %d at %d %s\r\n%s:\r\n  %3.2f %s at %d %s"),
            _("Best L/D"),
            (int)glide_polar.GetBestLD(),
            (int)Units::ToUserSpeed(glide_polar.GetVBestLD()),
            Units::GetSpeedName(),
            _("Min. sink"),
            (double)Units::ToUserVSpeed(glide_polar.GetSMin()),
            Units::GetVerticalSpeedName(),
            (int)Units::ToUserSpeed(glide_polar.GetVMin()),
            Units::GetSpeedName());
}

void
FlightStatisticsRenderer::CaptionTask(TCHAR *sTmp, const DerivedInfo &derived) const
{
  const CommonStats &common = derived.common_stats;

  if (!common.ordered_valid ||
      !derived.task_stats.total.remaining.IsDefined()) {
    _tcscpy(sTmp, _("No task"));
  } else {
    const fixed d_remaining = derived.task_stats.total.remaining.get_distance();
    TCHAR timetext1[100];
    TCHAR timetext2[100];
    if (common.ordered_has_targets) {
      Units::TimeToTextHHMMSigned(timetext1, (int)common.task_time_remaining);
      Units::TimeToTextHHMMSigned(timetext2, (int)common.aat_time_remaining);

      if (Layout::landscape) {
        _stprintf(sTmp,
            _T("%s:\r\n  %s\r\n%s:\r\n  %s\r\n%s:\r\n  %5.0f %s\r\n%s:\r\n  %5.0f %s\r\n"),
            _("Task to go"), timetext1, _("AAT to go"), timetext2,
            _("Distance to go"),
            (double)Units::ToUserDistance(d_remaining),
            Units::GetDistanceName(), _("Target speed"),
            (double)Units::ToUserTaskSpeed(common.aat_speed_remaining),
            Units::GetTaskSpeedName());
      } else {
        _stprintf(sTmp,
            _T("%s: %s\r\n%s: %s\r\n%s: %5.0f %s\r\n%s: %5.0f %s\r\n"),
            _("Task to go"), timetext1, _("AAT to go"), timetext2,
            _("Distance to go"),
            (double)Units::ToUserDistance(d_remaining),
            Units::GetDistanceName(),
            _("Target speed"),
            (double)Units::ToUserTaskSpeed(common.aat_speed_remaining),
            Units::GetTaskSpeedName());
      }
    } else {
      Units::TimeToTextHHMMSigned(timetext1, (int)common.task_time_remaining);
      _stprintf(sTmp, _T("%s: %s\r\n%s: %5.0f %s\r\n"),
                _("Task to go"), timetext1, _("Distance to go"),
                (double)Units::ToUserDistance(d_remaining),
                Units::GetDistanceName());
    }
  }
}
