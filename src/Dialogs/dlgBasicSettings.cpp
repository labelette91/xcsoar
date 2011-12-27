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

#include "Dialogs/Dialogs.h"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/Internal.hpp"
#include "Protection.hpp"
#include "DeviceBlackboard.hpp"
#include "ComputerSettings.hpp"
#include "Units/Units.hpp"
#include "Units/UnitsFormatter.hpp"
#include "Atmosphere/CuSonde.hpp"
#include "DataField/Base.hpp"
#include "DataField/Float.hpp"
#include "MainWindow.hpp"
#include "Components.hpp"
#include "GlideSolvers/GlidePolar.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Compiler.h"

#include <math.h>

static WndForm *wf = NULL;

static bool changed = false;

static GlidePolar glide_polar;

static void
SetButtons()
{
  WndButton* wb;

  if ((wb = (WndButton *)wf->FindByName(_T("cmdDump"))) != NULL) {
    wb->set_visible(glide_polar.HasBallast());
    wb->SetCaption(XCSoarInterface::GetComputerSettings().ballast_timer_active ?
        _("Stop") : _("Dump"));
  }
}

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static void
SetBallastTimer(bool active)
{
  if (protected_task_manager == NULL)
    return;

  if (active == XCSoarInterface::GetComputerSettings().ballast_timer_active)
    return;

  if (active && changed) {
    /* apply the new ballast settings before starting the timer */
    CommonInterface::SetComputerSettings().glide_polar_task = glide_polar;
    protected_task_manager->SetGlidePolar(glide_polar);
    changed = false;
  }

  XCSoarInterface::SetComputerSettings().ballast_timer_active = active;

  SetButtons();
}

static void
OnBallastDump(gcc_unused WndButton &Sender)
{
  SetBallastTimer(!XCSoarInterface::GetComputerSettings().ballast_timer_active);
}

static void
ShowAltitude(fixed altitude)
{
  static fixed last(-2);

  if (fabs(altitude - last) < fixed_one)
    return;

  last = altitude;

  LoadFormProperty(*wf, _T("prpAltitude"), ugAltitude, altitude);
  ShowFormControl(*wf, _T("prpAltitude"), true);
}

static void
HideAltitude()
{
  ShowFormControl(*wf, _T("prpAltitude"), false);
}

static void
RefreshAltitudeControl()
{
  const NMEAInfo &basic = CommonInterface::Basic();
  ComputerSettings &settings_computer =
    CommonInterface::SetComputerSettings();

  if (basic.pressure_altitude_available && settings_computer.pressure_available)
    ShowAltitude(settings_computer.pressure.PressureAltitudeToQNHAltitude(basic.pressure_altitude));
  else if (basic.baro_altitude_available)
    ShowAltitude(basic.baro_altitude);
  else
    HideAltitude();
}

static void
OnQnhData(DataField *_Sender, DataField::DataAccessKind_t Mode)
{
  DataFieldFloat *Sender = (DataFieldFloat *)_Sender;
  const NMEAInfo &basic = CommonInterface::Basic();
  ComputerSettings &settings_computer =
    CommonInterface::SetComputerSettings();

  switch (Mode) {
  case DataField::daChange:
    settings_computer.pressure =
      AtmosphericPressure::HectoPascal(Units::ToSysPressure(Sender->GetAsFixed()))
;
    settings_computer.pressure_available.Update(basic.clock);
    device_blackboard->SetQNH(Units::ToSysPressure(Sender->GetAsFixed()));
    RefreshAltitudeControl();
    break;

  case DataField::daSpecial:
    return;
  }
}

static void
SetBallast(void)
{
  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(_T("prpBallast"));
  if (wp) {
    if (glide_polar.IsBallastable()) {
      DataFieldFloat &df = *(DataFieldFloat *)wp->GetDataField();
      df.SetAsFloat(glide_polar.GetBallastLitres());
    } else
      wp->hide();

    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(_T("prpWingLoading"));
  if (wp) {
    const fixed wl = glide_polar.GetWingLoading();
    if (wl > fixed_zero) {
      DataFieldFloat &df = *(DataFieldFloat *)wp->GetDataField();
      df.SetAsFloat(wl);
    } else
      wp->hide();

    wp->RefreshDisplay();
  }
}

/**
 * This function is called repeatedly by the timer and updates the
 * current altitude and ballast. The ballast can change without user
 * input due to the dump function.
 */
static void
OnTimerNotify(gcc_unused WndForm &Sender)
{
  if (protected_task_manager != NULL &&
      XCSoarInterface::GetComputerSettings().ballast_timer_active && !changed) {
    /* get new GlidePolar values */
    glide_polar = CommonInterface::GetComputerSettings().glide_polar_task;

    /* display the new values on the screen */
    SetBallast();

    /* SetBallast() may have set the "changed" flag, reset it */
    changed = false;
  }

  RefreshAltitudeControl();
}

static void
OnBallastData(DataField *Sender, DataField::DataAccessKind_t Mode)
{
  DataFieldFloat &df = *(DataFieldFloat *)Sender;

  switch (Mode) {
  case DataField::daSpecial:
    SetBallastTimer(glide_polar.HasBallast() &&
                    !XCSoarInterface::GetComputerSettings().ballast_timer_active);
    break;
  case DataField::daChange:
    glide_polar.SetBallastLitres(df.GetAsFixed());
    changed = true;
    SetButtons();
    SetBallast();
    break;
  }
}

static void
OnBugsData(DataField *_Sender, DataField::DataAccessKind_t Mode)
{
  DataFieldFloat *Sender = (DataFieldFloat *)_Sender;

  switch (Mode) {
  case DataField::daChange:
    glide_polar.SetBugs(fixed_one - (Sender->GetAsFixed() / 100));
    changed = true;
    break;

  case DataField::daSpecial:
    return;
  }
}

static gcc_constexpr_data CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnBugsData),

  DeclareCallBackEntry(OnBallastData),
  DeclareCallBackEntry(OnQnhData),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnBallastDump),
  DeclareCallBackEntry(NULL)
};

void
dlgBasicSettingsShowModal()
{
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();

  glide_polar = settings.glide_polar_task;

  wf = LoadDialog(CallBackTable, XCSoarInterface::main_window,
                      _T("IDR_XML_BASICSETTINGS"));
  if (wf == NULL)
    return;

  changed = false;

  wf->SetTimerNotify(OnTimerNotify);
  OnTimerNotify(*wf);

  SetButtons();

  SetBallast();
  LoadFormProperty(*wf, _T("prpBugs"), (fixed_one - glide_polar.GetBugs()) * 100);
  LoadFormProperty(*wf, _T("prpQNH"), Units::ToUserPressure(settings.pressure.GetHectoPascal()));

  WndProperty* wp;
  wp = (WndProperty*)wf->FindByName(_T("prpQNH"));
  if (wp) {
    DataFieldFloat &df = *(DataFieldFloat *)wp->GetDataField();

    df.SetMin(Units::ToUserPressure(Units::ToSysUnit(fixed(850), unHectoPascal)));
    df.SetMax(Units::ToUserPressure(Units::ToSysUnit(fixed(1300), unHectoPascal)));
    df.SetStep(Units::ToUserPressure(Units::ToSysUnit(fixed_one, unHectoPascal)));
    df.SetUnits(Units::GetPressureName());
    df.SetStep(Units::PressureStep());
    df.SetFormat( Units::GetFormatUserPressure());
    wp->RefreshDisplay();
}
  wp = (WndProperty*)wf->FindByName(_T("prpTemperature"));
  if (wp) {
    DataFieldFloat &df = *(DataFieldFloat *)wp->GetDataField();
    df.SetMin(Units::ToUserTemperature(Units::ToSysUnit(fixed(-50), unGradCelcius)));
    df.SetMax(Units::ToUserTemperature(Units::ToSysUnit(fixed(60), unGradCelcius)));
    df.SetUnits(Units::GetTemperatureName());
    df.Set(Units::ToUserTemperature(settings.forecast_temperature));
    wp->RefreshDisplay();
  }

  if (wf->ShowModal() == mrOK) {
    ComputerSettings &settings = CommonInterface::SetComputerSettings();

    if (changed) {
      settings.glide_polar_task = glide_polar;

      if (protected_task_manager != NULL)
        protected_task_manager->SetGlidePolar(glide_polar);
    }

    SaveFormProperty(*wf, _T("prpTemperature"),
                     ugTemperature, settings.forecast_temperature);
  }

  delete wf;
}
