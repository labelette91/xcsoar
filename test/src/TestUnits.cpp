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

#include "Units/Units.hpp"
#include "TestUtil.hpp"

int main(int argc, char **argv)
{
  plan_tests(12);

  UnitSetting &config = Units::current;

  config.distance_unit = unMeter;
  ok1(equals(Units::ToUserDistance(fixed_one), 1));

  config.distance_unit = unKiloMeter;
  ok1(equals(Units::ToUserDistance(fixed_one), 0.001));

  config.temperature_unit = unKelvin;
  ok1(equals(Units::ToUserTemperature(fixed_zero), fixed_zero));
  ok1(equals(Units::ToSysTemperature(fixed_zero), fixed_zero));

  config.temperature_unit = unGradCelcius;
  ok1(equals(Units::ToUserTemperature(fixed_zero), -273.15));
  ok1(equals(Units::ToUserTemperature(fixed(20)), -253.15));
  ok1(equals(Units::ToSysTemperature(fixed_zero), 273.15));
  ok1(equals(Units::ToSysTemperature(fixed(20)), 293.15));

  config.temperature_unit = unGradFahrenheit;
  ok1(equals(Units::ToUserTemperature(fixed_zero), -459.67));
  ok1(equals(Units::ToSysTemperature(fixed_zero), 255.37));

  ok1(equals(Units::ToUserUnit(fixed(1013.25), unTorr), 760));
  ok1(equals(Units::ToUserUnit(fixed(1013.25), unInchMercury), 29.92));

  return exit_status();
}
