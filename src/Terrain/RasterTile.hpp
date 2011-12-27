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
#ifndef XCSOAR_RASTERTILE_HPP
#define XCSOAR_RASTERTILE_HPP

#include "Terrain/RasterBuffer.hpp"
#include "Geo/GeoBounds.hpp"
#include "Util/NonCopyable.hpp"
#include "Util/StaticArray.hpp"
#include "Util/Serial.hpp"

#include <assert.h>
#include <tchar.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#define RASTER_SLOPE_FACT 12

struct RasterLocation;
struct GridLocation;
class OperationEnvironment;

class RasterTile : private NonCopyable {
  struct MetaData {
    unsigned int xstart, ystart, xend, yend;
  };

public:
  unsigned int xstart, ystart, xend, yend;
  unsigned int width, height;

  /**
   * The distance of this tile to the center of the screen.  This
   * attribute is used to determine which tiles should be loaded.
   */
  unsigned distance;

  bool request;

  RasterBuffer buffer;

public:
  RasterTile()
    :xstart(0), ystart(0), xend(0), yend(0),
     width(0), height(0) {}

  void set(unsigned _xstart, unsigned _ystart,
           unsigned _xend, unsigned _yend) {
    xstart = _xstart;
    ystart = _ystart;
    xend = _xend;
    yend = _yend;
    width = xend - xstart;
    height = yend - ystart;
  }

  /**
   * Permanently disable this tile after a failure.
   */
  void Clear() {
    width = height = 0;
    request = false;
  }

  bool defined() const {
    return width > 0 && height > 0;
  }

  int get_distance() const {
    return distance;
  }

  bool is_requested() const {
    return request;
  }

  void set_request() {
    request = true;
  }

  void clear_request() {
    request = false;
  }

  bool SaveCache(FILE *file) const;
  bool LoadCache(FILE *file);

  bool CheckTileVisibility(int view_x, int view_y, unsigned view_radius);

  void Disable() {
    buffer.reset();
  }

  void Enable();
  bool IsEnabled() const {
    return buffer.defined();
  }
  bool IsDisabled() const {
    return !buffer.defined();
  }

  /**
   * Determine the non-interpolated height at the specified pixel
   * location.
   *
   * @param x the pixel column within the tile; may be out of range
   * @param y the pixel row within the tile; may be out of range
   */
  gcc_pure
  short GetHeight(unsigned x, unsigned y) const;

  /**
   * Determine the interpolated height at the specified sub-pixel
   * location.
   *
   * @param x the pixel column within the tile; may be out of range
   * @param y the pixel row within the tile; may be out of range
   * @param ix the sub-pixel column for interpolation (0..255)
   * @param iy the sub-pixel row for interpolation (0..255)
   */
  gcc_pure
  short GetInterpolatedHeight(unsigned x, unsigned y,
                              unsigned ix, unsigned iy) const;

  inline short* GetImageBuffer() {
    return buffer.get_data();
  }

  bool VisibilityChanged(int view_x, int view_y, unsigned view_radius);

  void ScanLine(unsigned ax, unsigned ay, unsigned bx, unsigned by,
                short *dest, unsigned size, bool interpolate) const {
    buffer.ScanLine(ax - (xstart << 8), ay - (ystart << 8),
                    bx - (xstart << 8), by - (ystart << 8),
                    dest, size, interpolate);
  }
};

class RasterTileCache : private NonCopyable {
  static const unsigned MAX_RTC_TILES = 4096;

  /**
   * The maximum number of tiles which are loaded at a time.  This
   * must be limited because the amount of memory is finite.
   */
#if defined(ANDROID)
  static const unsigned MAX_ACTIVE_TILES = 128;
#elif !defined(_WIN32_WCE)
  // desktop: use a lot of memory
  static const unsigned MAX_ACTIVE_TILES = 512;
#elif !defined(_WIN32_WCE) || (_WIN32_WCE >= 0x0400 && !defined(GNAV))
  // embedded: use less memory
  static const unsigned MAX_ACTIVE_TILES = 32;
#else
  // old Windows CE and Altair: use only little memory
  static const unsigned MAX_ACTIVE_TILES = 16;
#endif

  /**
   * The width and height of the terrain bitmap is shifted by this
   * number of bits to determine the overview size.
   */
  static const unsigned OVERVIEW_BITS = 4;

  /**
   * Target number of steps in intersection searches; total distance
   * is shifted by this number of bits
   */
  static const unsigned INTERSECT_BITS = 7;

public:
  /**
   * The fixed-point fractional part of sub-pixel coordinates.
   *
   * Do not edit!  There are still some hard-coded code sections left,
   * e.g. CombinedDivAndMod().
   */
  static const unsigned SUBPIXEL_BITS = 8;

protected:
  friend struct RTDistanceSort;

  struct MarkerSegmentInfo {
    static const uint16_t NO_TILE = (uint16_t)-1;

    /**
     * The position of this marker segment within the file.
     */
    uint32_t file_offset;

    /**
     * The associated tile number.  -1 if this segment does not belong
     * to a tile.
     */
    uint16_t tile;

    /**
     * The number of follow-up segments.
     */
    uint16_t count;

    MarkerSegmentInfo() {}
    MarkerSegmentInfo(uint32_t _file_offset, int _tile=NO_TILE)
      :file_offset(_file_offset), tile(_tile), count(0) {}

    bool IsTileSegment() const {
      return tile != NO_TILE;
    }
  };

  struct CacheHeader {
    enum {
#ifdef FIXED_MATH
      VERSION = 0x8,
#else
      VERSION = 0x9,
#endif
    };

    unsigned version;
    unsigned width, height;
    unsigned short tile_width, tile_height;
    unsigned tile_columns, tile_rows;
    unsigned num_marker_segments;
    GeoBounds bounds;
  };

  bool initialised;

  /** is the "bounds" attribute valid? */
  bool bounds_initialised;

  bool dirty;

  /**
   * This serial gets updated each time the tiles get loaded or
   * discarded.
   */
  Serial serial;

  AllocatedGrid<RasterTile> tiles;
  unsigned short tile_width, tile_height;

  RasterBuffer Overview;
  bool scan_overview;
  unsigned int width, height;
  unsigned int overview_width_fine, overview_height_fine;

  GeoBounds bounds;

  StaticArray<MarkerSegmentInfo, 8192> segments;

  /**
   * The number of remaining segments after the current one.
   */
  mutable unsigned remaining_segments;

  /**
   * An array that is used to sort the requested tiles by distance.
   * This is only used by PollTiles() internally, but is stored in the
   * class because it would be too large for the stack.
   */
  StaticArray<uint16_t, MAX_RTC_TILES> RequestTiles;

  /**
   * Progress callbacks for loading the file during startup.
   */
  OperationEnvironment *operation;

public:
  RasterTileCache():operation(NULL) {
    Reset();
  }

protected:
  void ScanTileLine(GridLocation start, GridLocation end,
                    short *buffer, unsigned size, bool interpolate) const;

public:
  /**
   * Determine the non-interpolated height at the specified pixel
   * location.
   *
   * @param x the pixel column within the map; may be out of range
   * @param y the pixel row within the map; may be out of range
   */
  gcc_pure
  short GetHeight(unsigned x, unsigned y) const;

  /**
   * Determine the interpolated height at the specified sub-pixel
   * location.
   *
   * @param lx the sub-pixel column within the map; may be out of range
   * @param ly the sub-pixel row within the map; may be out of range
   */
  gcc_pure
  short GetInterpolatedHeight(unsigned int lx,
                              unsigned int ly) const;

  /**
   * Scan a straight line and fill the buffer with the specified
   * number of samples along the line.
   *
   * @param start the sub-pixel start location
   * @param end the sub-pixel end location
   */
  void ScanLine(const RasterLocation start, const RasterLocation end,
                short *buffer, unsigned size, bool interpolate) const;

  bool FirstIntersection(int origin_x, int origin_y,
                         int destination_x, int destination_y,
                         short h_origin,
                         short h_dest,
                         const int slope_fact, const short h_ceiling,
                         const short h_safety,
                         unsigned& int_x, unsigned& int_y, short &h_int,
                         const bool can_climb) const;

  gcc_pure RasterLocation
  Intersection(int origin_x, int origin_y,
               int destination_x, int destination_y,
               short h_origin, const int slope_fact) const;

protected:
  void LoadJPG2000(const char *path);

  /**
   * Load a world file (*.tfw or *.j2w).
   */
  bool LoadWorldFile(const TCHAR *path);

  /**
   * Get field (not interpolated) directly, without bringing tiles to front.
   * @param px X position/256
   * @param px Y position/256
   * @param tile_index Remember position of active tile, or -1 for overview
   */
  short GetFieldDirect(const unsigned px, const unsigned py, int &tile_index) const;

public:
  bool LoadOverview(const char *path, const TCHAR *world_file,
                    OperationEnvironment &operation);

  bool SaveCache(FILE *file) const;
  bool LoadCache(FILE *file);

  void UpdateTiles(const char *path, int x, int y, unsigned radius);

  /**
   * Determines if there are still tiles scheduled to be loaded.  Call
   * this after UpdateTiles() to determine if UpdateTiles() should be
   * called again soon.
   */
  bool IsDirty() const {
    return dirty;
  }

  bool GetInitialised() const {
    return initialised;
  }

  const Serial &GetSerial() const {
    return serial;
  }

  void Reset();

  const GeoBounds &GetBounds() const {
    assert(bounds_initialised);

    return bounds;
  }

private:
  gcc_pure
  const MarkerSegmentInfo *
  FindMarkerSegment(uint32_t file_offset) const;

public:
  /* callback methods for libjasper (via jas_rtc.cpp) */

  long SkipMarkerSegment(long file_offset) const;
  void MarkerSegment(long file_offset, unsigned id);

  bool TileRequest(unsigned index);

  short *GetOverview() {
    return Overview.get_data();
  }

  void SetSize(unsigned width, unsigned height,
               unsigned tile_width, unsigned tile_height,
               unsigned tile_columns, unsigned tile_rows);
  short* GetImageBuffer(unsigned index);
  void SetLatLonBounds(double lon_min, double lon_max,
                       double lat_min, double lat_max);
  void SetTile(unsigned index, int xstart, int ystart, int xend, int yend);

  void SetInitialised(bool val) {
    initialised = val;
  }

protected:
  bool PollTiles(int x, int y, unsigned radius);

public:
  short GetMaxElevation() const {
    return Overview.get_max();
  }

  unsigned int GetWidth() const { return width; }
  unsigned int GetHeight() const { return height; }
};

#endif
