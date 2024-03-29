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

#ifndef XCSOAR_CAI302_INTERNAL_HPP
#define XCSOAR_CAI302_INTERNAL_HPP

#include "Device/Driver.hpp"

/** 
 * Device driver for Cambridge Aero Instruments 302 
 */
class CAI302Device : public AbstractDevice {
private:
  Port &port;

public:
  CAI302Device(Port &_port)
    :port(_port) {}

public:
  virtual bool Open(OperationEnvironment &env);
  virtual bool ParseNMEA(const char *line, struct NMEAInfo &info);
  virtual bool PutMacCready(fixed MacCready);
  virtual bool PutBugs(fixed bugs);
  virtual bool PutBallast(fixed ballast);
  virtual bool Declare(const Declaration &declaration, const Waypoint *home,
                       OperationEnvironment &env);

  virtual bool ReadFlightList(RecordedFlightList &flight_list,
                              OperationEnvironment &env);
  virtual bool DownloadFlight(const RecordedFlightInfo &flight,
                              const TCHAR *path,
                              OperationEnvironment &env);

public:
  /**
   * Restart the CAI302 by sending the command "SIF 0 0".
   */
  bool Reboot(OperationEnvironment &env);

  /**
   * Power off the CAI302 by sending the command "DIE".
   */
  bool PowerOff(OperationEnvironment &env);

  /**
   * Start logging unconditionally.
   */
  bool StartLogging(OperationEnvironment &env);

  /**
   * Stop logging unconditionally.
   */
  bool StopLogging(OperationEnvironment &env);

  /**
   * Set audio volume 0 is loudest, 170 is silent.
   */
  bool SetVolume(unsigned volume, OperationEnvironment &env);

  /**
   * Erase all waypoints.
   */
  bool ClearPoints(OperationEnvironment &env);

  /**
   * Erase the pilot name.
   */
  bool ClearPilot(OperationEnvironment &env);

  /**
   * Erase all log memory.
   */
  bool ClearLog(OperationEnvironment &env);
};

#endif
