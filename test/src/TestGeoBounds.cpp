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

#include "Geo/GeoBounds.hpp"
#include "TestUtil.hpp"

#include <stdio.h>

int main(int argc, char **argv)
{
  plan_tests(24);

  GeoPoint g(Angle::Degrees(fixed(2)), Angle::Degrees(fixed(4)));

  GeoBounds b(g);

  ok1(equals(b.east, 2));
  ok1(equals(b.west, 2));
  ok1(equals(b.north, 4));
  ok1(equals(b.south, 4));

  ok1(b.IsEmpty());

  g.latitude = Angle::Degrees(fixed(6));
  g.longitude = Angle::Degrees(fixed(8));
  b.Extend(g);

  ok1(equals(b.east, 8));
  ok1(equals(b.west, 2));
  ok1(equals(b.north, 6));
  ok1(equals(b.south, 4));

  ok1(!b.IsEmpty());

  g = b.GetCenter();
  ok1(equals(g.latitude, 5));
  ok1(equals(g.longitude, 5));

  ok1(b.IsInside(Angle::Degrees(fixed(7)), Angle::Degrees(fixed(4.5))));
  ok1(!b.IsInside(Angle::Degrees(fixed(9)), Angle::Degrees(fixed(4.5))));
  ok1(!b.IsInside(Angle::Degrees(fixed(7)), Angle::Degrees(fixed(1))));
  ok1(!b.IsInside(Angle::Degrees(fixed(9)), Angle::Degrees(fixed(1))));

  b = b.Scale(fixed(2));

  ok1(equals(b.east, 11));
  ok1(equals(b.west, -1));
  ok1(equals(b.north, 7));
  ok1(equals(b.south, 3));

  b = b.Scale(fixed(0.5));

  ok1(equals(b.east, 8));
  ok1(equals(b.west, 2));
  ok1(equals(b.north, 6));
  ok1(equals(b.south, 4));

  return exit_status();
}
