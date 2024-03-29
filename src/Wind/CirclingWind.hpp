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

#ifndef XCSOAR_CIRCLING_WIND_HPP
#define XCSOAR_CIRCLING_WIND_HPP

#include "Vector.hpp"
#include "Navigation/GeoPoint.hpp"
#include "Util/StaticArray.hpp"

struct MoreData;
struct DerivedInfo;

/**
 * Class to provide wind estimates from circling
 */
class CirclingWind
{
  /**
   * The windanalyser analyses the list of flightsamples looking for
   * windspeed and direction.
   */
  struct Sample
  {
    Vector v;
    fixed time;
    fixed mag;
  };

  // we are counting the number of circles, the first onces are probably not very round
  int circle_count;
  // true = left, false = right
  bool circling_left;
  // active is set to true or false by the slot_newFlightMode slot
  bool active;
  int circle_deg;
  Angle last_track;
  bool past_halfway;
  Vector min_vector;
  Vector max_vector;
  bool current_mode_ok;
  bool first;
  int start_circle;

  GeoPoint climb_startpos;
  GeoPoint climb_endpos;
  fixed climb_starttime;
  fixed climb_endtime;

  StaticArray<Sample, 50> samples;

public:
  struct Result
  {
    unsigned quality;
    Vector wind;

    Result() {}
    Result(int _quality):quality(_quality) {}
    Result(int _quality, Vector _wind):quality(_quality), wind(_wind) {}

    bool IsValid() const {
      return quality > 0;
    }
  };

  /**
   * Clear as if never flown
   */
  void Reset();

  /**
   * Called if the flightmode changes
   */
  void NewFlightMode(const DerivedInfo &derived, bool left, int marker);

  /**
   * Called if a new sample is available in the samplelist.
   */
  Result NewSample(const MoreData &info);

private:
  Result CalcWind();
};

#endif
