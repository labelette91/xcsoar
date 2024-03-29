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

#include "InputLookup.hpp"
#include "InputEvents.hpp"
#include "InputQueue.hpp"

// Mapping text names of events to the real thing
struct Text2EventSTRUCT {
  const TCHAR *text;
  pt2Event event;
};

static gcc_constexpr_data Text2EventSTRUCT Text2Event[] = {
#include "InputEvents_Text2Event.cpp"
  { NULL, NULL }
};

// Mapping text names of events to the real thing
static const TCHAR *const Text2GCE[] = {
#include "InputEvents_Text2GCE.cpp"
  NULL
};

// Mapping text names of events to the real thing
static const TCHAR *const Text2NE[] = {
#include "InputEvents_Text2NE.cpp"
  NULL
};

pt2Event
InputEvents::findEvent(const TCHAR *data)
{
  for (unsigned i = 0; Text2Event[i].text != NULL; ++i)
    if (_tcscmp(data, Text2Event[i].text) == 0)
      return Text2Event[i].event;

  return NULL;
}

int
InputEvents::findGCE(const TCHAR *data)
{
  int i;
  for (i = 0; i < GCE_COUNT; i++) {
    if (_tcscmp(data, Text2GCE[i]) == 0)
      return i;
  }

  return -1;
}

int
InputEvents::findNE(const TCHAR *data)
{
  int i;
  for (i = 0; i < NE_COUNT; i++) {
    if (_tcscmp(data, Text2NE[i]) == 0)
      return i;
  }

  return -1;
}
