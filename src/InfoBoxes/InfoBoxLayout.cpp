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

#include "InfoBoxes/InfoBoxLayout.hpp"
#include "Profile/Profile.hpp"
#include "Util/Macros.hpp"

#include <stdio.h>

/**
 * The number of info boxes in each geometry.
 */
static const unsigned char geometry_counts[] = {
  8, 8, 8, 8, 8, 8,
  9, 5, 12, 24, 12,
  12, 9, 8,
};

namespace InfoBoxLayout
{
  Geometry InfoBoxGeometry = ibTop4Bottom4;

  gcc_const
  static Geometry
  ValidateGeometry(Geometry geometry, UPixelScalar width, UPixelScalar height);

  gcc_pure
  static Geometry
  LoadGeometryFromProfile(UPixelScalar width, UPixelScalar height);

  static void
  CalcInfoBoxSizes(Layout &layout,
                   PixelRect rc, InfoBoxLayout::Geometry geometry);
}

void
InfoBoxLayout::Init(PixelRect rc)
{
  InfoBoxGeometry = LoadGeometryFromProfile(rc.right - rc.left,
                                            rc.bottom - rc.top);
}

static int
MakeTopRow(const InfoBoxLayout::Layout &layout,
           PixelRect *p, unsigned n, PixelScalar left, PixelScalar top)
{
  PixelRect *const end = p + n;
  const PixelScalar bottom = top + layout.control_height;
  while (p < end) {
    p->left = left;
    left += layout.control_width;
    p->right = left;
    p->top = top;
    p->bottom = bottom;

    ++p;
  }

  return bottom;
}

static int
MakeBottomRow(const InfoBoxLayout::Layout &layout,
              PixelRect *p, unsigned n, PixelScalar left, PixelScalar bottom)
{
  PixelScalar top = bottom - layout.control_height;
  MakeTopRow(layout, p, n, left, top);
  return top;
}

static int
MakeLeftColumn(const InfoBoxLayout::Layout &layout,
               PixelRect *p, unsigned n, PixelScalar left, PixelScalar top)
{
  PixelRect *const end = p + n;
  const PixelScalar right = left + layout.control_width;
  while (p < end) {
    p->left = left;
    p->right = right;
    p->top = top;
    top += layout.control_height;
    p->bottom = top;

    ++p;
  }

  return right;
}

static int
MakeRightColumn(const InfoBoxLayout::Layout &layout,
                PixelRect *p, unsigned n, PixelScalar right, PixelScalar top)
{
  PixelScalar left = right - layout.control_width;
  MakeLeftColumn(layout, p, n, left, top);
  return left;
}

InfoBoxLayout::Layout
InfoBoxLayout::Calculate(PixelRect rc, Geometry geometry)
{
  Layout layout;

  layout.count = geometry_counts[geometry];
  assert(layout.count <= InfoBoxSettings::Panel::MAX_CONTENTS);

  CalcInfoBoxSizes(layout, rc, geometry);

  layout.ClearVario();

  switch (geometry) {
  case ibTop4Bottom4:
    layout.count = 8;
    rc.top = MakeTopRow(layout, layout.positions, 4, rc.left, rc.top);
    rc.bottom = MakeBottomRow(layout, layout.positions + 4, 4,
                              rc.left, rc.bottom);
    break;

  case ibBottom8Vario:
    layout.vario.left = rc.right - layout.control_width;
    layout.vario.right = rc.right;
    layout.vario.top = rc.bottom - layout.control_height * 2;
    layout.vario.bottom = rc.bottom;

    /* fall through */

  case ibBottom8:
    assert(layout.count == 8);

    rc.bottom = MakeBottomRow(layout, layout.positions + 4, 4,
                              rc.left, rc.bottom);
    rc.bottom = MakeBottomRow(layout, layout.positions, 4,
                              rc.left, rc.bottom);
    break;

  case ibTop8:
    assert(layout.count == 8);

    rc.top = MakeTopRow(layout, layout.positions, 4, rc.left, rc.top);
    rc.top = MakeTopRow(layout, layout.positions + 4, 4, rc.left, rc.top);
    break;

  case ibLeft4Right4:
    assert(layout.count == 8);

    rc.left = MakeLeftColumn(layout, layout.positions, 4, rc.left, rc.top);
    rc.right = MakeRightColumn(layout, layout.positions + 4, 4,
                               rc.right, rc.top);
    break;

  case ibGNav2:
    assert(layout.count == 9);

    layout.vario.left = rc.right - layout.control_width;
    layout.vario.right = rc.right;
    layout.vario.top = 0;
    layout.vario.bottom = layout.vario.top + layout.control_height * 3;

    rc.left = MakeLeftColumn(layout, layout.positions, 6, rc.left, rc.top);
    rc.right = MakeRightColumn(layout, layout.positions + 6, 3, rc.right,
                               rc.top + 3 * layout.control_height);
    break;

  case ibLeft8:
    assert(layout.count == 8);

    rc.left = MakeLeftColumn(layout, layout.positions, 4, rc.left, rc.top);
    rc.left = MakeLeftColumn(layout, layout.positions + 4, 4, rc.left, rc.top);
    break;

  case ibRight8:
    assert(layout.count == 8);

    rc.right = MakeRightColumn(layout, layout.positions + 4, 4,
                               rc.right, rc.top);
    rc.right = MakeRightColumn(layout, layout.positions, 4, rc.right, rc.top);
    break;

  case ibRight12:
    assert(layout.count == 12);

    rc.right = MakeRightColumn(layout, layout.positions + 6, 6,
                               rc.right, rc.top);
    rc.right = MakeRightColumn(layout, layout.positions, 6, rc.right, rc.top);
    break;

  case ibBottom12:
    assert(layout.count == 12);

    rc.bottom = MakeBottomRow(layout, layout.positions + 6, 6,
                              rc.left, rc.bottom);
    rc.bottom = MakeBottomRow(layout, layout.positions, 6,
                              rc.left, rc.bottom);
    break;

  case ibTop12:
    assert(layout.count == 12);

    rc.top = MakeTopRow(layout, layout.positions, 6, rc.left, rc.top);
    rc.top = MakeTopRow(layout, layout.positions + 6, 6, rc.left, rc.top);
    break;

  case ibRight24:
    assert(layout.count == 24);

    rc.right = MakeRightColumn(layout, layout.positions + 16, 8,
                               rc.right, rc.top);
    rc.right = MakeRightColumn(layout, layout.positions + 8, 8,
                               rc.right, rc.top);
    rc.right = MakeRightColumn(layout, layout.positions, 8, rc.right, rc.top);
    break;

  case ibGNav:
    assert(layout.count == 9);

    layout.vario.left = rc.right - layout.control_width;
    layout.vario.right = rc.right;
    layout.vario.top = 0;
    layout.vario.bottom = layout.vario.top + layout.control_height * 3;

    rc.right = MakeRightColumn(layout, layout.positions + 6, 3,
                               rc.right, rc.top + 3 * layout.control_height);
    rc.right = MakeRightColumn(layout, layout.positions, 6, rc.right, rc.top);
    break;

  case ibSquare:
    assert(layout.count == 5);

    rc.right = MakeRightColumn(layout, layout.positions, 5, rc.right, rc.top);
    break;
  };

  layout.remaining = rc;
  return layout;
}

static InfoBoxLayout::Geometry
InfoBoxLayout::ValidateGeometry(InfoBoxLayout::Geometry geometry,
                                UPixelScalar width, UPixelScalar height)
{
  if ((unsigned)geometry >= ARRAY_SIZE(geometry_counts))
    /* out of range */
    geometry = ibTop4Bottom4;

  if (width > height) {
    /* landscape */

    switch (geometry) {
    case ibTop4Bottom4:
      return ibLeft4Right4;

    case ibBottom8:
      return ibRight8;

    case ibBottom8Vario:
      return ibGNav;

    case ibTop8:
      return ibLeft8;

    case ibLeft4Right4:
    case ibLeft8:
    case ibRight8:
    case ibGNav:
    case ibGNav2:
      break;

    case ibSquare:
      return ibRight8;

    case ibRight12:
    case ibRight24:
      break;

    case ibBottom12:
    case ibTop12:
      return ibRight12;
    }
  } else if (width == height) {
    /* square */
    geometry = ibSquare;
  } else {
    /* portrait */

    switch (geometry) {
    case ibTop4Bottom4:
    case ibBottom8:
    case ibBottom8Vario:
    case ibBottom12:
    case ibTop8:
    case ibTop12:
      break;

    case ibLeft4Right4:
      return ibTop4Bottom4;

    case ibLeft8:
      return ibTop8;

    case ibRight8:
    case ibGNav:
    case ibGNav2:
    case ibSquare:
      return ibBottom8;

    case ibRight12:
    case ibRight24:
      return ibBottom12;
    }
  }

  return geometry;
}

InfoBoxLayout::Geometry
InfoBoxLayout::LoadGeometryFromProfile(UPixelScalar width, UPixelScalar height)
{
  unsigned tmp;
  Geometry geometry = Profile::Get(szProfileInfoBoxGeometry, tmp)
    ? (Geometry)tmp
    : ibTop4Bottom4;

  return ValidateGeometry(geometry, width, height);
}

void
InfoBoxLayout::CalcInfoBoxSizes(Layout &layout,
                                PixelRect rc, InfoBoxLayout::Geometry geometry)
{
  switch (geometry) {
  case ibTop4Bottom4:
  case ibBottom8:
  case ibBottom12:
  case ibTop8:
  case ibTop12:
    // calculate control dimensions
    layout.control_width = 2 * (rc.right - rc.left) / layout.count;
    layout.control_height = (rc.bottom - rc.top) / CONTROLHEIGHTRATIO;
    break;

  case ibBottom8Vario:
    // calculate control dimensions
    layout.control_width = 2 * (rc.right - rc.left) / (layout.count + 2);
    layout.control_height = (rc.bottom - rc.top) / CONTROLHEIGHTRATIO;
    break;

  case ibLeft4Right4:
  case ibLeft8:
  case ibRight8:
    // calculate control dimensions
    layout.control_width = (rc.right - rc.left) / CONTROLHEIGHTRATIO * 1.3;
    layout.control_height = 2 * (rc.bottom - rc.top) / layout.count;
    break;

  case ibGNav:
  case ibGNav2:
  case ibRight12:
    // calculate control dimensions
    layout.control_height = (rc.bottom - rc.top) / 6;
    // preserve relative shape
    layout.control_width = layout.control_height * 1.44;
    break;

  case ibSquare:
    // calculate control dimensions
    layout.control_width = (rc.right - rc.left) * 0.2;
    layout.control_height = (rc.bottom - rc.top) / 5;
    break;

  case ibRight24:
    layout.control_height = (rc.bottom - rc.top) / 8;
    layout.control_width = layout.control_height * 1.44;
    break;
  }
}
