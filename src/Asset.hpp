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
#ifndef ASSET_H
#define ASSET_H

#include "Compiler.h"
#ifdef ANDROID
#include "Android/Main.hpp"
#include "Android/NativeView.hpp"
#include "unix/tchar.h"
#endif

#include <tchar.h>

// asset/registration data
extern TCHAR asset_number[];

void ReadAssetNumber(void);

// model info

enum ModelType {
  MODELTYPE_PNA_PNA,
  MODELTYPE_PNA_HP31X,
  MODELTYPE_PNA_MEDION_P5,
  MODELTYPE_PNA_MIO,
  MODELTYPE_PNA_NOKIA_500,
  MODELTYPE_PNA_PN6000,
};

#if defined(_WIN32_WCE) && !defined(GNAV)
#define HAVE_MODEL_TYPE

extern ModelType global_model_type;

#else

#define global_model_type MODELTYPE_PNA_PNA

#endif

gcc_constexpr_function
static inline bool
HasModelType()
{
#ifdef HAVE_MODEL_TYPE
  return true;
#else
  return false;
#endif
}

/**
 * Returns whether this is a debug build.
 */
gcc_constexpr_function
static inline bool
IsDebug()
{
#ifdef NDEBUG
  return false;
#else
  return true;
#endif
}

/**
 * Returns whether the application is running on an embedded platform.
 * @return True if host hardware is an embedded platform, False otherwise
 */
gcc_constexpr_function
static inline bool
IsEmbedded()
{
#if defined(_WIN32_WCE) || defined(ANDROID)
  return true;
#else
  return false;
#endif
}

/**
 * Returns whether the application is running on Pocket PC / Windows
 * CE / Windows Mobile.
 */
gcc_constexpr_function
static inline bool
IsWindowsCE()
{
#ifdef _WIN32_WCE
  return true;
#else
  return false;
#endif
}

/**
 * Returns whether the application is running on an old version of
 * Windows CE (pre 5.0).  Starting with version 5.0, several bug
 * workarounds are disabled at compile time.
 */
gcc_constexpr_function
static inline bool
IsOldWindowsCE()
{
#if defined(_WIN32_WCE) && _WIN32_WCE < 0x0500
  return true;
#else
  return false;
#endif
}

/**
 * Is XCSoar running on ancient and slow hardware?  If yes, then some
 * expensive UI features are disabled.
 */
gcc_constexpr_function
static inline bool
IsAncientHardware()
{
#if defined(_WIN32_WCE) && _WIN32_WCE < 0x0400
  /* Windows CE 3.0 (PPC2000 & PPC2002) */
  return true;
#else
  /* we assume that all other platforms are fast enough */
  return false;
#endif
}

/**
 * Returns whether the application is running on a HP31x
 * @return True if host hardware is a HP31x, False otherwise
 */
static inline bool
IsHP31X()
{
  return global_model_type == MODELTYPE_PNA_HP31X;
}

/**
 * Returns whether the application is running on a Medion P5
 * @return True if host hardware is a Medion P5, False otherwise
 */
static inline bool
IsMedionP5()
{
  return global_model_type == MODELTYPE_PNA_MEDION_P5;
}

/**
 * Returns whether the application is running on an Altair
 * @return True if host hardware is an Altair, False otherwise
 */
gcc_constexpr_function
static inline bool
IsAltair()
{
#if defined(GNAV)
  return true;
#else
  return false;
#endif
}

/**
 * Returns whether the application is running on Android
 */
gcc_constexpr_function
static inline bool
IsAndroid()
{
#if defined(ANDROID)
  return true;
#else
  return false;
#endif
}

/**
 * Returns whether the application is running on Galaxy Tab with Android 2.2
 */
static inline bool
IsGalaxyTab22()
{
#if defined(ANDROID)
  assert(native_view);
  return native_view->GetAPILevel() == 8 &&
         (_tcscmp(native_view->GetProduct(), _T("GT-P1000")) == 0 ||
          _tcscmp(native_view->GetProduct(), _T("GT-P1010")) == 0);
#else
  return false;
#endif
}

/**
 * Does this device have little main memory?  On those, some expensive
 * features are disabled.
 */
gcc_constexpr_function
static inline bool
HasLittleMemory()
{
  return IsAncientHardware() || IsAltair();
}

/**
 * Returns whether the application is compiled with IOIOLib
 */
gcc_constexpr_function
static inline bool
HasIOIOLib()
{
#ifdef IOIOLIB
  return true;
#else
  return false;
#endif
}

/**
 * Does this device have a pointer device? (mouse or touch screen)
 * @return True if a touch screen or mouse is assumed for the hardware
 * that XCSoar is running on, False if the hardware has only buttons
 */
gcc_constexpr_function
static inline bool
HasPointer()
{
  return !IsAltair();
}

/**
 * Does this device have a keyboard device?
 * @return True if a keyboard is assumed for the hardware
 * that XCSoar is running on, False if the hardware has no keyboard
 */
gcc_constexpr_function
static inline bool
HasKeyboard()
{
  return !IsEmbedded();
}

/**
 * Does this device have a display with colors?
 *
 * XXX not yet implemented!
 */
gcc_constexpr_function
static inline bool
HasColors()
{
  return true;
}

#ifndef _WIN32_WCE
extern int SCREENWIDTH;
extern int SCREENHEIGHT;
#endif

void InitAsset();

#endif
