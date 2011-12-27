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

#include "MapDisplayConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Form/Util.hpp"
#include "DataField/Enum.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Form/RowFormWidget.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"

enum ControlIndex {
  OrientationCruise,
  OrientationCircling,
  CirclingZoom,
  MapShiftBias,
  GliderScreenPosition,
  MaxAutoZoomDistance,
};

static const StaticEnumChoice orientation_list[] = {
  { TRACKUP, N_("Track up"),
    N_("The moving map display will be rotated so the glider's track is oriented up.") },
  { NORTHUP, N_("North up"),
    N_("The moving map display will always be orientated north to south and the glider icon will be rotated to show its course.") },
  { TARGETUP, N_("Target up"),
    N_("The moving map display will be rotated so the navigation target is oriented up.") },
  { 0 }
};

static const StaticEnumChoice shift_bias_list[] = {
  { MAP_SHIFT_BIAS_NONE, N_("None"), N_("Disable adjustments.") },
  { MAP_SHIFT_BIAS_TRACK, N_("Track"),
    N_("Use a recent average of the ground track as basis.") },
  { MAP_SHIFT_BIAS_TARGET, N_("Target"),
    N_("Use the current target waypoint as basis.") },
  { 0 }
};

class MapDisplayConfigPanel : public RowFormWidget {

public:
  MapDisplayConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook(), Layout::Scale(150)) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
  void UpdateVisibilities();
};

/** XXX this hack is needed because the form callbacks don't get a
    context pointer - please refactor! */
static MapDisplayConfigPanel *instance;

void
MapDisplayConfigPanel::UpdateVisibilities()
{
  bool northup = GetValueInteger(OrientationCruise) == NORTHUP;

  GetControl(MapShiftBias).set_visible(northup);
}

static void
OnShiftTypeData(DataField *Sender, DataField::DataAccessKind_t Mode)
{
  switch (Mode) {
  case DataField::daChange: {
    instance->UpdateVisibilities();
    break;
  }
  case DataField::daSpecial:
    return;
  }
}

void
MapDisplayConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  instance = this;

  RowFormWidget::Prepare(parent, rc);

  const MapSettings &settings_map = CommonInterface::GetMapSettings();

  AddEnum(_("Cruise orientation"),
          _("Determines how the screen is rotated with the glider"),
          orientation_list,
          settings_map.cruise_orientation,
          OnShiftTypeData);

  AddEnum(_("Circling orientation"),
          _("Determines how the screen is rotated with the glider while circling"),
          orientation_list,
          settings_map.cruise_orientation,
          OnShiftTypeData);

  AddBoolean(_("Circling zoom"),
             _("If enabled, then the map will zoom in automatically when entering circling mode and zoom out automatically when leaving circling mode."),
             settings_map.circle_zoom_enabled);

  AddEnum(_("Map shift reference"),
          _("Determines what is used to shift the glider from the map center"),
          shift_bias_list,
          settings_map.map_shift_bias,
          OnShiftTypeData);

  AddInteger(_("Glider position offset"),
             _("Defines the location of the glider drawn on the screen in percent from the screen edge."),
             _T("%d %%"), _T("%d"), 10, 50, 5,
             settings_map.glider_screen_position);

  AddFloat(_("Max. auto zoom distance"),
           _("The upper limit for auto zoom distance."),
           _T("%.0f %s"), _T("%.0f"), fixed(20), fixed(250), fixed(10), false,
           ugDistance, settings_map.max_auto_zoom_distance);

  UpdateVisibilities();
}

bool
MapDisplayConfigPanel::Save(bool &_changed, bool &require_restart)
{
  bool changed = false;

  MapSettings &settings_map = CommonInterface::SetMapSettings();

  changed |= SaveValueEnum(OrientationCruise, szProfileOrientationCruise,
                           settings_map.cruise_orientation);

  changed |= SaveValueEnum(OrientationCircling, szProfileOrientationCircling,
                           settings_map.circling_orientation);

  changed |= SaveValueEnum(MapShiftBias, szProfileMapShiftBias,
                           settings_map.map_shift_bias);

  changed |= SaveValue(GliderScreenPosition, szProfileGliderScreenPosition,
                       settings_map.glider_screen_position);

  changed |= SaveValue(CirclingZoom, szProfileCircleZoom,
                       settings_map.circle_zoom_enabled);

  changed |= SaveValue(MaxAutoZoomDistance, ugDistance,
                       szProfileMaxAutoZoomDistance,
                       settings_map.max_auto_zoom_distance);

  _changed |= changed;

  return true;
}

Widget *
CreateMapDisplayConfigPanel()
{
  return new MapDisplayConfigPanel();
}
