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
#ifndef AIRCRAFT_SIM_HPP
#define AIRCRAFT_SIM_HPP

#include "Navigation/Aircraft.hpp"

class AircraftSim {
  AircraftState state, state_last;

  fixed random_mag;

public:
  const AircraftState& GetState() const {
    return state;
  }
  const AircraftState& GetLastState() const {
    return state_last;
  }
  AircraftState& GetState() {
    return state;
  }

  void SetWind(const fixed speed, const Angle direction);

  void Start(const GeoPoint& location_start,
             const GeoPoint& location_last,
             const fixed& altitude);

  bool Update(const Angle heading, const fixed timestep=fixed_one);

  fixed GetTime() const {
    return state.time;
  }

private:
  void Integrate(const Angle heading, const fixed timestep);

  GeoPoint GetEndPoint(const Angle heading, const fixed timestep) const;
};

#endif
