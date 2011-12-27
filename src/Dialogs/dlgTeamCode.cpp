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

#include "Dialogs/Traffic.hpp"
#include "Dialogs/Internal.hpp"
#include "Dialogs/TextEntry.hpp"
#include "Dialogs/Waypoint.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "UIGlobals.hpp"
#include "DataField/Float.hpp"
#include "FLARM/FlarmDetails.hpp"
#include "ComputerSettings.hpp"
#include "Screen/Layout.hpp"
#include "DataField/Base.hpp"
#include "StringUtil.hpp"
#include "TeamCodeCalculation.hpp"
#include "Compiler.h"
#include "Profile/Profile.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Units/AngleFormatter.hpp"

#include <stdio.h>

static WndForm *wf = NULL;

static void
Update()
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const TeamInfo &teamcode_info = CommonInterface::Calculated();
  StaticString<100> buffer;

  if (teamcode_info.teammate_available && basic.track_available) {
    FormatAngleDelta(buffer.buffer(), buffer.MAX_SIZE,
                     teamcode_info.teammate_vector.bearing - basic.track);
  } else {
    buffer = _T("---");
  }

  SetFormValue(*wf, _T("prpRelBearing"), buffer);

  if (teamcode_info.teammate_available) {
    LoadFormProperty(*wf, _T("prpBearing"),
                     teamcode_info.teammate_vector.bearing.Degrees());
    LoadFormProperty(*wf, _T("prpRange"), ugDistance,
                     teamcode_info.teammate_vector.distance);
  }

  SetFormValue(*wf, _T("prpOwnCode"),
               teamcode_info.own_teammate_code.GetCode());
  SetFormValue(*wf, _T("prpMateCode"),
               CommonInterface::GetComputerSettings().team_code.GetCode());

  const TeamCodeSettings &settings = CommonInterface::GetComputerSettings();
  SetFormValue(*wf, _T("prpFlarmLock"),
               settings.team_flarm_tracking
               ? settings.team_flarm_callsign.c_str()
               : _T(""));
}

static void
OnSetWaypointClicked(gcc_unused WndButton &button)
{
  const Waypoint* wp = dlgWaypointSelect(UIGlobals::GetMainWindow(),
                                         XCSoarInterface::Basic().location);
  if (wp != NULL) {
    XCSoarInterface::SetComputerSettings().team_code_reference_waypoint = wp->id;
    Profile::Set(szProfileTeamcodeRefWaypoint, wp->id);
  }
}

static void
OnCodeClicked(gcc_unused WndButton &button)
{
  TCHAR newTeammateCode[10];

  CopyString(newTeammateCode,
             XCSoarInterface::GetComputerSettings().team_code.GetCode(), 10);

  if (!dlgTextEntryShowModal(*(SingleWindow *)button.get_root_owner(),
                             newTeammateCode, 7))
    return;

  TrimRight(newTeammateCode);

  XCSoarInterface::SetComputerSettings().team_code.Update(newTeammateCode);
  if (!string_is_empty(XCSoarInterface::GetComputerSettings().team_code.GetCode())) {
    XCSoarInterface::SetComputerSettings().team_code_valid = true;
    XCSoarInterface::SetComputerSettings().team_flarm_tracking = false;
  }
  else
    XCSoarInterface::SetComputerSettings().team_code_valid = false;
}

static void
OnFlarmLockClicked(gcc_unused WndButton &button)
{
  TeamCodeSettings &settings = CommonInterface::SetComputerSettings();
  TCHAR newTeamFlarmCNTarget[settings.team_flarm_callsign.MAX_SIZE];
  _tcscpy(newTeamFlarmCNTarget, settings.team_flarm_callsign.c_str());

  if (!dlgTextEntryShowModal(*(SingleWindow *)button.get_root_owner(),
                             newTeamFlarmCNTarget, 4))
    return;

  settings.team_flarm_callsign = newTeamFlarmCNTarget;
  settings.team_code_valid = false;

  if (string_is_empty(XCSoarInterface::GetComputerSettings().team_flarm_callsign)) {
    settings.team_flarm_tracking = false;
    settings.team_flarm_id.Clear();
    return;
  }

  const FlarmId *ids[30];
  unsigned count = FlarmDetails::FindIdsByCallSign(
      XCSoarInterface::GetComputerSettings().team_flarm_callsign, ids, 30);

  if (count > 0) {
    const FlarmId *id =
      dlgFlarmDetailsListShowModal(UIGlobals::GetMainWindow(),
                                   _("Set new teammate:"), ids, count);

    if (id != NULL && id->IsDefined()) {
      settings.team_flarm_id = *id;
      settings.team_flarm_tracking = true;
      return;
    }
  } else {
    MessageBoxX(_("Unknown Competition Number"),
                _("Not Found"), MB_OK | MB_ICONINFORMATION);
  }

  settings.team_flarm_tracking = false;
  settings.team_flarm_id.Clear();
  settings.team_flarm_callsign.clear();
}

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static void
OnTimerNotify(gcc_unused WndForm &Sender)
{
  Update();
}

static gcc_constexpr_data CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnFlarmLockClicked),
  DeclareCallBackEntry(OnCodeClicked),
  DeclareCallBackEntry(OnSetWaypointClicked),
  DeclareCallBackEntry(NULL)
};

void
dlgTeamCodeShowModal(void)
{
  wf = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
                  Layout::landscape ? _T("IDR_XML_TEAMCODE_L") :
                                      _T("IDR_XML_TEAMCODE"));

  if (!wf)
    return;

  Update();

  wf->SetTimerNotify(OnTimerNotify);

  wf->ShowModal();

  delete wf;
}
