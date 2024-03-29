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

#ifndef XCSOAR_ROUGH_VSPEED_HPP
#define XCSOAR_ROUGH_VSPEED_HPP

#include "Math/fixed.hpp"
#include "Util/TypeTraits.hpp"
#include "Compiler.h"

#include <stdint.h>

/**
 * Store an rough vertical speed value, when the exact value is not
 * needed.
 *
 * The accuracy is 1/256 m/s.
 */
class RoughVSpeed {
  int16_t value;

  gcc_constexpr_function
  static int16_t Import(fixed x) {
    return (int16_t)(x * 256);
  }

  gcc_constexpr_function
  static fixed Export(int16_t x) {
    return fixed(x) / 256;
  }

public:
  RoughVSpeed() = default;

  gcc_constexpr_ctor
  RoughVSpeed(fixed _value):value(Import(_value)) {}

  RoughVSpeed &operator=(fixed other) {
    value = Import(other);
    return *this;
  }

  gcc_constexpr_method
  operator fixed() const {
    return Export(value);
  }
};

static_assert(is_trivial<RoughVSpeed>::value, "type is not trivial");

#endif
