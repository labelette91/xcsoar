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

#ifndef XCSOAR_FORM_BUTTON_HPP
#define XCSOAR_FORM_BUTTON_HPP

#include "Screen/ButtonWindow.hpp"
#include "Renderer/ButtonRenderer.hpp"

struct DialogLook;
class ContainerWindow;

/**
 * This class is used for creating buttons.
 * It is based on the WindowControl class.
 */
class WndButton : public ButtonWindow {
public:
  typedef void (*ClickNotifyCallback)(WndButton &button);
  typedef void (*LeftRightNotifyCallback)(WndButton &button);

protected:
  const DialogLook &look;
  ButtonRenderer renderer;

private:
  /**
   * The callback-function that should be called when the button is pressed
   * @see SetOnClickNotify()
   */
  ClickNotifyCallback click_callback;

  /**
   * The callback-functions that should be called when the Left and Right
   * keys are pressed
   * @see SetOnLeftNotify() and SetOnRightNotify()
   */
  LeftRightNotifyCallback left_callback;
  LeftRightNotifyCallback right_callback;

public:
  /**
   * Constructor of the WndButton class
   * @param Parent Parent window/ContainerControl
   * @param Caption Text on the button
   * @param X x-Coordinate relative to the parent
   * @param Y y-Coordinate relative to the parent
   * @param Width Width of the button
   * @param Height Height of the button
   * @param Function The function that should be called
   * when the button is clicked
   */
  WndButton(ContainerWindow &parent, const DialogLook &look,
            const TCHAR *caption, const PixelRect &rc,
            ButtonWindowStyle style,
            ClickNotifyCallback click_callback = NULL,
            LeftRightNotifyCallback left_callback = NULL,
            LeftRightNotifyCallback right_callback = NULL);

  /**
   * Sets the function that should be called when the button is pressed
   * @param Function Pointer to the function to be called
   */
  void
  SetOnClickNotify(ClickNotifyCallback _click_callback)
  {
    click_callback = _click_callback;
  }

  /**
   * Sets the function that should be called when the Left key is pressed
   * @param Function Pointer to the function to be called
   */
  void
  SetOnLeftNotify(LeftRightNotifyCallback _left_callback)
  {
    left_callback = _left_callback;
  }

  /**
   * Sets the function that should be called when the Right key is pressed
   * @param Function Pointer to the function to be called
   */
  void
  SetOnRightNotify(LeftRightNotifyCallback _right_callback)
  {
    right_callback = _right_callback;
  }
  /**
   * Sets the Caption/Text of the Control and resets the cached text height
   * (derived from WindowControl)
   * @param Value The new Caption/Text of the Control
   */
  void SetCaption(const TCHAR *caption) {
    set_text(caption);
  }

  /**
   * Called when the button is clicked (either by mouse or by
   * keyboard).  The default implementation invokes the OnClick
   * callback.
   */
  virtual bool on_clicked();

private:
  /**
   * Called when the Left (Right) key is pressed
   * Only called if callback have been explicitly set
   */
  bool on_left();
  bool on_right();

protected:
  virtual bool on_key_check(unsigned key_code) const;
  virtual bool on_key_down(unsigned key_code);
  virtual void on_paint(Canvas &canvas);
};

#endif
