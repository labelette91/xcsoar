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

#ifndef XCSOAR_SCREEN_BUSY_HPP
#define XCSOAR_SCREEN_BUSY_HPP

#include "Asset.hpp"

#ifdef WIN32
#include <windows.h>
#endif

/**
 * Show the hour glass cursor while this object exists.
 */
class ScopeBusyIndicator {
#ifdef WIN32
  HCURSOR old_cursor;

public:
  ScopeBusyIndicator()
    :old_cursor(::SetCursor(::LoadCursor(NULL, IDC_WAIT))) {
    if (IsAltair() && IsEmbedded())
      SetCursorPos(160,120);
  }

  ~ScopeBusyIndicator() {
    ::SetCursor(old_cursor);
    if (IsAltair() && IsEmbedded())
      SetCursorPos(640, 480);
  }
#endif /* WIN32 */
};

#endif
