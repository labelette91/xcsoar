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

#include "Profile/Profile.hpp"
#include "Profile/InfoBoxConfig.hpp"
#include "Profile/ComputerProfile.hpp"
#include "Profile/UIProfile.hpp"
#include "LogFile.hpp"
#include "Dialogs/XML.hpp"
#include "Interface.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"

void
Profile::Use()
{
  LogStartUp(_T("Read registry settings"));

  Load(CommonInterface::SetComputerSettings());
  Load(CommonInterface::SetUISettings());

  GetEnum(szProfileAppDialogStyle, dialog_style_setting);
}

void
Profile::SetSoundSettings()
{
  const ComputerSettings &settings_computer =
    XCSoarInterface::GetComputerSettings();

  Set(szProfileSoundVolume,
      settings_computer.sound_volume);
  Set(szProfileSoundDeadband,
      settings_computer.sound_deadband);
  Set(szProfileSoundAudioVario,
      settings_computer.sound_vario_enabled);
  Set(szProfileSoundTask,
      settings_computer.sound_task_enabled);
  Set(szProfileSoundModes,
      settings_computer.sound_modes_enabled);
}
