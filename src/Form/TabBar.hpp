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

#ifndef XCSOAR_FORM_TABBAR_HPP
#define XCSOAR_FORM_TABBAR_HPP

#include "Util/StaticArray.hpp"
#include "Form/Tabbed.hpp"
#include "Util/StaticString.hpp"

struct DialogLook;
class Bitmap;
class WndOwnerDrawFrame;
class ContainerWindow;
class TabDisplay;
class OneTabButton;

/** TabBarControl displays tabs that show/hide the windows
 * associated with each tab.  For example a "Panel" control.
 * It can also display buttons with no associated Window.
 * Supports pre and post- tab click callbacks
 * Each tab must be added via code (not via XML)
 * ToDo: support lazy loading
 */
class TabBarControl : public ContainerWindow {

protected:
  TabbedControl pager;

  TabDisplay * tab_display;
  StaticArray<OneTabButton *, 32> buttons;
  const UPixelScalar tab_line_height;
  bool flip_orientation;
  /** if false (default) Client rectangle is adjacent to tabs
   *  if true, Client rectangle overlaps tabs (for advanced drawing)
   */
  bool client_overlap_tabs;

public:
  /**
   * Constructor used for stand-alone TabBarControl
   * @param parent
   * @param x, y Location of the tab bar (unused)
   * @param width, height.  Size of the tab bar
   * @param style
   * @return
   */
  TabBarControl(ContainerWindow &parent, const DialogLook &look,
                PixelScalar x, PixelScalar y,
                UPixelScalar width, UPixelScalar height,
                const WindowStyle style = WindowStyle(),
                bool _flipOrientation = false,
                bool _clientOverlapTabs = false);

  ~TabBarControl();

  void SetPageFlippedCallback(TabbedControl::PageFlippedCallback cb) {
    pager.SetPageFlippedCallback(cb);
  }

private:
#define TabLineHeightInitUnscaled (unsigned)5

public:
  unsigned AddTab(Widget *widget, const TCHAR *caption,
                  bool button_only=false,
                  const Bitmap *bmp=NULL);

public:
  gcc_pure
  unsigned GetTabCount() const {
    return pager.GetTabCount();
  }

  gcc_pure
  unsigned GetCurrentPage() const {
    return pager.GetCurrentPage();
  }

  gcc_pure
  const Widget *GetCurrentWidget() const {
    return pager.GetCurrentWidget();
  }

  void ClickPage(unsigned i);

  void SetCurrentPage(unsigned i);
  void NextPage();
  void PreviousPage();

  bool Save(bool &changed, bool &require_restart) {
    return pager.Save(changed, require_restart);
  }

  gcc_pure
  UPixelScalar GetTabHeight() const;

  gcc_pure
  UPixelScalar GetTabWidth() const;

  /**
   * calculates the size and position of ith button
   * works in landscape or portrait mode
   * @param i index of button
   * @return Rectangle of button coordinates
   */
  gcc_pure
  const PixelRect &GetButtonSize(unsigned i) const;

  gcc_pure
  const TCHAR *GetButtonCaption(unsigned i) const;

  gcc_pure
  const Bitmap *GetButtonIcon(unsigned i) const;

  gcc_pure
  bool GetButtonIsButtonOnly(unsigned i) const;

  const StaticArray<OneTabButton *, 32>
      &GetTabButtons() { return buttons; }

  UPixelScalar GetTabLineHeight() const {
    return tab_line_height;
  }

  void SetClientOverlapTabs(bool value);
};

/**
 * TabDisplay class handles onPaint callback for TabBar UI
 * and handles Mouse and key events
 * TabDisplay uses a pointer to TabBarControl
 * to show/hide the appropriate pages in the Container Class
 */
class TabDisplay: public PaintWindow
{
protected:
  TabBarControl& tab_bar;
  const DialogLook &look;
  bool dragging; // tracks that mouse is down and captured
  int down_index; // index of tab where mouse down occurred
  bool drag_off_button; // set by mouse_move
  bool flip_orientation;

public:
  /**
   *
   * @param parent
   * @param _theTabBar. An existing TabBar object
   * @param left. Left position of the tab bar box in the parent window
   * @param top Top position of the tab bar box in the parent window
   * @param width Width of tab bar box in the parent window
   * @param height Height of tab bar box in the parent window
   */
  TabDisplay(TabBarControl& _theTabBar, const DialogLook &look,
             ContainerWindow &parent,
             PixelScalar left, PixelScalar top,
             UPixelScalar width, UPixelScalar height,
             bool _flipOrientation = false);

  /**
   * Paints one button
   */
  static void PaintButton(Canvas &canvas, const unsigned CaptionStyle,
                          const TCHAR *caption, const PixelRect &rc,
                          bool isButtonOnly, const Bitmap *bmp,
                          const bool isDown, bool inverse);

  /**
   * @return -1 if there is no button at the specified position
   */
  gcc_pure
  int GetButtonIndexAt(RasterPoint p) const;

public:
  UPixelScalar GetTabHeight() const {
    return this->get_height();
  }

  UPixelScalar GetTabWidth() const {
    return this->get_width();
  }

protected:
  /**
   * paints the tab buttons
   * @param canvas
   */
  virtual void on_paint(Canvas &canvas);
  //ToDo: support function buttons

  /**
   * track key presses to navigate without mouse
   * @param key_code
   * @return
   */
  virtual void on_killfocus();
  virtual void on_setfocus();
  virtual bool on_key_check(unsigned key_code) const;
  virtual bool on_key_down(unsigned key_code);

  /**
   * track mouse clicks
   */
  virtual bool on_mouse_down(PixelScalar x, PixelScalar y);
  virtual bool on_mouse_up(PixelScalar x, PixelScalar y);
  virtual bool on_mouse_move(PixelScalar x, PixelScalar y, unsigned keys);
  void drag_end();
};

/**
 * OneTabButton class holds display and callbacks data for a single tab
 */
class OneTabButton {
public:
  StaticString<32> caption;
  bool is_button_only;
  const Bitmap *bmp;
  PixelRect but_size;

public:
  OneTabButton(const TCHAR* _Caption,
               bool _IsButtonOnly,
               const Bitmap *_bmp)
    :is_button_only(_IsButtonOnly),
     bmp(_bmp)
  {
    caption = _Caption;
    but_size.left = 0;
    but_size.top = 0;
    but_size.right = 0;
    but_size.bottom = 0;
  };
};

#endif
