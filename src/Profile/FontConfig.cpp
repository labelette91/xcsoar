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

#include "Profile/FontConfig.hpp"
#include "Profile/Profile.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h> /* for strtol() */

static bool
GetFontFromString(const TCHAR *Buffer1, LOGFONT* lplf)
{
  // FontDescription of format:
  // typical font entry
  // 26,0,0,0,700,1,0,0,0,0,0,4,2,<fontname>

  LOGFONT lfTmp;

  assert(lplf != NULL);
  memset((void *)&lfTmp, 0, sizeof(LOGFONT));

  TCHAR *p;
  lfTmp.lfHeight = _tcstol(Buffer1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfWidth = _tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfEscapement = _tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfOrientation = _tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  //FW_THIN   100
  //FW_NORMAL 400
  //FW_MEDIUM 500
  //FW_BOLD   700
  //FW_HEAVY  900

  lfTmp.lfWeight = _tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfItalic = (unsigned char)_tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfUnderline = (unsigned char)_tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfStrikeOut = (unsigned char)_tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfCharSet = (unsigned char)_tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfOutPrecision = (unsigned char)_tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfClipPrecision = (unsigned char)_tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  // DEFAULT_QUALITY			   0
  // RASTER_FONTTYPE			   0x0001
  // DRAFT_QUALITY			     1
  // NONANTIALIASED_QUALITY  3
  // ANTIALIASED_QUALITY     4
  // CLEARTYPE_QUALITY       5
  // CLEARTYPE_COMPAT_QUALITY 6

  lfTmp.lfQuality = (unsigned char)_tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  lfTmp.lfPitchAndFamily = (unsigned char)_tcstol(p + 1, &p, 10);
  if (*p != _T(','))
    return false;

  _tcscpy(lfTmp.lfFaceName, p + 1);

  *lplf = lfTmp;
  return true;
}

bool
Profile::GetFont(const TCHAR *key, LOGFONT* lplf)
{
  TCHAR Buffer[128];

  assert(key != NULL);
  assert(key[0] != '\0');
  assert(lplf != NULL);

  if (Get(key, Buffer, sizeof(Buffer) / sizeof(TCHAR)))
    return GetFontFromString(Buffer, lplf);

  return false;
}

void
Profile::SetFont(const TCHAR *key, LOGFONT &logfont)
{
  StaticString<256> buffer;

  assert(key != NULL);
  assert(key[0] != '\0');

  buffer.Format(_T("%d,%d,0,0,%d,%d,0,0,0,0,0,%d,%d,%s"), logfont.lfHeight,
                logfont.lfWidth, logfont.lfWeight, logfont.lfItalic,
                logfont.lfQuality, logfont.lfPitchAndFamily,
                logfont.lfFaceName);
  Profile::Set(key, buffer);
}
