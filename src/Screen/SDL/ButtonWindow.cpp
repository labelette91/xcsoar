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

#include "Screen/ButtonWindow.hpp"
#include "Screen/ContainerWindow.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"

void
ButtonWindow::set(ContainerWindow &parent, const TCHAR *text, unsigned id,
                  const PixelRect &rc,
                  const ButtonWindowStyle style)
{
  reset();

  PaintWindow::set(parent, rc, style);

  this->text = text;
  this->id = id;
}

void
ButtonWindow::set_down(bool _down)
{
  if (_down == down)
    return;

  down = _down;
  invalidate();
}

bool
ButtonWindow::on_key_check(unsigned key_code) const
{
  switch (key_code) {
  case VK_RETURN:
    return true;

  default:
    return PaintWindow::on_key_check(key_code);
  }
}

bool
ButtonWindow::on_key_down(unsigned key_code)
{
  switch (key_code) {
  case VK_RETURN:
  case VK_SPACE:
    set_down(false);

    if (!on_clicked() && id != 0 && parent != NULL)
      parent->on_command(id, 0);
    return true;

  default:
    return PaintWindow::on_key_down(key_code);
  }
}

bool
ButtonWindow::on_mouse_move(PixelScalar x, PixelScalar y, unsigned keys)
{
  if (dragging) {
    set_down(x >= 0 && y >= 0 &&
             (unsigned)x < get_width() && (unsigned)y < get_height());
    return true;
  } else
    return PaintWindow::on_mouse_move(x, y, keys);
}

bool
ButtonWindow::on_mouse_down(PixelScalar x, PixelScalar y)
{
  if (is_tab_stop())
    set_focus();

  set_down(true);
  set_capture();
  dragging = true;
  return true;
}

bool
ButtonWindow::on_mouse_up(PixelScalar x, PixelScalar y)
{
  if (!dragging)
    return true;

  dragging = false;
  release_capture();

  if (!down)
    return true;

  set_down(false);

  if (!on_clicked() && id != 0 && parent != NULL)
    parent->on_command(id, 0);

  return true;
}

void
ButtonWindow::on_setfocus()
{
  PaintWindow::on_setfocus();
  invalidate();
}

void
ButtonWindow::on_killfocus()
{
  PaintWindow::on_killfocus();
  invalidate();
}

bool
ButtonWindow::on_cancel_mode()
{
  dragging = false;
  set_down(false);

  return PaintWindow::on_cancel_mode();
}

void
ButtonWindow::on_paint(Canvas &canvas)
{
  if (has_focus()) {
    Pen pen(Layout::Scale(1), COLOR_BLACK);
    canvas.Select(pen);
    canvas.SelectHollowBrush();
    canvas.Rectangle(-1, -1, canvas.get_width(), canvas.get_height());
  }

  PixelRect rc = {
    2, 2, PixelScalar(canvas.get_width() - 4),
    PixelScalar(canvas.get_height() - 4),
  };

  if (down) {
    rc.left += Layout::FastScale(1);
    rc.top += Layout::FastScale(1);
  }

  canvas.DrawButton(get_client_rect(), down);

  canvas.SetTextColor(is_enabled() ? COLOR_BLACK : COLOR_GRAY);
  canvas.SetBackgroundTransparent();
  canvas.formatted_text(&rc, text.c_str(), get_text_style());
}

bool
ButtonWindow::on_clicked()
{
  return false;
}
