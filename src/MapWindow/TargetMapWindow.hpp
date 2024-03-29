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

#ifndef XCSOAR_TARGET_MAP_WINDOW_HPP
#define XCSOAR_TARGET_MAP_WINDOW_HPP

#include "Projection/MapWindowProjection.hpp"
#include "Renderer/AirspaceRenderer.hpp"
#include "Screen/BufferWindow.hpp"
#include "Screen/LabelBlock.hpp"
#include "MapWindowBlackboard.hpp"
#include "Renderer/BackgroundRenderer.hpp"
#include "Renderer/WaypointRenderer.hpp"
#include "Renderer/TrailRenderer.hpp"
#include "Compiler.h"

#ifndef ENABLE_OPENGL
#include "Screen/BufferCanvas.hpp"
#endif

struct WaypointLook;
struct TaskLook;
struct AircraftLook;
class ContainerWindow;
class TopographyStore;
class TopographyRenderer;
class Waypoints;
class Airspaces;
class ProtectedTaskManager;
class GlideComputer;

class TargetMapWindow : public BufferWindow {
  const TaskLook &task_look;
  const AircraftLook &aircraft_look;

#ifndef ENABLE_OPENGL
  // graphics vars

  BufferCanvas buffer_canvas;
  BufferCanvas stencil_canvas;
#endif

  MapWindowProjection projection;

  LabelBlock label_block;

  BackgroundRenderer background;
  TopographyRenderer *topography_renderer;

  AirspaceRenderer airspace_renderer;

  WaypointRenderer way_point_renderer;

  TrailRenderer trail_renderer;

  ProtectedTaskManager *task;
  const GlideComputer *glide_computer;

  unsigned target_index;

  enum DragMode {
    DRAG_NONE,
    DRAG_TARGET,
  } drag_mode;

  RasterPoint drag_start, drag_last;

public:
  TargetMapWindow(const WaypointLook &waypoint_look,
                  const AirspaceLook &airspace_look,
                  const TrailLook &trail_look,
                  const TaskLook &task_look,
                  const AircraftLook &aircraft_look);
  virtual ~TargetMapWindow();

  void set(ContainerWindow &parent, PixelScalar left, PixelScalar top,
           UPixelScalar width, UPixelScalar height, WindowStyle style);

  void SetTerrain(RasterTerrain *terrain);
  void SetTopograpgy(TopographyStore *topography);

  void SetAirspaces(Airspaces *airspace_database) {
    airspace_renderer.SetAirspaces(airspace_database);
  }

  void SetWaypoints(const Waypoints *way_points) {
    way_point_renderer.set_way_points(way_points);
  }

  void SetTask(ProtectedTaskManager *_task) {
    task = _task;
  }

  void SetGlideComputer(const GlideComputer *_gc) {
    glide_computer = _gc;
  }

  void SetTarget(unsigned index);

private:
  /**
   * Renders the terrain background
   * @param canvas The drawing canvas
   */
  void RenderTerrain(Canvas &canvas);

  /**
   * Renders the topography
   * @param canvas The drawing canvas
   */
  void RenderTopography(Canvas &canvas);

  /**
   * Renders the topography labels
   * @param canvas The drawing canvas
   */
  void RenderTopographyLabels(Canvas &canvas);

  /**
   * Renders the airspace
   * @param canvas The drawing canvas
   */
  void RenderAirspace(Canvas &canvas);

  void RenderTrail(Canvas &canvas);

  void DrawWaypoints(Canvas &canvas);

  void DrawTask(Canvas &canvas);

private:
  /**
   * If PanTarget, paints target during drag
   * Used by dlgTarget
   *
   * @param drag_last location of target
   * @param canvas
   */
  void TargetPaintDrag(Canvas &canvas, const RasterPoint last_drag);

  /**
   * If PanTarget, tests if target is clicked
   * Used by dlgTarget
   *
   * @param drag_last location of click
   *
   * @return true if click is near target
   */
  bool isClickOnTarget(const RasterPoint drag_last);

  /**
   * If PanTarget, tests if drag destination
   * is in OZ of target being edited
   * Used by dlgTarget
   *
   * @param x mouse_up location
   * @param y mouse_up location
   *
   * @return true if location is in OZ
   */
  bool isInSector(const int x, const int y);

  /**
   * If PanTarget, updates task with new target
   * Used by dlgTarget
   *
   * @param x mouse_up location
   * @param y mouse_up location
   *
   * @return true if successful
   */
  bool TargetDragged(const int x, const int y);

protected:
  virtual void on_create();
  virtual void on_destroy();
  virtual void on_resize(UPixelScalar width, UPixelScalar height);

  virtual void on_paint_buffer(Canvas& canvas);
  virtual void on_paint(Canvas& canvas);

  virtual bool on_cancel_mode();

  virtual bool on_mouse_down(PixelScalar x, PixelScalar y);
  virtual bool on_mouse_up(PixelScalar x, PixelScalar y);
  virtual bool on_mouse_move(PixelScalar x, PixelScalar y, unsigned keys);
};

#endif
