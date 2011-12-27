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

#ifndef XCSOAR_WAY_POINT_GLUE_HPP
#define XCSOAR_WAY_POINT_GLUE_HPP

#include <tchar.h>

class Waypoints;
class RasterTerrain;
class OperationEnvironment;
struct ComputerSettings;

class WaypointReaderBase;

/**
 * This class is used to parse different waypoint files
 */
namespace WaypointGlue {
  /**
   * This functions checks if the home and teamcode waypoint
   * indices exist and if necessary tries to find new ones in the waypoint list
   * @param way_points Waypoint list
   * @param terrain RasterTerrain (for placing the aircraft
   * in the middle of the terrain if no home was found)
   * @param settings SETTING_COMPUTER (for determining the
   * special waypoint indices)
   * @param reset This should be true if the waypoint file was changed,
   * it resets all special waypoints indices
   */
  void SetHome(Waypoints &way_points, const RasterTerrain *terrain,
               ComputerSettings &settings, const bool reset);

  /**
   * Reads the waypoints out of the two waypoint files and appends them to the
   * specified waypoint list
   * @param way_points The waypoint list to fill
   * @param terrain RasterTerrain (for automatic waypoint height)
   */
  bool LoadWaypoints(Waypoints &way_points,
                     const RasterTerrain *terrain,
                     OperationEnvironment &operation);
  bool LoadWaypointFile(int num, Waypoints &way_points,
                        const RasterTerrain *terrain,
                        OperationEnvironment &operation);
  bool LoadMapFileWaypoints(int num, const TCHAR* key,
                            Waypoints &way_points, const RasterTerrain *terrain,
                            OperationEnvironment &operation);
  bool SaveWaypoints(const Waypoints &way_points);
  bool SaveWaypointFile(const Waypoints &way_points, int num);

  bool IsWritable();
};

#endif
