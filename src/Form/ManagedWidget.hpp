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

#ifndef XCSOAR_MANAGED_WIDGET_HPP
#define XCSOAR_MANAGED_WIDGET_HPP

#include "Screen/Point.hpp"
#include "Util/DebugFlag.hpp"

class ContainerWindow;
class Widget;

/**
 * A wrapper for a #Widget pointer that keeps track of the Widget's
 * state.  It will enforce some of the #Widget API rules.  It will
 * take care for hiding, unpreparing and deleting the given #Widget
 * instance, and will not call Widget::Prepare() and Widget::Show()
 * twice.
 */
class ManagedWidget {
  ContainerWindow &parent;
  PixelRect position;

  Widget *widget;

  bool prepared, visible;

  DebugFlag have_position;

public:
  ManagedWidget(ContainerWindow &_parent)
    :parent(_parent), widget(NULL) {}

  ~ManagedWidget() {
    Clear();
  }

  bool IsDefined() const {
    return widget != NULL;
  }

  bool IsPrepared() const {
    return IsDefined() && prepared;
  }

  bool IsVisible() const {
    return IsPrepared() && visible;
  }

  void Clear();
  void Set(Widget *widget);

  Widget *Get() {
    return widget;
  }

  void Move(const PixelRect &position);

  void Show();
  void Hide();

  void SetVisible(bool _visible);
};

#endif
