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

#ifndef XCSOAR_SCREEN_OPENGL_POINT_HPP
#define XCSOAR_SCREEN_OPENGL_POINT_HPP

#include "Screen/OpenGL/Types.hpp"
#include "Screen/OpenGL/Features.hpp"

#ifdef HAVE_GLES
#define RASTER_POINT_SIZE 4
#else
#define RASTER_POINT_SIZE 8
#endif

typedef GLvalue PixelScalar;
typedef GLuvalue UPixelScalar;

struct RasterPoint {
  /**
   * Type to be used by vector math, where a range of
   * max(GLvalue)*max(GLvalue) is needed.
   */
#ifdef HAVE_GLES
  typedef int SquareType;
#else
  typedef float SquareType;
#endif
  PixelScalar x, y;
};

struct PixelSize {
  PixelScalar cx, cy;

  bool operator==(const PixelSize &other) const {
    return cx == other.cx && cy == other.cy;
  }

  bool operator!=(const PixelSize &other) const {
    return !(*this == other);
  }
};

struct PixelRect {
  PixelScalar left, top, right, bottom;
};

#endif
