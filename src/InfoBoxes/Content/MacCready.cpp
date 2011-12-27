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

#include "InfoBoxes/Content/MacCready.hpp"
#include "InfoBoxes/Data.hpp"
#include "InfoBoxes/Panel/MacCreadyEdit.hpp"
#include "InfoBoxes/Panel/MacCreadySetup.hpp"
#include "Units/UnitsFormatter.hpp"
#include "Interface.hpp"

#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "DeviceBlackboard.hpp"

#include "Dialogs/dlgInfoBoxAccess.hpp"
#include "Screen/Layout.hpp"
#include "Profile/Profile.hpp"
#include "Util/Macros.hpp"

#include <tchar.h>
#include <stdio.h>

static void
SetVSpeed(InfoBoxData &data, fixed value)
{
  TCHAR buffer[32];
  Units::FormatUserVSpeed(value, buffer, 32, false);
  data.SetValue(buffer[0] == _T('+') ? buffer + 1 : buffer);
  data.SetValueUnit(Units::current.vertical_speed_unit);
}

/*
 * Subpart callback function pointers
 */

static gcc_constexpr_data InfoBoxContentMacCready::PanelContent panels[] = {
  InfoBoxContentMacCready::PanelContent (
    N_("Edit"),
    LoadMacCreadyEditPanel),

  InfoBoxContentMacCready::PanelContent (
    N_("Setup"),
    LoadMacCreadySetupPanel),
};

const InfoBoxContentMacCready::DialogContent InfoBoxContentMacCready::dlgContent = {
  ARRAY_SIZE(panels), &panels[0],
};

const InfoBoxContentMacCready::DialogContent*
InfoBoxContentMacCready::GetDialogContent() {
  return &dlgContent;
}

/*
 * Subpart normal operations
 */

void
InfoBoxContentMacCready::Update(InfoBoxData &data)
{
  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();

  SetVSpeed(data, settings_computer.glide_polar_task.GetMC());

  // Set Comment
  if (XCSoarInterface::GetComputerSettings().task.auto_mc)
    data.SetComment(_("AUTO"));
  else
    data.SetComment(_("MANUAL"));
}

bool
InfoBoxContentMacCready::HandleKey(const InfoBoxKeyCodes keycode)
{
  if (protected_task_manager == NULL)
    return false;

  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();
  const GlidePolar &polar = settings_computer.glide_polar_task;
  TaskBehaviour &task_behaviour = CommonInterface::SetComputerSettings().task;
  fixed mc = polar.GetMC();

  switch (keycode) {
  case ibkUp:
    mc = std::min(mc + Units::ToSysVSpeed(fixed_one / 10), fixed(5));
    ActionInterface::SetMacCready(mc);
    task_behaviour.auto_mc = false;
    Profile::Set(szProfileAutoMc, false);
    return true;

  case ibkDown:
    mc = std::max(mc - Units::ToSysVSpeed(fixed_one / 10), fixed_zero);
    ActionInterface::SetMacCready(mc);
    task_behaviour.auto_mc = false;
    Profile::Set(szProfileAutoMc, false);
    return true;

  case ibkLeft:
    task_behaviour.auto_mc = false;
    Profile::Set(szProfileAutoMc, false);
    return true;

  case ibkRight:
    task_behaviour.auto_mc = true;
    Profile::Set(szProfileAutoMc, true);
    return true;

  case ibkEnter:
    task_behaviour.auto_mc = !task_behaviour.auto_mc;
    Profile::Set(szProfileAutoMc, task_behaviour.auto_mc);
    return true;
  }
  return false;
}

bool
InfoBoxContentMacCready::HandleQuickAccess(const TCHAR *misc)
{
  if (protected_task_manager == NULL)
    return false;

  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();
  const GlidePolar &polar = settings_computer.glide_polar_task;
  fixed mc = polar.GetMC();

  if (_tcscmp(misc, _T("+0.1")) == 0) {
    return HandleKey(ibkUp);

  } else if (_tcscmp(misc, _T("+0.5")) == 0) {
    mc = std::min(mc + Units::ToSysVSpeed(fixed_half), fixed(5));
    ActionInterface::SetMacCready(mc);
    return true;

  } else if (_tcscmp(misc, _T("-0.1")) == 0) {
    return HandleKey(ibkDown);

  } else if (_tcscmp(misc, _T("-0.5")) == 0) {
    mc = std::max(mc - Units::ToSysVSpeed(fixed_half), fixed_zero);
    ActionInterface::SetMacCready(mc);
    return true;

  } else if (_tcscmp(misc, _T("mode")) == 0) {
    return HandleKey(ibkEnter);
  }

  return false;
}
