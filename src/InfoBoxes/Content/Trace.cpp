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

#include "InfoBoxes/Content/Trace.hpp"
#include "InfoBoxes/InfoBoxWindow.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Renderer/FlightStatisticsRenderer.hpp"
#include "Renderer/TraceHistoryRenderer.hpp"
#include "Renderer/ThermalBandRenderer.hpp"
#include "Renderer/TaskProgressRenderer.hpp"
#include "Units/UnitsFormatter.hpp"
#include "Units/Units.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "DeviceBlackboard.hpp"
#include "Screen/Layout.hpp"
#include "Protection.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Computer/GlideComputer.hpp"
#include "Dialogs/dlgInfoBoxAccess.hpp"
#include "Dialogs/dlgAnalysis.hpp"
#include "Util/Macros.hpp"

static PixelRect
get_spark_rect(const InfoBoxWindow &infobox)
{
  PixelRect rc = infobox.GetValueRect();
  rc.top += Layout::FastScale(2);
  rc.right -= Layout::FastScale(2);
  rc.left += Layout::FastScale(2);
  return rc;
}

void
InfoBoxContentSpark::do_paint(InfoBoxWindow &infobox, Canvas &canvas,
                              const TraceVariableHistory& var,
                              const bool center)
{
  if (var.empty())
    return;

  const Look &look = CommonInterface::main_window.GetLook();
  TraceHistoryRenderer renderer(look.trace_history, look.vario, look.chart);
  renderer.RenderVario(canvas, get_spark_rect(infobox), var, center,
                       CommonInterface::GetComputerSettings().glide_polar_task.GetMC());
}

void
InfoBoxContentVarioSpark::on_custom_paint(InfoBoxWindow &infobox, Canvas &canvas)
{
  do_paint(infobox, canvas, CommonInterface::Calculated().trace_history.BruttoVario);
}

void
InfoBoxContentNettoVarioSpark::on_custom_paint(InfoBoxWindow &infobox, Canvas &canvas)
{
  do_paint(infobox, canvas, CommonInterface::Calculated().trace_history.NettoVario);
}

void
InfoBoxContentCirclingAverageSpark::on_custom_paint(InfoBoxWindow &infobox, Canvas &canvas)
{
  do_paint(infobox, canvas, CommonInterface::Calculated().trace_history.CirclingAverage,
    false);
}

void
InfoBoxContentSpark::label_vspeed(InfoBoxData &data,
                                  const TraceVariableHistory& var)
{
  if (var.empty())
    return;

  TCHAR sTmp[32];
  Units::FormatUserVSpeed(var.last(), sTmp,
                          ARRAY_SIZE(sTmp));
  data.SetComment(sTmp);

  data.SetCustom();
}

void
InfoBoxContentVarioSpark::Update(InfoBoxData &data)
{
  label_vspeed(data, CommonInterface::Calculated().trace_history.BruttoVario);
}

void
InfoBoxContentNettoVarioSpark::Update(InfoBoxData &data)
{
  label_vspeed(data, CommonInterface::Calculated().trace_history.NettoVario);
}

void
InfoBoxContentCirclingAverageSpark::Update(InfoBoxData &data)
{
  label_vspeed(data, CommonInterface::Calculated().trace_history.CirclingAverage);
}


void
InfoBoxContentBarogram::Update(InfoBoxData &data)
{
  const MoreData &basic = CommonInterface::Basic();
  TCHAR sTmp[32];

  Units::FormatUserAltitude(basic.nav_altitude, sTmp,
                            ARRAY_SIZE(sTmp));
  data.SetComment(sTmp);

  data.SetCustom();
}

void
InfoBoxContentBarogram::on_custom_paint(InfoBoxWindow &infobox, Canvas &canvas)
{
  const Look &look = CommonInterface::main_window.GetLook();
  FlightStatisticsRenderer fs(glide_computer->GetFlightStats(),
                              look.chart, look.map);
  fs.RenderBarographSpark(canvas, get_spark_rect(infobox),
                          infobox.GetLook().inverse,
                          XCSoarInterface::Basic(),
                          XCSoarInterface::Calculated(), protected_task_manager);
}

bool
InfoBoxContentBarogram::HandleKey(const InfoBoxKeyCodes keycode)
{
  switch (keycode) {
  case ibkEnter:
    dlgAnalysisShowModal(XCSoarInterface::main_window,
                         CommonInterface::main_window.GetLook(),
                         CommonInterface::Full(), *glide_computer,
                         protected_task_manager,
                         &airspace_database,
                         terrain, 0);
    return true;

  case ibkUp:
  case ibkDown:
  case ibkLeft:
  case ibkRight:
    break;
  }
  return false;
}


void
InfoBoxContentThermalBand::on_custom_paint(InfoBoxWindow &infobox, Canvas &canvas)
{
  const Look &look = CommonInterface::main_window.GetLook();
  ThermalBandRenderer renderer(look.thermal_band, look.chart);
  renderer.DrawThermalBandSpark(CommonInterface::Basic(),
                                CommonInterface::Calculated(),
                                CommonInterface::GetComputerSettings(),
                                canvas,
                                infobox.GetValueAndCommentRect(),
                                XCSoarInterface::GetComputerSettings().task);
}

void
InfoBoxContentThermalBand::Update(InfoBoxData &data)
{
  data.SetCustom();
}


void
InfoBoxContentTaskProgress::on_custom_paint(InfoBoxWindow &infobox, Canvas &canvas)
{
  const Look &look = CommonInterface::main_window.GetLook();
  TaskProgressRenderer renderer(look.map.task);
  renderer.Draw(CommonInterface::Calculated().
                common_stats.ordered_summary,
                canvas, infobox.GetValueAndCommentRect(),
                infobox.GetLook().inverse);
}

void
InfoBoxContentTaskProgress::Update(InfoBoxData &data)
{
  data.SetCustom();
}
