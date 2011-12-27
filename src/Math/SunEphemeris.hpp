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

#ifndef SUN_EPHEMERIS_HPP
#define SUN_EPHEMERIS_HPP

#include "Math/fixed.hpp"
#include "Math/Angle.hpp"
#include "Compiler.h"

struct GeoPoint;
struct BrokenDateTime;

/**
 * Sun ephemeris model, used largely for calculations of sunset times
 * @see http://www.sci.fi/~benefon/azimalt.cpp
 */
namespace SunEphemeris
{
  struct Result {
    fixed day_length, morning_twilight, evening_twilight;
    fixed time_of_noon, time_of_sunset, time_of_sunrise;
    Angle azimuth;
  };

  /**
   * Calculates all sun-related important times
   * depending on time of year and location
   * @param Location Location to be used in calculation
   * @param Basic NMEA_INFO for current date
   * @param Calculated DERIVED_INFO (not yet used)
   * @param TimeZone The timezone
   * @return Sunset time
   */
  Result CalcSunTimes(const GeoPoint &location, const BrokenDateTime &date_time,
                      fixed time_zone);
}

#endif
