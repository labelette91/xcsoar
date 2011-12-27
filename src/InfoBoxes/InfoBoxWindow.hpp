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

#ifndef XCSOAR_INFO_BOX_HPP
#define XCSOAR_INFO_BOX_HPP

#include "Util/StaticString.hpp"
#include "InfoBoxes/Content/Base.hpp"
#include "Screen/PaintWindow.hpp"
#include "Screen/Timer.hpp"
#include "PeriodClock.hpp"
#include "Data.hpp"

enum BorderKind_t {
  bkNone,
  bkTop,
  bkRight,
  bkBottom,
  bkLeft
};

#define BORDERTOP    (1<<bkTop)
#define BORDERRIGHT  (1<<bkRight)
#define BORDERBOTTOM (1<<bkBottom)
#define BORDERLEFT   (1<<bkLeft)

struct InfoBoxSettings;
struct InfoBoxLook;

class InfoBoxWindow : public PaintWindow
{
  /** timeout of infobox focus [ms] */
  static gcc_constexpr_data unsigned FOCUS_TIMEOUT_MAX = 20 * 1000;

private:
  InfoBoxContent *content;
  ContainerWindow &parent;

  const InfoBoxSettings &settings;
  const InfoBoxLook &look;

  int border_kind;

  InfoBoxData data;

  int id;

  /**
   * draw the selector event if the InfoBox window is not the system focus
   */
  bool force_draw_selector;

  /** a timer which returns keyboard focus back to the map window after a while */
  WindowTimer focus_timer;

  PixelRect title_rect;
  PixelRect value_rect;
  PixelRect comment_rect;
  PixelRect value_and_comment_rect;

  PeriodClock click_clock;

  /**
   * Paints the InfoBox title to the given canvas
   * @param canvas The canvas to paint on
   */
  void PaintTitle(Canvas &canvas);
  /**
   * Paints the InfoBox value to the given canvas
   * @param canvas The canvas to paint on
   */
  void PaintValue(Canvas &canvas);
  /**
   * Paints the InfoBox comment on the given canvas
   * @param canvas The canvas to paint on
   */
  void PaintComment(Canvas &canvas);
  /**
   * Paints the InfoBox with borders, title, comment and value
   */
  void Paint(Canvas &canvas);

public:
  void PaintInto(Canvas &dest, PixelScalar xoff, PixelScalar yoff,
                 UPixelScalar width, UPixelScalar height);

  /**
   * Sets the InfoBox ID to the given Value
   * @param id New value of the InfoBox ID
   */
  void SetID(const int id);
  int GetID() { return id; };
  /**
   * Sets the InfoBox title to the given Value
   * @param Value New value of the InfoBox title
   */
  void SetTitle(const TCHAR *title);

  const TCHAR* GetTitle() {
    return data.title;
  };

  /**
   * Constructor of the InfoBoxWindow class
   * @param Parent The parent ContainerWindow (usually MainWindow)
   * @param X x-Coordinate of the InfoBox
   * @param Y y-Coordinate of the InfoBox
   * @param Width Width of the InfoBox
   * @param Height Height of the InfoBox
   */
  InfoBoxWindow(ContainerWindow &parent, PixelScalar x, PixelScalar y,
                UPixelScalar width, UPixelScalar height, int border_flags,
                const InfoBoxSettings &settings, const InfoBoxLook &_look,
                WindowStyle style=WindowStyle());

  ~InfoBoxWindow();

  const InfoBoxLook &GetLook() const {
    return look;
  }

  void SetContentProvider(InfoBoxContent *_content);
  void UpdateContent();

protected:
  bool HandleKey(InfoBoxContent::InfoBoxKeyCodes keycode);

public:
  /**
   * This passes a given value to the InfoBoxContent for further processing
   * and updates the InfoBox.
   * @param Value Value to handle
   * @return True on success, Fales otherwise
   */
  bool HandleQuickAccess(const TCHAR *value);

  const InfoBoxContent::DialogContent *GetDialogContent();

  const PixelRect GetValueRect() const {
    return value_rect;
  }
  const PixelRect GetValueAndCommentRect() const {
    return value_and_comment_rect;
  }

protected:
  virtual void on_destroy();

  /**
   * This event handler is called when a key is pressed down while the InfoBox
   * is focused
   * @param key_code The code of the key that was pressed
   * @return True if the event has been handled, False otherwise
   */
  virtual bool on_key_down(unsigned key_code);
  /**
   * This event handler is called when a mouse button is pressed down over
   * the InfoBox
   * @param x x-Coordinate where the mouse button was pressed
   * @param y y-Coordinate where the mouse button was pressed
   * @return True if the event has been handled, False otherwise
   */
  virtual bool on_mouse_down(PixelScalar x, PixelScalar y);

  virtual bool on_mouse_up(PixelScalar x, PixelScalar y);

  /**
   * This event handler is called when a mouse button is double clicked over
   * the InfoBox
   * @param x x-Coordinate where the mouse button was pressed
   * @param y y-Coordinate where the mouse button was pressed
   * @return True if the event has been handled, False otherwise
   */
  virtual bool on_mouse_double(PixelScalar x, PixelScalar y);

  virtual void on_resize(UPixelScalar width, UPixelScalar height);

  /**
   * This event handler is called when the InfoBox needs to be repainted
   * @param canvas The canvas to paint on
   */
  virtual void on_paint(Canvas &canvas);

  virtual bool on_cancel_mode();
  virtual void on_setfocus();
  virtual void on_killfocus();

  /**
   * This event handler is called when a timer is triggered
   * @param id Id of the timer that triggered the handler
   */
  virtual bool on_timer(WindowTimer &timer);
};

#endif
