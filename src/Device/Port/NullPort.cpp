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

#include "NullPort.hpp"

#include <stdio.h>

NullPort::NullPort()
  :Port(*(Port::Handler *)this)
{
}

NullPort::NullPort(Port::Handler &_handler)
  :Port(_handler)
{
}

void
NullPort::Flush(void)
{
}

size_t
NullPort::Write(const void *data, size_t length)
{
  return length;
}

bool
NullPort::StopRxThread()
{
  return true;
}

bool
NullPort::StartRxThread(void)
{
  return true;
}

bool
NullPort::SetRxTimeout(unsigned Timeout)
{
  return true;
}

unsigned
NullPort::GetBaudrate() const
{
  return 0;
}

unsigned
NullPort::SetBaudrate(unsigned BaudRate)
{
  return BaudRate;
}

int
NullPort::Read(void *Buffer, size_t Size)
{
  return -1;
}

void
NullPort::DataReceived(const void *data, size_t length)
{
}
