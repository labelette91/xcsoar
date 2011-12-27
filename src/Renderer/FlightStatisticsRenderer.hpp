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

#ifndef FLIGHT_STATISTICS_RENDERER_HPP
#define FLIGHT_STATISTICS_RENDERER_HPP

#include "Screen/Point.hpp"
#include "Renderer/TrailRenderer.hpp"

#include <tchar.h>

struct NMEAInfo;
struct DerivedInfo;
class ClimbHistory;
class TracePointVector;
struct TaskBehaviour;
struct ComputerSettings;
struct MapSettings;
class Canvas;
class WindStore;
class Airspaces;
class RasterTerrain;
class GlidePolar;
class Chart;
class TaskManager;
class ProtectedTaskManager;
class TraceComputer;
class FlightStatistics;
struct ContestStatistics;
struct ChartLook;
struct MapLook;
class CuSonde;

class FlightStatisticsRenderer {
  const FlightStatistics &fs;
  const ChartLook &chart_look;
  const MapLook &map_look;

  mutable TrailRenderer trail_renderer;

public:
  FlightStatisticsRenderer(const FlightStatistics &_flight_statistics,
                           const ChartLook &_chart_look,
                           const MapLook &_map_look);

public:
  void RenderBarograph(Canvas &canvas, const PixelRect rc,
                       const NMEAInfo &nmea_info,
                       const DerivedInfo &derived_info,
                       const ProtectedTaskManager *task) const;

  void RenderBarographSpark(Canvas &canvas, const PixelRect rc, bool inverse,
                            const NMEAInfo &nmea_info,
                            const DerivedInfo &derived_info,
                            const ProtectedTaskManager *task) const;

  void RenderClimb(Canvas &canvas, const PixelRect rc,
                   const GlidePolar& glide_polar) const;

  void RenderGlidePolar(Canvas &canvas, const PixelRect rc,
                        const ClimbHistory &climb_history,
                        const ComputerSettings &settings_computer,
                        const GlidePolar& glide_polar) const;

  void RenderWind(Canvas &canvas, const PixelRect rc,
                  const NMEAInfo &nmea_info,
                  const WindStore &wind_store) const;

  void RenderTemperature(Canvas &canvas, const PixelRect rc,
                         const CuSonde &cu_sonde) const;

  void RenderOLC(Canvas &canvas, const PixelRect rc,
                 const NMEAInfo &nmea_info,
                 const DerivedInfo &calculated,
                 const ComputerSettings &settings_computer,
                 const MapSettings &settings_map,
                 const ContestStatistics &contest,
                 const TraceComputer &trace_computer) const;

  void RenderTask(Canvas &canvas, const PixelRect rc,
                  const NMEAInfo &nmea_info,
                  const DerivedInfo &derived_info,
                  const ComputerSettings &settings_computer,
                  const MapSettings &settings_map,
                  const ProtectedTaskManager &task,
                  const TraceComputer *trace_computer) const;

  void RenderSpeed(Canvas &canvas, const PixelRect rc,
                   const NMEAInfo &nmea_info,
                   const DerivedInfo &derived_info,
                   const TaskManager &task) const;

  void CaptionBarograph(TCHAR *sTmp);
  void CaptionClimb(TCHAR* sTmp);
  void CaptionPolar(TCHAR * sTmp, const GlidePolar& glide_polar) const;

  void CaptionTempTrace(TCHAR *sTmp, const CuSonde &cu_sonde) const;
  void CaptionTask(TCHAR *sTmp, const DerivedInfo &derived) const;
  void CaptionOLC(TCHAR *sTmp, const TaskBehaviour &task_behaviour,
                  const DerivedInfo &derived) const;
};

#endif
