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

#include "Form/SymbolButton.hpp"
#include "Form/Control.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "resource.h"

void
WndSymbolButton::on_paint(Canvas &canvas)
{
  PixelRect rc = {
    PixelScalar(0), PixelScalar(0), PixelScalar(canvas.get_width()),
    PixelScalar(canvas.get_height())
  };

  bool pressed = is_down();

  renderer.DrawButton(canvas, rc, has_focus(), pressed);
  // If button has text on it
  tstring caption = get_text();
  if (caption.empty())
    return;

  rc = renderer.GetDrawingRect(rc, pressed);

  canvas.SelectNullPen();
  if (!is_enabled())
    canvas.Select(look.button.disabled.brush);
  else if (has_focus())
    canvas.Select(look.button.focused.foreground_brush);
  else
    canvas.Select(look.button.standard.foreground_brush);

  const char ch = (char)caption[0];

  // Draw arrow symbols instead of < and >
  if (ch == '<' || ch == '>') {
    int size = min(rc.right - rc.left, rc.bottom - rc.top) / 5;

    RasterPoint Arrow[3];
    Arrow[0].x = (rc.left + rc.right) / 2 + (ch == '<' ? size : -size);
    Arrow[0].y = (rc.top + rc.bottom) / 2 + size;
    Arrow[1].x = (rc.left + rc.right) / 2 + (ch == '<' ? -size : size);
    Arrow[1].y = (rc.top + rc.bottom) / 2;
    Arrow[2].x = (rc.left + rc.right) / 2 + (ch == '<' ? size : -size);
    Arrow[2].y = (rc.top + rc.bottom) / 2 - size;

    canvas.DrawTriangleFan(Arrow, 3);
  }

  // Draw arrow symbols instead of v and ^
  else if (ch == '^' || ch == 'v') {
    int size = min(rc.right - rc.left, rc.bottom - rc.top) / 5;

    RasterPoint Arrow[3];
    Arrow[0].x = (rc.left + rc.right) / 2 +
                 size;
    Arrow[0].y = (rc.top + rc.bottom) / 2 +
                 (ch == '^' ? size : -size);
    Arrow[1].x = (rc.left + rc.right) / 2;
    Arrow[1].y = (rc.top + rc.bottom) / 2 +
                 (ch == '^' ? -size : size);
    Arrow[2].x = (rc.left + rc.right) / 2 - size;
    Arrow[2].y = (rc.top + rc.bottom) / 2 +
                 (ch == '^' ? size : -size);

    canvas.DrawTriangleFan(Arrow, 3);
  }

  // Draw symbols instead of + and -
  else if (ch == '+' || ch == '-') {
    int size = min(rc.right - rc.left, rc.bottom - rc.top) / 5;

    canvas.Rectangle((rc.left + rc.right) / 2 - size,
                     (rc.top + rc.bottom) / 2 - size / 3,
                     (rc.left + rc.right) / 2 + size,
                     (rc.top + rc.bottom) / 2 + size / 3);

    if (ch == '+')
      canvas.Rectangle((rc.left + rc.right) / 2 - size / 3,
                       (rc.top + rc.bottom) / 2 - size,
                       (rc.left + rc.right) / 2 + size / 3,
                       (rc.top + rc.bottom) / 2 + size);
  }

  // Draw Fly bitmap
  else if (caption == _T("Fly")) {
    Bitmap launcher1_bitmap(IDB_LAUNCHER1);
    canvas.ClearWhite();
    if (is_down())
      canvas.invert_stretch_transparent(launcher1_bitmap, COLOR_YELLOW);
    else
      canvas.stretch_transparent(launcher1_bitmap, COLOR_BLUE);
  }

  // Draw Simulator bitmap
  else if (caption == _T("Simulator")) {
    Bitmap launcher2_bitmap(IDB_LAUNCHER2);
    canvas.ClearWhite();
    if (is_down())
      canvas.invert_stretch_transparent(launcher2_bitmap, COLOR_YELLOW);
    else
      canvas.stretch_transparent(launcher2_bitmap, COLOR_BLUE);
  }

  else if (caption == _T("Green")) {
    InflateRect(&rc, -3, -3);
    canvas.DrawFilledRectangle(rc, Color(0x74, 0xFF, 0));
  } else if (caption == _T("Blue")) {
    InflateRect(&rc, -3, -3);
    canvas.DrawFilledRectangle(rc, Color(0, 0x90, 0xFF));
  } else if (caption == _T("Magenta")) {
    InflateRect(&rc, -3, -3);
    canvas.DrawFilledRectangle(rc, Color(0xFF, 0, 0xCB));
  } else if (caption == _T("Yellow")) {
    InflateRect(&rc, -3, -3);
    canvas.DrawFilledRectangle(rc, Color(0xFF, 0xE8, 0));
  }
}
