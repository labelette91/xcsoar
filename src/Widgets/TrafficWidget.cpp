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

#include "TrafficWidget.hpp"
#include "Dialogs/Traffic.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Math/Screen.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Form/CheckBox.hpp"
#include "Form/Button.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Profile/Profile.hpp"
#include "Compiler.h"
#include "FLARM/Friends.hpp"
#include "Gauge/FlarmTrafficLook.hpp"
#include "Gauge/FlarmTrafficWindow.hpp"
#include "Language/Language.hpp"
#include "GestureManager.hpp"
#include "Units/UnitsFormatter.hpp"
#include "Input/InputEvents.hpp"
#include "Interface.hpp"

/**
 * A Window which renders FLARM traffic, with user interaction.
 */
class FlarmTrafficControl2 : public FlarmTrafficWindow {
protected:
  bool enable_auto_zoom;
  unsigned zoom;
  Angle task_direction;
  GestureManager gestures;

public:
  FlarmTrafficControl2(const FlarmTrafficLook &look)
    :FlarmTrafficWindow(look, Layout::Scale(10)),
     enable_auto_zoom(true),
     zoom(2),
     task_direction(Angle::Degrees(fixed_minus_one)) {}

protected:
  void CalcAutoZoom();

public:
  void Update(Angle new_direction, const FlarmState &new_data,
              const TeamCodeSettings &new_settings);
  void UpdateTaskDirection(bool show_task_direction, Angle bearing);

  bool GetNorthUp() const {
    return enable_north_up;
  }

  void SetNorthUp(bool enabled);

  void ToggleNorthUp() {
    SetNorthUp(!GetNorthUp());
  }

  bool GetAutoZoom() const {
    return enable_auto_zoom;
  }

  static unsigned GetZoomDistance(unsigned zoom);

  void SetZoom(unsigned _zoom) {
    zoom = _zoom;
    SetDistance(fixed(GetZoomDistance(_zoom)));
  }

  void SetAutoZoom(bool enabled);

  void ToggleAutoZoom() {
    SetAutoZoom(!GetAutoZoom());
  }

  void ZoomOut();
  void ZoomIn();

protected:
  void PaintTrafficInfo(Canvas &canvas) const;
  void PaintTaskDirection(Canvas &canvas) const;

protected:
  virtual void on_create();
  virtual void on_paint(Canvas &canvas);
  virtual bool on_mouse_move(PixelScalar x, PixelScalar y, unsigned keys);
  virtual bool on_mouse_down(PixelScalar x, PixelScalar y);
  virtual bool on_mouse_up(PixelScalar x, PixelScalar y);
  virtual bool on_mouse_double(PixelScalar x, PixelScalar y);
  virtual bool on_key_down(unsigned key_code);
  bool on_mouse_gesture(const TCHAR* gesture);
};

/** XXX this hack is needed because the form callbacks don't get a
    context pointer - please refactor! */
static TrafficWidget *instance;

void
FlarmTrafficControl2::on_create()
{
  FlarmTrafficWindow::on_create();

  Profile::Get(szProfileFlarmSideData, side_display_type);
  Profile::Get(szProfileFlarmAutoZoom, enable_auto_zoom);
  Profile::Get(szProfileFlarmNorthUp, enable_north_up);
}

unsigned
FlarmTrafficControl2::GetZoomDistance(unsigned zoom)
{
  switch (zoom) {
  case 0:
    return 500;
  case 1:
    return 1000;
  case 3:
    return 5000;
  case 4:
    return 10000;
  case 2:
  default:
    return 2000;
  }
}

void
FlarmTrafficControl2::SetNorthUp(bool enabled)
{
  enable_north_up = enabled;
  Profile::Set(szProfileFlarmNorthUp, enabled);
  //north_up->set_checked(enabled);
}

void
FlarmTrafficControl2::SetAutoZoom(bool enabled)
{
  enable_auto_zoom = enabled;
  Profile::Set(szProfileFlarmAutoZoom, enabled);
  //auto_zoom->set_checked(enabled);
}

void
FlarmTrafficControl2::CalcAutoZoom()
{
  bool warning_mode = WarningMode();
  RoughDistance zoom_dist = fixed_zero;

  for (auto it = data.traffic.begin(), end = data.traffic.end();
      it != end; ++it) {
    if (warning_mode && !it->HasAlarm())
      continue;

    zoom_dist = max(it->distance, zoom_dist);
  }

  fixed zoom_dist2 = zoom_dist;
  for (unsigned i = 0; i <= 4; i++) {
    if (i == 4 || fixed(GetZoomDistance(i)) >= zoom_dist2) {
      SetZoom(i);
      break;
    }
  }
}

void
FlarmTrafficControl2::Update(Angle new_direction, const FlarmState &new_data,
                            const TeamCodeSettings &new_settings)
{
  FlarmTrafficWindow::Update(new_direction, new_data, new_settings);

  if (enable_auto_zoom || WarningMode())
    CalcAutoZoom();
}

void
FlarmTrafficControl2::UpdateTaskDirection(bool show_task_direction, Angle bearing)
{
  if (!show_task_direction)
    task_direction = Angle::Degrees(fixed_minus_one);
  else
    task_direction = bearing.AsBearing();
}

/**
 * Zoom out one step
 */
void
FlarmTrafficControl2::ZoomOut()
{
  if (WarningMode())
    return;

  if (zoom < 4)
    SetZoom(zoom + 1);

  SetAutoZoom(false);
}

/**
 * Zoom in one step
 */
void
FlarmTrafficControl2::ZoomIn()
{
  if (WarningMode())
    return;

  if (zoom > 0)
    SetZoom(zoom - 1);

  SetAutoZoom(false);
}

/**
 * Paints an arrow into the direction of the current task leg
 * @param canvas The canvas to paint on
 */
void
FlarmTrafficControl2::PaintTaskDirection(Canvas &canvas) const
{
  if (negative(task_direction.Degrees()))
    return;

  canvas.Select(look.radar_pen);
  canvas.SelectHollowBrush();

  RasterPoint triangle[4];
  triangle[0].x = 0;
  triangle[0].y = -radius / Layout::FastScale(1) + 15;
  triangle[1].x = 7;
  triangle[1].y = triangle[0].y + 30;
  triangle[2].x = -triangle[1].x;
  triangle[2].y = triangle[1].y;
  triangle[3].x = triangle[0].x;
  triangle[3].y = triangle[0].y;

  PolygonRotateShift(triangle, 4, radar_mid.x, radar_mid.y,
                     task_direction - (enable_north_up ?
                                       Angle::Zero() : heading));

  // Draw the arrow
  canvas.polygon(triangle, 4);
}

/**
 * Paints the basic info for the selected target on the given canvas
 * @param canvas The canvas to paint on
 */
void
FlarmTrafficControl2::PaintTrafficInfo(Canvas &canvas) const
{
  // Don't paint numbers if no plane selected
  if (selection == -1 && !WarningMode())
    return;

  // Shortcut to the selected traffic
  FlarmTraffic traffic = data.traffic[WarningMode() ? warning : selection];
  assert(traffic.IsDefined());

  // Temporary string
  TCHAR tmp[20];
  // Temporary string size
  PixelSize sz;

  PixelRect rc;
  rc.left = padding;
  rc.top = padding;
  rc.right = canvas.get_width() - padding;
  rc.bottom = canvas.get_height() - padding;

  // Set the text color and background
  switch (traffic.alarm_level) {
  case FlarmTraffic::AlarmType::LOW:
    canvas.SetTextColor(look.warning_color);
    break;
  case FlarmTraffic::AlarmType::IMPORTANT:
  case FlarmTraffic::AlarmType::URGENT:
    canvas.SetTextColor(look.alarm_color);
    break;
  case FlarmTraffic::AlarmType::NONE:
    canvas.SetTextColor(look.default_color);
    break;
  }

  canvas.SetBackgroundTransparent();

  // Climb Rate
  if (!WarningMode() && traffic.climb_rate_avg30s_available) {
    Units::FormatUserVSpeed(traffic.climb_rate_avg30s, tmp, 20);
    canvas.Select(look.info_values_font);
    sz = canvas.CalcTextSize(tmp);
    canvas.text(rc.right - sz.cx, rc.top + look.info_labels_font.GetHeight(), tmp);

    canvas.Select(look.info_labels_font);
    sz = canvas.CalcTextSize(_("Vario"));
    canvas.text(rc.right - sz.cx, rc.top, _("Vario"));
  }

  // Distance
  Units::FormatUserDistance(traffic.distance, tmp, 20);
  canvas.Select(look.info_values_font);
  sz = canvas.CalcTextSize(tmp);
  canvas.text(rc.left, rc.bottom - sz.cy, tmp);

  canvas.Select(look.info_labels_font);
  canvas.text(rc.left,
              rc.bottom - look.info_values_font.GetHeight() - look.info_labels_font.GetHeight(),
              _("Distance"));

  // Relative Height
  Units::FormatUserArrival(traffic.relative_altitude, tmp, 20);
  canvas.Select(look.info_values_font);
  sz = canvas.CalcTextSize(tmp);
  canvas.text(rc.right - sz.cx, rc.bottom - sz.cy, tmp);

  canvas.Select(look.info_labels_font);
  sz = canvas.CalcTextSize(_("Rel. Alt."));
  canvas.text(rc.right - sz.cx,
              rc.bottom - look.info_values_font.GetHeight() - look.info_labels_font.GetHeight(),
              _("Rel. Alt."));

  // ID / Name
  unsigned font_size;
  if (traffic.HasName()) {
    canvas.Select(look.call_sign_font);
    font_size = look.call_sign_font.GetHeight();

    if (!traffic.HasAlarm())
      canvas.SetTextColor(look.selection_color);

    _tcscpy(tmp, traffic.name);
  } else {
    font_size = look.info_labels_font.GetHeight();
    traffic.id.Format(tmp);
  }

  if (!WarningMode()) {
    // Team color dot
    FlarmFriends::Color team_color = FlarmFriends::GetFriendColor(traffic.id);

    // If no color found but target is teammate
    if (team_color == FlarmFriends::NONE &&
        settings.team_flarm_tracking &&
        traffic.id == settings.team_flarm_id)
      // .. use yellow color
      team_color = FlarmFriends::GREEN;

    // If team color found -> draw a colored circle around the target
    if (team_color != FlarmFriends::NONE) {
      switch (team_color) {
      case FlarmFriends::GREEN:
        canvas.Select(look.team_brush_green);
        break;
      case FlarmFriends::BLUE:
        canvas.Select(look.team_brush_blue);
        break;
      case FlarmFriends::YELLOW:
        canvas.Select(look.team_brush_green);
        break;
      case FlarmFriends::MAGENTA:
        canvas.Select(look.team_brush_magenta);
        break;
      default:
        break;
      }

      canvas.SelectNullPen();
      canvas.circle(rc.left + Layout::FastScale(7), rc.top + (font_size / 2),
                    Layout::FastScale(7));

      rc.left += Layout::FastScale(16);
    }
  }

  canvas.text(rc.left, rc.top, tmp);
}

void
FlarmTrafficControl2::on_paint(Canvas &canvas)
{
  canvas.ClearWhite();

  PaintTaskDirection(canvas);
  FlarmTrafficWindow::Paint(canvas);
  PaintTrafficInfo(canvas);
}

void
TrafficWidget::OpenDetails()
{
  // If warning is displayed -> prevent from opening details dialog
  if (view->WarningMode())
    return;

  // Don't open the details dialog if no plane selected
  const FlarmTraffic *traffic = view->GetTarget();
  if (traffic == NULL)
    return;

  // Show the details dialog
  dlgFlarmTrafficDetailsShowModal(traffic->id);
}

void
TrafficWidget::ZoomIn()
{
  view->ZoomIn();
}

void
TrafficWidget::ZoomOut()
{
  view->ZoomOut();
}

void
TrafficWidget::PreviousTarget()
{
  view->PrevTarget();
}

void
TrafficWidget::NextTarget()
{
  view->NextTarget();
}

void
TrafficWidget::SwitchData()
{
  view->side_display_type++;
  if (view->side_display_type > 2)
    view->side_display_type = 1;

  Profile::Set(szProfileFlarmSideData, view->side_display_type);
}

void
TrafficWidget::SetAutoZoom(bool value)
{
  view->SetAutoZoom(value);
}

void
TrafficWidget::ToggleAutoZoom()
{
  view->ToggleAutoZoom();
}

void
TrafficWidget::SetNorthUp(bool value)
{
  view->SetAutoZoom(value);
}

void
TrafficWidget::ToggleNorthUp()
{
  view->ToggleNorthUp();
}

void
TrafficWidget::Update()
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();

#if 0
  if (XCSoarInterface::GetUISettings().auto_close_flarm_dialog &&
      (!basic.flarm.available ||
       basic.flarm.GetActiveTrafficCount() == 0))
    wf->SetModalResult(mrOK);
#endif

  view->Update(basic.track,
               basic.flarm,
               XCSoarInterface::GetComputerSettings());

  view->UpdateTaskDirection(calculated.task_stats.task_valid &&
                            calculated.task_stats.current_leg.solution_remaining.IsOk(),
                            calculated.task_stats.
                            current_leg.solution_remaining.cruise_track_bearing);
}

bool
FlarmTrafficControl2::on_mouse_move(PixelScalar x, PixelScalar y,
                                   gcc_unused unsigned keys)
{
  gestures.Update(x, y);

  return true;
}

bool
FlarmTrafficControl2::on_mouse_down(PixelScalar x, PixelScalar y)
{
  gestures.Start(x, y, Layout::Scale(20));

  return true;
}

bool
FlarmTrafficControl2::on_mouse_up(PixelScalar x, PixelScalar y)
{
  const TCHAR *gesture = gestures.Finish();
  if (gesture && on_mouse_gesture(gesture))
    return true;

  if (!WarningMode())
    SelectNearTarget(x, y, Layout::Scale(15));

  return true;
}

bool
FlarmTrafficControl2::on_mouse_double(PixelScalar x, PixelScalar y)
{
  InputEvents::ShowMenu();
  return true;
}

bool
FlarmTrafficControl2::on_mouse_gesture(const TCHAR* gesture)
{
  if (_tcscmp(gesture, _T("U")) == 0) {
    ZoomIn();
    return true;
  }
  if (_tcscmp(gesture, _T("D")) == 0) {
    ZoomOut();
    return true;
  }
  if (_tcscmp(gesture, _T("L")) == 0) {
    PrevTarget();
    return true;
  }
  if (_tcscmp(gesture, _T("R")) == 0) {
    NextTarget();
    return true;
  }
  if (_tcscmp(gesture, _T("UD")) == 0) {
    SetAutoZoom(true);
    return true;
  }
  if (_tcscmp(gesture, _T("DR")) == 0) {
    instance->OpenDetails();
    return true;
  }
  if (_tcscmp(gesture, _T("RL")) == 0) {
    instance->SwitchData();
    return true;
  }

  return false;
}

bool
FlarmTrafficControl2::on_key_down(unsigned key_code)
{
  switch (key_code) {
  case VK_UP:
    if (!HasPointer())
      break;

    ZoomIn();
    return true;

  case VK_DOWN:
    if (!HasPointer())
      break;

    ZoomOut();
    return true;

  case VK_LEFT:
#ifdef GNAV
  case '6':
#endif
    PrevTarget();
    return true;

  case VK_RIGHT:
#ifdef GNAV
  case '7':
#endif
    NextTarget();
    return true;
  }

  return FlarmTrafficWindow::on_key_down(key_code) ||
    InputEvents::processKey(key_code);
}

void
TrafficWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  instance = this;

  WindowStyle style;
  style.hide();
  style.enable_double_clicks();

  const Look &look = CommonInterface::main_window.GetLook();
  view = new FlarmTrafficControl2(look.flarm_dialog);
  view->set(parent, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            style);
  SetWindow(view);
}

void
TrafficWidget::Unprepare()
{
  delete view;

  WindowWidget::Unprepare();
}

void
TrafficWidget::Show(const PixelRect &rc)
{
  // Update Radar and Selection for the first time
  Update();

#if 0
  // Get the last chosen Side Data configuration
  auto_zoom = (CheckBox *)form.FindByName(_T("AutoZoom"));
  auto_zoom->set_checked(view->GetAutoZoom());

  north_up = (CheckBox *)form.FindByName(_T("NorthUp"));
  north_up->set_checked(view->GetNorthUp());
#endif

  WindowWidget::Show(rc);

  CommonInterface::GetLiveBlackboard().AddListener(*this);
}

void
TrafficWidget::Hide()
{
  CommonInterface::GetLiveBlackboard().RemoveListener(*this);
  WindowWidget::Hide();
}

bool
TrafficWidget::SetFocus()
{
  GetWindow()->set_focus();
  return true;
}

void
TrafficWidget::OnGPSUpdate(const MoreData &basic)
{
  Update();
}
