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
#ifndef ROUTELINK_HPP
#define ROUTELINK_HPP

#include <limits.h>
#include "Config.hpp"
#include "Math/fixed.hpp"
#include "Math/Angle.hpp"
#include "Navigation/SearchPoint.hpp"
#include "Rough/RoughAltitude.hpp"

#include <utility>

class GlidePolar;
struct GlideResult;
class TaskProjection;
class RasterMap;
struct SpeedVector;

typedef AFlatGeoPoint RoutePoint;

/**
 * Class used for primitive 3d navigation links.
 *
 * For route planning, these routes are defined in reverse time order,
 * that is, the first link is the destination (later in time), second link
 * is the origin (earlier in time).
 */
struct RouteLinkBase {
  /** Destination location */
  RoutePoint first;
  /** Origin location */
  RoutePoint second;

  RouteLinkBase(const RoutePoint& _dest, const RoutePoint& _origin)
    :first(_dest), second(_origin) {}

  /**
   * Equality comparison operator
   *
   * @param o object to compare to
   *
   * @return true if origins and destinations are equal
   */
  gcc_pure
  bool operator==(const RouteLinkBase &o) const {
    return (first == o.first) && (second == o.second);
  }

  /**
   * Ordering operator, used for set ordering.  Uses lexicographic comparison.
   *
   * @param o object to compare to
   *
   * @return true if lexicographically smaller
   */
  gcc_pure
  bool
  operator<(const RouteLinkBase &o) const
  {
    if (first.Longitude != o.first.Longitude)
      return first.Longitude < o.first.Longitude;
    if (first.Latitude != o.first.Latitude)
      return first.Latitude < o.first.Latitude;
    if (first.altitude != o.first.altitude)
      return first.altitude < o.first.altitude;

    if (second.Longitude != o.second.Longitude)
      return second.Longitude < o.second.Longitude;
    if (second.Latitude != o.second.Latitude)
      return second.Latitude < o.second.Latitude;
    if (second.altitude != o.second.altitude)
      return second.altitude < o.second.altitude;

    return false;
   }

  /**
   * Return 2d Distance of this link
   * @return distance in FlatGeoPoint units
   */
  gcc_pure
  unsigned Distance() const {
    return first.Distance(second);
  }

  /**
   * Test whether this link is too short to be considered
   * for path planning (based on manhattan distance).
   *
   * @return true if this link is short
   */
  gcc_pure
  bool IsShort() const;

  /**
   * Calculate the dot product of this link with another.
   * Can be used to test projection of one link in direction of
   * another.
   *
   * @param o second object in dot product
   *
   * @return dot product of this object with second object
   */
  gcc_pure
  int DotProduct(const RouteLinkBase& o) const {
    return (o.second-o.first).DotProduct(second-first);
  }

  /**
   * Calculate the cross product of this link with another.
   * Can be used to test orthogonality of two links.
   *
   * @param o second object in cross product
   *
   * @return cross product of this object with second object
   */
  gcc_pure
  int CrossProduct(const RouteLinkBase& o) const {
    return (o.second - o.first).CrossProduct(second - first);
  }
};

/**
 * Extension of RouteLinkBase to store additional data
 * on actual distance, reciprocal of distance, and direction indices
 * for fast lookup of performance via RoutePolars.
 */
struct RouteLink: public RouteLinkBase {
public:
  /** Distance (m) */
  fixed d;
  /** Reciprocal of Distance (1/m) */
  fixed inv_d;
  /** Direction index to be used for RoutePolar lookups */
  unsigned polar_index;

  RouteLink(const RouteLinkBase& link, const TaskProjection& proj);
  RouteLink (const RoutePoint& _first, const RoutePoint& _second,
             const TaskProjection& proj);

  /**
   * Generate RouteLink projected flat such that the destination altitude equals
   * the start altitude.  The start altitude is unaffected.
   *
   * @return link equivalent to this link flattened
   */
  RouteLink Flat() const;

private:
  void CalcSpeedups(const TaskProjection& proj);
};

#endif
