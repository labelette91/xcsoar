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

#ifndef WINDSTORE_H
#define WINDSTORE_H

#include "Wind/WindMeasurementList.hpp"

struct NMEAInfo;
struct MoreData;
struct DerivedInfo;

/**
 * WindStore receives single windmeasurements and stores these. It uses
 * single measurements to provide a mean value, differentiated for altitude.
 */
class WindStore
{
  fixed _lastAltitude;
  WindMeasurementList windlist;

  /**
   * The time stamp (NMEAInfo::clock) of the last wind update.  It is
   * used to update DerivedInfo::estimated_wind_available.
   */
  fixed update_clock;

  bool updated;

public:
  /**
   * Called with new measurements. The quality is a measure for how good the
   * measurement is. Higher quality measurements are more important in the
   * end result and stay in the store longer.
   */
  void SlotMeasurement(const MoreData &info,
      Vector windvector, int quality);

  /**
   * Called if the altitude changes.
   * Determines where measurements are stored and may result in a NewWind
   * signal.
   */
  void SlotAltitude(const MoreData &info, DerivedInfo &derived);

  /**
   * Send if a new wind vector has been established. This may happen as
   * new measurements flow in, but also if the altitude changes.
   */
  void NewWind(const NMEAInfo &info, DerivedInfo &derived, Vector& wind) const;

  gcc_pure
  const Vector GetWind(fixed Time, fixed h, bool &found) const;

  /** Clear as if never flown */
  void reset();

private:
  /**
   * Recalculates the wind from the stored measurements.
   * May result in a NewWind signal.
   */
  void recalculateWind(const MoreData &info, DerivedInfo &derived) const;
};

#endif
