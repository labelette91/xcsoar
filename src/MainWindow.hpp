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

#ifndef XCSOAR_MAIN_WINDOW_HXX
#define XCSOAR_MAIN_WINDOW_HXX

#include "Screen/SingleWindow.hpp"
#include "Screen/Timer.hpp"
#include "InfoBoxes/InfoBoxLayout.hpp"
#include "PopupMessage.hpp"
#include "BatteryTimer.hpp"
#include "DisplayMode.hpp"
#include "Form/ManagedWidget.hpp"

#include <assert.h>

struct ComputerSettings;
struct MapSettings;
struct Look;
class GlueMapWindow;
class Widget;
class StatusMessageList;
class RasterTerrain;
class TopographyStore;
class MapWindowProjection;

/**
 * The XCSoar main window.
 */
class MainWindow : public SingleWindow {
  enum cmd {
    /**
     * Check the airspace_warning_pending flag and show the airspace
     * warning dialog.
     */
    CMD_AIRSPACE_WARNING,

    /**
     * Called by the #MergeThread when new GPS data is available.
     */
    CMD_GPS_UPDATE,

    /**
     * Called by the calculation thread when new calculation results
     * are available.  This updates the map and the info boxes.
     */
    CMD_CALCULATED_UPDATE,
  };

  Look *look;

  GlueMapWindow *map;

  /**
   * A #Widget that is shown instead of the map.  The #GlueMapWindow
   * is hidden and the DrawThread is suspended while this attribute is
   * non-NULL.
   */
  Widget *widget;

  ManagedWidget vario;

  ManagedWidget traffic_gauge;
  bool suppress_traffic_gauge, force_traffic_gauge;

  ManagedWidget thermal_assistant;

public:
  PopupMessage popup;

private:
  WindowTimer timer;

  BatteryTimer battery_timer;

  PixelRect map_rect;
  bool FullScreen;

  bool airspace_warning_pending;

public:
  MainWindow(const StatusMessageList &status_messages);
  virtual ~MainWindow();

  static bool find(const TCHAR *text) {
    return TopWindow::find(_T("XCSoarMain"), text);
  }

#ifdef USE_GDI
  static bool register_class(HINSTANCE hInstance);
#endif

protected:
  /**
   * Is XCSoar already up and running?
   */
  bool IsRunning() {
    /* it is safe enough to say that XCSoar initialization is complete
       after the MapWindow has been created */
    return map != NULL;
  }

  /**
   * Destroy the current Widget, but don't reactivate the map.  The
   * caller is responsible for reactivating the map or another Widget.
   */
  void KillWidget();

public:
  void set(const TCHAR *text,
           PixelScalar left, PixelScalar top,
           UPixelScalar width, UPixelScalar height);

  void Initialise();
  void InitialiseConfigured();

  /**
   * Destroy the components of the main view (map, info boxes,
   * gauges).
   */
  void Deinitialise();

  /**
   * Destroy and re-create all info boxes, and adjust the map
   * position/size.
   */
  void ReinitialiseLayout();

  /**
   * Adjust the flarm radar position
   */
  void ReinitialiseLayout_flarm(PixelRect rc, const InfoBoxLayout::Layout ib_layout);

  /**
   * Adjust vario
   */
  void ReinitialiseLayout_vario(const InfoBoxLayout::Layout &layout);

  void ReinitialiseLayoutTA(PixelRect rc, const InfoBoxLayout::Layout &layout);

  /**
   * Adjust the window position and size, to make it full-screen again
   * after display rotation.
   */
  void ReinitialisePosition();

  void reset();

  /**
   * Suspend threads that are owned by this object.
   */
  void SuspendThreads();

  /**
   * Resumt threads that are owned by this object.
   */
  void ResumeThreads();

  /**
   * Set the keyboard focus on the default element (i.e. the
   * MapWindow).
   */
  void SetDefaultFocus();

  /**
   * Trigger a full redraw of the screen.
   */
  void full_redraw();

  bool GetFullScreen() const {
    return FullScreen;
  }

  void SetFullScreen(bool _full_screen);

  /**
   * A new airspace warning was found.  This method sends the
   * CMD_AIRSPACE_WARNING command to this window, which displays the
   * airspace warning dialog.
   */
  void SendAirspaceWarning() {
    airspace_warning_pending = true;
    send_user(CMD_AIRSPACE_WARNING);
  }

  void SendGPSUpdate() {
    send_user(CMD_GPS_UPDATE);
  }

  void SendCalculatedUpdate() {
    send_user(CMD_CALCULATED_UPDATE);
  }

  void SetTerrain(RasterTerrain *terrain);
  void SetTopography(TopographyStore *topography);

  gcc_pure
  DisplayMode GetDisplayMode() const;

  const Look &GetLook() const {
    assert(look != NULL);

    return *look;
  }

  Look &SetLook() {
    assert(look != NULL);

    return *look;
  }

  void SetComputerSettings(const ComputerSettings &settings_computer);
  void SetMapSettings(const MapSettings &settings_map);

  /**
   * Returns the map even if it is not active.  May return NULL if
   * there is no map.
   */
  gcc_pure
  GlueMapWindow *GetMap() {
    return map;
  }

  /**
   * Is the map active, i.e. currently visible?
   */
  bool IsMapActive() const {
    return widget == NULL;
  }

  /**
   * Returns the map if it is active, or NULL if the map is not
   * active.
   */
  gcc_pure
  GlueMapWindow *GetMapIfActive();

  /**
   * Activate the map and return a pointer to it.  May return NULL if
   * there is no map.
   */
  GlueMapWindow *ActivateMap();

  /**
   * Replace the map with a #Widget.  The Widget instance gets deleted
   * when the map gets reactivated with ActivateMap() or if another
   * Widget gets set.
   */
  void SetWidget(Widget *_widget);

  void UpdateGaugeVisibility();

  gcc_pure
  const MapWindowProjection &GetProjection() const;

  void ToggleSuppressFLARMRadar();
  void ToggleForceFLARMRadar();

private:
  void UpdateTrafficGaugeVisibility();

protected:
  virtual void on_resize(UPixelScalar width, UPixelScalar height);
  bool on_activate();
  void on_setfocus();
  virtual bool on_key_down(unsigned key_code);
  bool on_timer(WindowTimer &timer);
  virtual bool on_user(unsigned id);
  void on_create();
  void on_destroy();
  bool on_close();

#ifdef ANDROID
  virtual void on_pause();
#endif
};

#endif
