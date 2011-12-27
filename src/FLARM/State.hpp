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

#ifndef XCSOAR_FLARM_STATE_HPP
#define XCSOAR_FLARM_STATE_HPP

#include "FLARM/Traffic.hpp"
#include "NMEA/Validity.hpp"
#include "Util/TrivialArray.hpp"
#include "Util/TypeTraits.hpp"

/**
 * Received FLARM data, cached
 */
struct FlarmState
{
  enum {
    FLARM_MAX_TRAFFIC = 25,
  };

  enum class GPSStatus: uint8_t {
    NONE = 0,
    GPS_2D = 1,
    GPS_3D = 2,
  };

  /** Number of received FLARM devices */
  unsigned short rx;
  /** Transmit status */
  bool tx;

  /**
   * Is there new FLARM traffic present?
   * @see traffic
   */
  bool new_traffic;

  /** GPS status */
  GPSStatus gps;

  /** Alarm level of FLARM (0-3) */
  FlarmTraffic::AlarmType alarm_level;

  /** Is FLARM information available? */
  Validity available;
  /** Flarm traffic information */
  TrivialArray<FlarmTraffic, FLARM_MAX_TRAFFIC> traffic;

public:
  void Clear();

  bool IsDetected() const {
    return available || !traffic.empty();
  }

  /**
   * Adds data from the specified object, unless already present in
   * this one.
   */
  void Complement(const FlarmState &add) {
    if (!available && add.available)
      *this = add;
  }

  unsigned GetActiveTrafficCount() const {
    return traffic.size();
  }

  /**
   * Looks up an item in the traffic list.
   *
   * @param id FLARM id
   * @return the FLARM_TRAFFIC pointer, NULL if not found
   */
  FlarmTraffic *FindTraffic(FlarmId id) {
    for (auto it = traffic.begin(), end = traffic.end(); it != end; ++it)
      if (it->id == id)
        return it;

    return NULL;
  }

  /**
   * Looks up an item in the traffic list.
   *
   * @param id FLARM id
   * @return the FLARM_TRAFFIC pointer, NULL if not found
   */
  const FlarmTraffic *FindTraffic(FlarmId id) const {
    for (auto it = traffic.begin(), end = traffic.end(); it != end; ++it)
      if (it->id == id)
        return it;

    return NULL;
  }

  /**
   * Looks up an item in the traffic list.
   *
   * @param name the name or call sign
   * @return the FLARM_TRAFFIC pointer, NULL if not found
   */
  FlarmTraffic *FindTraffic(const TCHAR *name) {
    for (auto it = traffic.begin(), end = traffic.end(); it != end; ++it)
      if (it->name.equals(name))
        return it;

    return NULL;
  }

  /**
   * Looks up an item in the traffic list.
   *
   * @param name the name or call sign
   * @return the FLARM_TRAFFIC pointer, NULL if not found
   */
  const FlarmTraffic *FindTraffic(const TCHAR *name) const {
    for (auto it = traffic.begin(), end = traffic.end(); it != end; ++it)
      if (it->name.equals(name))
        return it;

    return NULL;
  }

  /**
   * Allocates a new FLARM_TRAFFIC object from the array.
   *
   * @return the FLARM_TRAFFIC pointer, NULL if the array is full
   */
  FlarmTraffic *AllocateTraffic() {
    return traffic.full()
      ? NULL
      : &traffic.append();
  }

  /**
   * Search for the previous traffic in the ordered list.
   */
  const FlarmTraffic *PreviousTraffic(const FlarmTraffic *t) const {
    return t > traffic.begin()
      ? t - 1
      : NULL;
  }

  /**
   * Search for the next traffic in the ordered list.
   */
  const FlarmTraffic *NextTraffic(const FlarmTraffic *t) const {
    return t + 1 < traffic.end()
      ? t + 1
      : NULL;
  }

  /**
   * Search for the first traffic in the ordered list.
   */
  const FlarmTraffic *FirstTraffic() const {
    return traffic.empty() ? NULL : traffic.begin();
  }

  /**
   * Search for the last traffic in the ordered list.
   */
  const FlarmTraffic *LastTraffic() const {
    return traffic.empty() ? NULL : traffic.end() - 1;
  }

  /**
   * Finds the most critical alert.  Returns NULL if there is no
   * alert.
   */
  const FlarmTraffic *FindMaximumAlert() const;

  unsigned TrafficIndex(const FlarmTraffic *t) const {
    return t - traffic.begin();
  }

  void Refresh(fixed Time) {
    available.Expire(Time, fixed(10));
    if (!available)
      traffic.clear();

    for (unsigned i = traffic.size(); i-- > 0;)
      if (!traffic[i].Refresh(Time))
        traffic.quick_remove(i);

    new_traffic = false;
  }
};

static_assert(is_trivial<FlarmState>::value, "type is not trivial");

#endif
