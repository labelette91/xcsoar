/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

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

#ifndef XCSOAR_PROFILE_HPP
#define XCSOAR_PROFILE_HPP

#include "ProfileKeys.hpp"
#include "Engine/Math/fixed.hpp"
#include <tchar.h>

namespace Profile
{
  /**
   * Reads the profile settings from the registry and adjusts the
   * application settings
   */
  void Use();
  /**
   * Loads the profile files into the registry
   */
  void Load();
  /**
   * Loads the given profile file into the registry
   */
  void LoadFile(const TCHAR *szFile);
  /**
   * Saves the registry into the profile files
   */
  void Save();
  /**
   * Saves the registry into the given profile file
   */
  void SaveFile(const TCHAR *szFile);
  /**
   * Saves the sound settings to the registry
   */
  void SaveSoundSettings();
  /**
   * Saves the wind settings to the registry
   */
  void SaveWindToRegistry();
  /**
   * Loads the wind settings from the registry
   */
  void LoadWindFromRegistry();
  /**
   * Saves the airspace mode setting to the registry
   * @param i Airspace class index
   */
  void SetRegistryAirspaceMode(int i);
  /**
   * Sets the files to load when calling Load()
   * @param override NULL or file to load when calling Load()
   */
  void SetFiles(const TCHAR* override);

  bool Get(const TCHAR *szRegValue, int &pPos);
  bool Get(const TCHAR *szRegValue, short &pPos);
  bool Get(const TCHAR *szRegValue, bool &pPos);
  bool Get(const TCHAR *szRegValue, unsigned &pPos);
  bool Get(const TCHAR *szRegValue, double &pPos);

  bool Set(const TCHAR *szRegValue, int pPos);
  bool Set(const TCHAR *szRegValue, short pPos);
  bool Set(const TCHAR *szRegValue, bool pPos);
  bool Set(const TCHAR *szRegValue, unsigned pPos);
  bool Set(const TCHAR *szRegValue, double pPos);

  /**
   * Reads the airspace mode setting from the registry
   * @param i Airspace class index
   * @return The mode
   */
  int GetRegistryAirspaceMode(int i);

  int GetScaleList(fixed *List, size_t Size);
};

#endif
