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

#ifndef XCSOAR_UNITS_SETTINGS_HPP
#define XCSOAR_UNITS_SETTINGS_HPP

#include "Compiler.h"

enum CoordinateFormats {
  CF_DDMMSS = 0,
  CF_DDMMSS_SS,
  CF_DDMM_MMM,
  CF_DD_DDDD,
};

enum Unit {
  unUndef,
  unKiloMeter,
  unNauticalMiles,
  unStatuteMiles,
  unKiloMeterPerHour,
  unKnots,
  unStatuteMilesPerHour,
  unMeterPerSecond,
  unFeetPerMinute,
  unMeter,
  unFeet,
  unFlightLevel,
  unKelvin,
  unGradCelcius, // K = C° + 273,15
  unGradFahrenheit, // K = (°F + 459,67) / 1,8
  unHectoPascal,
  unMilliBar,
  unTorr,
  unInchMercury,

  /**
   * The sentinel: the number of units in this enum.
   */
  unCount
};

enum UnitGroup
{
  ugNone,
  ugDistance,
  ugAltitude,
  ugTemperature,
  ugHorizontalSpeed,
  ugVerticalSpeed,
  ugWindSpeed,
  ugTaskSpeed,
  ugPressure,
};

struct UnitSetting
{
  /** Unit for distances */
  Unit distance_unit;
  /** Unit for altitudes, heights */
  Unit altitude_unit;
  /** Unit for temperature */
  Unit temperature_unit;
  /** Unit for aircraft speeds */
  Unit speed_unit;
  /** Unit for vertical speeds, varios */
  Unit vertical_speed_unit;
  /** Unit for wind speeds */
  Unit wind_speed_unit;
  /** Unit for task speeds */
  Unit task_speed_unit;
  /** Unit for pressures */
  Unit pressure_unit;

  /** Unit for lat/lon */
  CoordinateFormats coordinate_format;

  void SetDefaults();

  /**
   * Return the configured unit for a given group.
   */
  gcc_pure
  Unit GetByGroup(UnitGroup group) const;
};

#endif
