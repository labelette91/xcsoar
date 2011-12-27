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

#include "WindEdit.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/dlgInfoBoxAccess.hpp"
#include "Form/TabBar.hpp"
#include "Form/XMLWidget.hpp"
#include "Form/Edit.hpp"
#include "DataField/Float.hpp"
#include "Interface.hpp"
#include "Units/Units.hpp"

class WindEditPanel : public XMLWidget {
public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Show(const PixelRect &rc);
};

static void
PnlEditOnWindSpeed(gcc_unused DataFieldFloat &Sender)
{
  const NMEAInfo &basic = XCSoarInterface::Basic();
  ComputerSettings &settings_computer =
    XCSoarInterface::SetComputerSettings();
  const bool external_wind = basic.external_wind_available &&
    settings_computer.use_external_wind;

  if (!external_wind) {
    settings_computer.manual_wind.norm =
      Units::ToSysWindSpeed(Sender.GetAsFixed());
    settings_computer.manual_wind_available.Update(basic.clock);
  }
}

static void
PnlEditOnWindDirection(gcc_unused DataFieldFloat &Sender)
{
  const NMEAInfo &basic = XCSoarInterface::Basic();
  ComputerSettings &settings_computer =
    XCSoarInterface::SetComputerSettings();
  const bool external_wind = basic.external_wind_available &&
    settings_computer.use_external_wind;

  if (!external_wind) {
    settings_computer.manual_wind.bearing = Angle::Degrees(Sender.GetAsFixed());
    settings_computer.manual_wind_available.Update(basic.clock);
  }
}

static gcc_constexpr_data
CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(PnlEditOnWindSpeed),
  DeclareCallBackEntry(PnlEditOnWindDirection),
  DeclareCallBackEntry(NULL)
};

void
WindEditPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  LoadWindow(CallBackTable, parent, _T("IDR_XML_INFOBOXWINDEDIT"));
}

void
WindEditPanel::Show(const PixelRect &rc)
{
  const NMEAInfo &basic = XCSoarInterface::Basic();
  const ComputerSettings &settings_computer =
    XCSoarInterface::GetComputerSettings();
  const bool external_wind = basic.external_wind_available &&
    settings_computer.use_external_wind;

  WndProperty* wp;

  const SpeedVector wind = CommonInterface::Calculated().GetWindOrZero();

  wp = (WndProperty *)form.FindByName(_T("prpSpeed"));
  if (wp) {
    wp->set_enabled(!external_wind);
    DataFieldFloat &df = *(DataFieldFloat *)wp->GetDataField();
    df.SetMax(Units::ToUserWindSpeed(Units::ToSysUnit(fixed(200), unKiloMeterPerHour)));
    df.SetUnits(Units::GetSpeedName());
    df.Set(Units::ToUserWindSpeed(wind.norm));
    wp->RefreshDisplay();
  }

  wp = (WndProperty *)form.FindByName(_T("prpDirection"));
  if (wp) {
    wp->set_enabled(!external_wind);
    DataFieldFloat &df = *(DataFieldFloat *)wp->GetDataField();
    df.Set(wind.bearing.Degrees());
    wp->RefreshDisplay();
  }

  XMLWidget::Show(rc);
}

Widget *
LoadWindEditPanel(unsigned id)
{
  return new WindEditPanel();
}
