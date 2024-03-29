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

#ifndef TOPOGRAPHY_STORE_HPP
#define TOPOGRAPHY_STORE_HPP

#include "Util/NonCopyable.hpp"
#include "Util/StaticArray.hpp"

#include <tchar.h>

class WindowProjection;
class TopographyFile;
class NLineReader;
class OperationEnvironment;
struct zzip_dir;

/**
 * Class used to manage and render vector topography layers
 */
class TopographyStore : private NonCopyable {
public:
  enum {
    /** maximum number of topography layers */
    MAXTOPOGRAPHY = 20,
  };

private:
  StaticArray<TopographyFile *, MAXTOPOGRAPHY> files;

public:
  ~TopographyStore();

  unsigned size() const {
    return files.size();
  }

  const TopographyFile &operator [](unsigned i) const {
    return *files[i];
  }

  /**
   * @param max_update the maximum number of files updated in this
   * call
   * @return the number of files which were updated
   */
  unsigned ScanVisibility(const WindowProjection &m_projection,
                          unsigned max_update=1024);

  void Load(OperationEnvironment &operation, NLineReader &reader,
            const TCHAR *directory, struct zzip_dir *zdir = NULL);
  void Reset();
};

#endif
