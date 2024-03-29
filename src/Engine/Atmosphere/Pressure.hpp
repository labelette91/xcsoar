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

#ifndef XCSOAR_ATMOSPHERE_PRESSURE_H
#define XCSOAR_ATMOSPHERE_PRESSURE_H

#include "Math/fixed.hpp"
#include "Compiler.h"

/**
 * ICAO Standard Atmosphere calculations (valid in Troposphere, alt<11000m)
 *
 */
class AtmosphericPressure 
{
  /** Pressure at sea level, hPa */
  fixed qnh;

  /**
   * @param qnh the QNH in hPa
   */
  explicit gcc_constexpr_ctor
  AtmosphericPressure(fixed _qnh):qnh(_qnh) {}

public:
  /**
   * Non-initialising constructor.
   */
  AtmosphericPressure() = default;

  /**
   * Returns an object representing the standard pressure (1013.25
   * hPa).
   */
  static gcc_constexpr_function
  AtmosphericPressure Standard() {
    return AtmosphericPressure(fixed(1013.25));
  }

  static gcc_constexpr_function
  AtmosphericPressure Pascal(fixed value) {
    return AtmosphericPressure(value / 100);
  }

  static gcc_constexpr_function
  AtmosphericPressure HectoPascal(fixed value) {
    return AtmosphericPressure(value);
  }

  fixed GetPascal() {
    return GetHectoPascal() * 100;
  }

  /**
   * Access QNH value
   *
   * @return QNH value (hPa)
   */
  fixed GetHectoPascal() const {
    return qnh;
  }

  /**
   * Calculates the current QNH by comparing a pressure value to a
   * known altitude of a certain location
   *
   * @param pressure Current pressure
   * @param alt_known Altitude of a known location (m)
   */
  gcc_const
  static AtmosphericPressure FindQNHFromPressure(const AtmosphericPressure pressure,
                                                 const fixed alt_known);

  /**
   * Converts altitude with QNH=1013.25 reference to QNH adjusted altitude
   * @param alt 1013.25-based altitude (m)
   * @return QNH-based altitude (m)
   */
  gcc_pure
  fixed PressureAltitudeToQNHAltitude(const fixed alt) const;

  /**
   * Converts QNH adjusted altitude to pressure altitude (with QNH=1013.25 as reference)
   * @param alt QNH-based altitude(m)
   * @return pressure altitude (m)
   */
  gcc_pure
  fixed QNHAltitudeToPressureAltitude(const fixed alt) const;

  /**
   * Calculates the air density from a given QNH-based altitude
   * @param altitude QNH-based altitude (m)
   * @return Air density (kg/m^3)
   */
  gcc_pure
  static fixed AirDensity(const fixed altitude);

  /**
   * Divide TAS by this number to get IAS
   * @param altitude QNH-based altitude (m)
   * @return Ratio of TAS to IAS
   */
  gcc_pure
  static fixed AirDensityRatio(const fixed altitude);

  /**
   * Converts a pressure value to the corresponding QNH-based altitude
   *
   * See http://wahiduddin.net/calc/density_altitude.htm
   *
   * Example:
   * QNH=1014, ps=100203 => alt = 100
   * @see QNHAltitudeToStaticPressure
   * @param ps Air pressure
   * @return Altitude over QNH-based zero (m)
   */
  gcc_pure
  fixed StaticPressureToQNHAltitude(const AtmosphericPressure ps) const;

  /**
   * Converts a QNH-based altitude to the corresponding pressure
   *
   * See http://wahiduddin.net/calc/density_altitude.htm
   *
   * Example:
   * alt= 100, QNH=1014 => ps = 100203 Pa
   * @see StaticPressureToAltitude
   * @param alt Altitude over QNH-based zero (m)
   * @return Air pressure at given altitude
   */
  gcc_pure
  AtmosphericPressure QNHAltitudeToStaticPressure(const fixed alt) const;

  /**
   * Converts a pressure value to pressure altitude (with QNH=1013.25 as reference)
   * @param ps Air pressure
   * @return pressure altitude (m)
   */
  gcc_const
  static fixed StaticPressureToPressureAltitude(const AtmosphericPressure ps);

  /**
   * Converts a 1013.25 hPa based altitude to the corresponding pressure
   *
   * @see StaticPressureToAltitude
   * @param alt Altitude over 1013.25 hPa based zero(m)
   * @return Air pressure at given altitude
   */
  gcc_const
  static AtmosphericPressure PressureAltitudeToStaticPressure(const fixed alt);
};

#endif
