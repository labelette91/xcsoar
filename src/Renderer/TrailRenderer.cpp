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

#include "TrailRenderer.hpp"
#include "Look/TrailLook.hpp"
#include "Screen/Canvas.hpp"
#include "NMEA/Info.hpp"
#include "NMEA/Derived.hpp"
#include "MapSettings.hpp"
#include "Computer/TraceComputer.hpp"
#include "Projection/WindowProjection.hpp"
#include "Engine/Math/Earth.hpp"
#include "Engine/Contest/ContestResult.hpp"

#include <algorithm>

using std::min;
using std::max;

bool
TrailRenderer::LoadTrace(const TraceComputer &trace_computer)
{
  trace.clear();
  trace_computer.LockedCopyTo(trace);
  return !trace.empty();
}

bool
TrailRenderer::LoadTrace(const TraceComputer &trace_computer,
                         unsigned min_time,
                         const WindowProjection &projection)
{
  trace.clear();
  trace_computer.LockedCopyTo(trace, min_time,
                              projection.GetGeoScreenCenter(),
                              projection.DistancePixelsToMeters(3));
  return !trace.empty();
}

TaskProjection
TrailRenderer::GetBounds(const GeoPoint fallback_location) const
{
  return get_bounds(trace, fallback_location);
}

/**
 * This function returns the corresponding SnailTrail
 * color array index to the input
 * @param cv Input value between -1.0 and 1.0
 * @return SnailTrail color array index
 */
gcc_const
static int
GetSnailColorIndex(fixed cv)
{
  return max((short)0, min((short)(TrailLook::NUMSNAILCOLORS - 1),
                           (short)((cv + fixed_one) / 2 * TrailLook::NUMSNAILCOLORS)));
}

void
TrailRenderer::Draw(Canvas &canvas, const TraceComputer &trace_computer,
                    const WindowProjection &projection, unsigned min_time,
                    bool enable_traildrift, const RasterPoint pos,
                    const NMEAInfo &basic, const DerivedInfo &calculated,
                    const MapSettings &settings)
{
  if (settings.trail_length == TRAIL_OFF)
    return;

  if (!LoadTrace(trace_computer, min_time, projection))
    return;

  if (!calculated.wind_available)
    enable_traildrift = false;

  GeoPoint traildrift;
  if (enable_traildrift) {
    GeoPoint tp1 = FindLatitudeLongitude(basic.location,
                                         calculated.wind.bearing,
                                         calculated.wind.norm);
    traildrift = basic.location - tp1;
  }

  fixed value_max, value_min;

  if (settings.snail_type == stAltitude) {
    value_max = fixed(1000);
    value_min = fixed(500);
    for (auto it = trace.begin(); it != trace.end(); ++it) {
      value_max = max(it->GetAltitude(), value_max);
      value_min = min(it->GetAltitude(), value_min);
    }
  } else {
    value_max = fixed(0.75);
    value_min = fixed(-2.0);
    for (auto it = trace.begin(); it != trace.end(); ++it) {
      value_max = max(it->GetVario(), value_max);
      value_min = min(it->GetVario(), value_min);
    }
    value_max = min(fixed(7.5), value_max);
    value_min = max(fixed(-5.0), value_min);
  }

  bool scaled_trail = settings.snail_scaling_enabled &&
                      projection.GetMapScale() <= fixed_int_constant(6000);

  const GeoBounds bounds = projection.GetScreenBounds().Scale(fixed_four);

  RasterPoint last_point;
  bool last_valid = false;
  for (auto it = trace.begin(), end = trace.end(); it != end; ++it) {
    const GeoPoint gp = enable_traildrift
      ? it->get_location().Parametric(traildrift,
                                      it->CalculateDrift(basic.time))
      : it->get_location();
    if (!bounds.IsInside(gp)) {
      /* the point is outside of the MapWindow; don't paint it */
      last_valid = false;
      continue;
    }

    RasterPoint pt = projection.GeoToScreen(gp);

    if (last_valid) {
      if (settings.snail_type == stAltitude) {
        unsigned index((it->GetAltitude() - value_min) / (value_max - value_min)
                       * (TrailLook::NUMSNAILCOLORS - 1));
        index = max(0u, min(TrailLook::NUMSNAILCOLORS - 1, index));
        canvas.Select(look.hpSnail[index]);
      } else {
        const fixed colour_vario = negative(it->GetVario())
          ? - it->GetVario() / value_min
          : it->GetVario() / value_max ;

        if (!scaled_trail)
          canvas.Select(look.hpSnail[GetSnailColorIndex(colour_vario)]);
        else
          canvas.Select(look.hpSnailVario[GetSnailColorIndex(colour_vario)]);
      }
      canvas.line_piece(last_point, pt);
    }
    last_point = pt;
    last_valid = true;
  }

  canvas.line(last_point, pos);
}

void
TrailRenderer::DrawTraceVector(Canvas &canvas, const Projection &projection,
                               const TracePointVector &trace)
{
  points.GrowDiscard(trace.size());

  unsigned n = 0;
  for (auto i = trace.begin(), end = trace.end(); i != end; ++i)
    points[n++] = projection.GeoToScreen(i->get_location());

  canvas.DrawPolyline(points.begin(), n);
}

void
TrailRenderer::Draw(Canvas &canvas, const WindowProjection &projection)
{
  canvas.Select(look.trace_pen);
  DrawTraceVector(canvas, projection, trace);
}

void
TrailRenderer::Draw(Canvas &canvas, const TraceComputer &trace_computer,
                    const WindowProjection &projection,
                    unsigned min_time)
{
  if (LoadTrace(trace_computer, min_time, projection))
    Draw(canvas, projection);
}

void
TrailRenderer::Draw(Canvas &canvas, const WindowProjection &projection,
                    const ContestTraceVector &trace)
{
  points.GrowDiscard(trace.size());

  unsigned n = 0;
  for (auto i = trace.begin(), end = trace.end(); i != end; ++i)
    points[n++] = projection.GeoToScreen(i->get_location());

  canvas.DrawPolyline(points.begin(), n);
}

