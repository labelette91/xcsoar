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

#ifndef WINDMEASUREMENTLIST_H
#define WINDMEASUREMENTLIST_H

#include "Util/StaticArray.hpp"
#include "Vector.hpp"

/**
 * Structure to hold a single wind measurement
 */
struct WindMeasurement
{
  Vector vector;                /**< Wind speed vector */
  int quality;                  /**< Quality of fit */
  long time;                    /**< Time of fix */
  fixed altitude;               /**< Altitude of fix */

  gcc_pure
  int Score(long _time) const {
    // Calculate the score of this item. The item with the highest
    // score is the least important one.  We may need to adjust the
    // proportion of the quality and the elapsed time. Currently, one
    // quality-point (scale: 1 to 5) is equal to 10 minutes.
    return 600 * (6 - quality) + (_time - time);
  }
};

/**
 * The WindMeasurementList is a list that can contain and
 * process windmeasurements.
 * @author Andr� Somers
 */
class WindMeasurementList
{
protected:
  StaticArray<WindMeasurement, 200> measurements;

public:
  /**
   * Returns the weighted mean windvector over the stored values, or 0
   * if no valid vector could be calculated (for instance: too little or
   * too low quality data).
   */
  gcc_pure
  const Vector getWind(fixed Time, fixed alt, bool &found) const;

  /** Adds the windvector vector with quality quality to the list. */
  void addMeasurement(fixed Time, Vector vector, fixed alt, int quality);

  void Reset();

protected:
  /**
   * getLeastImportantItem is called to identify the item that should be
   * removed if the list is too full. Reimplemented from LimitedList.
   */
  gcc_pure
  unsigned int getLeastImportantItem(fixed Time);
};

#endif
