/* Copyright_License {

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

#include "AirspacePolygon.hpp"
#include "Math/Earth.hpp"
#include "Navigation/Geometry/GeoVector.hpp"
#include "Navigation/Flat/FlatBoundingBox.hpp"
#include "Navigation/TaskProjection.hpp"
#include "AirspaceIntersectSort.hpp"

#include <assert.h>

AirspacePolygon::AirspacePolygon(const std::vector<GeoPoint> &pts,
                                 const bool prune)
  :AbstractAirspace(Shape::POLYGON)
{
  if (pts.size() < 2) {
    m_is_convex = true;
  } else {
    m_border.reserve(pts.size() + 1);

    for (auto v = pts.begin(); v != pts.end(); ++v)
      m_border.push_back(SearchPoint(*v));

    // ensure airspace is closed
    GeoPoint p_start = pts[0];
    GeoPoint p_end = *(pts.end() - 1);
    if (p_start != p_end)
      m_border.push_back(SearchPoint(p_start));


    if (prune) {
      // only for testing
      m_border.PruneInterior();
      m_is_convex = true;
    } else {
      m_is_convex = m_border.IsConvex();
    }
  }
}

const GeoPoint 
AirspacePolygon::GetCenter() const
{
  if (m_border.empty())
    return GeoPoint(Angle::Zero(), Angle::Zero());

  return m_border[0].get_location();
}

bool 
AirspacePolygon::Inside(const GeoPoint &loc) const
{
  return m_border.IsInside(loc);
}

AirspaceIntersectionVector
AirspacePolygon::Intersects(const GeoPoint &start, const GeoPoint &end) const
{
  const FlatRay ray(m_task_projection->project(start),
                    m_task_projection->project(end));

  AirspaceIntersectSort sorter(start, end, *this);

  for (auto it = m_border.begin(); it + 1 != m_border.end(); ++it) {

    const FlatRay r_seg(it->get_flatLocation(), (it + 1)->get_flatLocation());
    fixed t;
    if (ray.IntersectsDistinct(r_seg, t))
      sorter.add(t, m_task_projection->unproject(ray.Parametric(t)));
  }

  return sorter.all();
}

GeoPoint 
AirspacePolygon::ClosestPoint(const GeoPoint &loc) const
{
  const FlatGeoPoint p = m_task_projection->project(loc);
  const FlatGeoPoint pb = m_border.NearestPoint(p);
  return m_task_projection->unproject(pb);
}
