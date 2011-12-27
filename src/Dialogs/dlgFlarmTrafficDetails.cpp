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

/**
 * @file
 * The FLARM Traffic Details dialog displaying extended information about
 * the FLARM targets from the FLARMnet database
 * @todo Button that opens the Waypoint details dialog of the
 * home airport (if found in FLARMnet and local waypoint database)
 */

#include "Dialogs/Traffic.hpp"
#include "Dialogs/Internal.hpp"
#include "Dialogs/TextEntry.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "FLARM/FlarmNet.hpp"
#include "FLARM/Traffic.hpp"
#include "FLARM/FlarmDetails.hpp"
#include "FLARM/Friends.hpp"
#include "Screen/Layout.hpp"
#include "Engine/Math/Earth.hpp"
#include "LocalPath.hpp"
#include "MainWindow.hpp"
#include "Components.hpp"
#include "Units/UnitsFormatter.hpp"
#include "Units/AngleFormatter.hpp"
#include "Util/StringUtil.hpp"
#include "Util/Macros.hpp"
#include "Language/Language.hpp"
#include "Compiler.h"

#include <math.h>
#include <stdio.h>

static WndForm *wf = NULL;
static FlarmId target_id;

/**
 * Updates all the dialogs fields, that are changing frequently.
 * e.g. climb speed, distance, height
 */
static void
UpdateChanging()
{
  TCHAR tmp[20];
  const FlarmTraffic* target =
      XCSoarInterface::Basic().flarm.FindTraffic(target_id);

  bool target_ok = target && target->IsDefined();

  // Fill distance field
  if (target_ok)
    Units::FormatUserDistance(target->distance, tmp, 20);
  else
    _tcscpy(tmp, _T("--"));
  ((WndProperty *)wf->FindByName(_T("prpDistance")))->SetText(tmp);

  // Fill horizontal direction field
  if (target_ok)
    FormatAngleDelta(tmp, ARRAY_SIZE(tmp),
                     target->Bearing() - CommonInterface::Basic().track);
  else
    _tcscpy(tmp, _T("--"));
  ((WndProperty *)wf->FindByName(_T("prpDirectionH")))->SetText(tmp);

  // Fill altitude field
  if (target_ok && target->altitude_available)
    Units::FormatUserAltitude(target->altitude, tmp, 20);
  else
    _tcscpy(tmp, _T("--"));
  ((WndProperty *)wf->FindByName(_T("prpAltitude")))->SetText(tmp);

  // Fill vertical direction field
  if (target_ok) {
    Angle dir = Angle::Radians((fixed)atan2(target->relative_altitude,
                                            target->distance)).AsDelta();
    FormatVerticalAngleDelta(tmp, ARRAY_SIZE(tmp), dir);
  } else
    _tcscpy(tmp, _T("--"));
  ((WndProperty *)wf->FindByName(_T("prpDirectionV")))->SetText(tmp);

  // Fill climb speed field
  if (target_ok && target->climb_rate_avg30s_available)
    Units::FormatUserVSpeed(target->climb_rate_avg30s, tmp, 20);
  else
    _tcscpy(tmp, _T("--"));
  ((WndProperty *)wf->FindByName(_T("prpVSpeed")))->SetText(tmp);
}

/**
 * Updates all the dialogs fields.
 * Should be called on dialog opening as it closes the dialog when the
 * target does not exist.
 */
static void
Update()
{
  TCHAR tmp[200], tmp_id[7];

  // Set the dialog caption
  _stprintf(tmp, _T("%s (%s)"),
            _("FLARM Traffic Details"), target_id.Format(tmp_id));
  wf->SetCaption(tmp);

  // Try to find the target in the FLARMnet database
  /// @todo: make this code a little more usable
  const FlarmNet::Record *record = FlarmDetails::LookupRecord(target_id);
  if (record) {
    // Fill the pilot name field
    _tcscpy(tmp, record->pilot);
    ((WndProperty *)wf->FindByName(_T("prpPilot")))->SetText(tmp);

    // Fill the frequency field
    _tcscpy(tmp, record->frequency);
    _tcscat(tmp, _T(" MHz"));
    ((WndProperty *)wf->FindByName(_T("prpFrequency")))->SetText(tmp);

    // Fill the home airfield field
    _tcscpy(tmp, record->airfield);
    ((WndProperty *)wf->FindByName(_T("prpAirport")))->SetText(tmp);

    // Fill the plane type field
    _tcscpy(tmp, record->plane_type);
    ((WndProperty *)wf->FindByName(_T("prpPlaneType")))->SetText(tmp);
  } else {
    // Fill the pilot name field
    ((WndProperty *)wf->FindByName(_T("prpPilot")))->SetText(_T("--"));

    // Fill the frequency field
    ((WndProperty *)wf->FindByName(_T("prpFrequency")))->SetText(_T("--"));

    // Fill the home airfield field
    ((WndProperty *)wf->FindByName(_T("prpAirport")))->SetText(_T("--"));

    // Fill the plane type field
    const FlarmTraffic* target =
        XCSoarInterface::Basic().flarm.FindTraffic(target_id);

    const TCHAR* actype;
    if (target == NULL ||
        (actype = FlarmTraffic::GetTypeString(target->type)) == NULL)
      actype = _T("--");

    ((WndProperty *)wf->FindByName(_T("prpPlaneType")))->SetText(actype);
  }

  // Fill the callsign field (+ registration)
  // note: don't use target->Name here since it is not updated
  //       yet if it was changed
  const TCHAR* cs = FlarmDetails::LookupCallsign(target_id);
  if (cs != NULL && cs[0] != 0) {
    _tcscpy(tmp, cs);
    if (record) {
      _tcscat(tmp, _T(" ("));
      _tcscat(tmp, record->registration);
      _tcscat(tmp, _T(")"));
    }
  } else
    _tcscpy(tmp, _T("--"));
  ((WndProperty *)wf->FindByName(_T("prpCallsign")))->SetText(tmp);

  // Update the frequently changing fields too
  UpdateChanging();
}

/**
 * This event handler is called when the timer is activated and triggers the
 * update of the variable fields of the dialog
 */
static void
OnTimerNotify(gcc_unused WndForm &Sender)
{
  UpdateChanging();
}

/**
 * This event handler is called when the "Close" button is pressed
 */
static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

/**
 * This event handler is called when the "Team" button is pressed
 */
static void
OnTeamClicked(gcc_unused WndButton &Sender)
{
  // Ask for confirmation
  if (MessageBoxX(_("Do you want to set this FLARM contact as your new teammate?"),
                  _("New Teammate"), MB_YESNO) != IDYES)
    return;

  TeamCodeSettings &settings = CommonInterface::SetComputerSettings();

  // Set the Teammate callsign
  const TCHAR *callsign = FlarmDetails::LookupCallsign(target_id);
  if (callsign == NULL || string_is_empty(callsign)) {
    settings.team_flarm_callsign.clear();
  } else {
    // copy the 3 first chars from the name
    settings.team_flarm_callsign = callsign;
  }

  // Start tracking
  settings.team_flarm_id = target_id;
  settings.team_flarm_tracking = true;
  settings.team_code_valid = false;

  // Close the dialog
  wf->SetModalResult(mrOK);
}

/**
 * This event handler is called when the "Change Callsign" button is pressed
 */
static void
OnCallsignClicked(gcc_unused WndButton &Sender)
{
  StaticString<21> newName;
  newName.clear();
  if (TextEntryDialog(XCSoarInterface::main_window, newName,
                      _("Competition ID")) &&
      FlarmDetails::AddSecondaryItem(target_id, newName))
    FlarmDetails::SaveSecondary();

  Update();
}

static void
OnFriendBlueClicked(gcc_unused WndButton &Sender)
{
  FlarmFriends::SetFriendColor(target_id, FlarmFriends::BLUE);
}

static void
OnFriendGreenClicked(gcc_unused WndButton &Sender)
{
  FlarmFriends::SetFriendColor(target_id, FlarmFriends::GREEN);
}

static void
OnFriendYellowClicked(gcc_unused WndButton &Sender)
{
  FlarmFriends::SetFriendColor(target_id, FlarmFriends::YELLOW);
}

static void
OnFriendMagentaClicked(gcc_unused WndButton &Sender)
{
  FlarmFriends::SetFriendColor(target_id, FlarmFriends::MAGENTA);
}

static void
OnFriendClearClicked(gcc_unused WndButton &Sender)
{
  FlarmFriends::SetFriendColor(target_id, FlarmFriends::NONE);
}

static gcc_constexpr_data CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnTeamClicked),
  DeclareCallBackEntry(OnCallsignClicked),
  DeclareCallBackEntry(OnFriendGreenClicked),
  DeclareCallBackEntry(OnFriendBlueClicked),
  DeclareCallBackEntry(OnFriendYellowClicked),
  DeclareCallBackEntry(OnFriendMagentaClicked),
  DeclareCallBackEntry(OnFriendClearClicked),
  DeclareCallBackEntry(NULL)
};

/**
 * The function opens the FLARM Traffic Details dialog
 */
void
dlgFlarmTrafficDetailsShowModal(FlarmId id)
{
  if (wf)
    return;

  target_id = id;

  // Load dialog from XML
  wf = LoadDialog(CallBackTable,
      XCSoarInterface::main_window, Layout::landscape ?
      _T("IDR_XML_FLARMTRAFFICDETAILS_L") : _T("IDR_XML_FLARMTRAFFICDETAILS"));
  assert(wf != NULL);

  // Set dialog events
  wf->SetTimerNotify(OnTimerNotify);

  // Update fields for the first time
  Update();

  // Show the dialog
  wf->ShowModal();

  // After dialog closed -> delete it
  delete wf;
  wf = NULL;
}
