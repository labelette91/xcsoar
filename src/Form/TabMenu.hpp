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

#ifndef XCSOAR_FORM_TABMENU_HPP
#define XCSOAR_FORM_TABMENU_HPP

#include "Util/StaticArray.hpp"
#include "Form/Tabbed.hpp"
#include "Form/TabBar.hpp"
#include "Dialogs/XML.hpp"

struct DialogLook;
class WndForm;
class TabMenuDisplay;
class OneSubMenuButton;
class OneMainMenuButton;
class ContainerWindow;

/** TabMenuControl is a two-level menu structure.
 * The menu items are linked to Tab panels
 * The tabbed panels are loaded via code, not via XML
 *
 * The menu structure is initialized via an array parameter
 * from the client.
 * Creates a vertical menu in both Portrait and in landscape modes
 * ToDo: lazy loading of panels (XML and Init() routines)
 */
class TabMenuControl : public ContainerWindow {
public:

  /**List of all submenu items in array of PageItem[0 to (n-1)]
   * The menus must be sorrted by main_menu_index and the order to appear
   */
  struct PageItem {
    const TCHAR *menu_caption;

     /* The main menu page Enter menu page into the array
      * 0 to (GetNumMainMenuCaptions() - 1) */
    unsigned main_menu_index;

    Widget *(*Load)();
  };

  enum tab_menu_values {  // fix indent
      MAX_MAIN_MENU_ITEMS = 7, // excludes "Main Menu" which is a "super menu"
      LARGE_VALUE = 1000,
      MAIN_MENU_PAGE = 999,
  };

  /* internally used structure for tracking menu down and selection status */
  struct MenuTabIndex {
    static const unsigned NO_MAIN_MENU = 997;
    static const unsigned NO_SUB_MENU = 998;

    unsigned main_index;
    unsigned sub_index;

    gcc_constexpr_ctor
    explicit MenuTabIndex(unsigned mainNum, unsigned subNum=NO_SUB_MENU)
      :main_index(mainNum), sub_index(subNum) {}

    gcc_constexpr_function
    static MenuTabIndex None() {
      return MenuTabIndex(NO_MAIN_MENU, NO_SUB_MENU);
    }

    gcc_constexpr_method
    bool IsNone() const {
      return main_index == NO_MAIN_MENU;
    }

    gcc_constexpr_method
    bool IsMain() const {
      return main_index != NO_MAIN_MENU && sub_index == NO_SUB_MENU;
    }

    gcc_constexpr_method
    bool IsSub() const {
      return sub_index != NO_SUB_MENU;
    }

    gcc_constexpr_method
    bool operator==(const MenuTabIndex &other) const {
      return main_index == other.main_index &&
        sub_index == other.sub_index;
    }
  };

protected:
  TabbedControl pager;

  StaticArray<OneSubMenuButton *, 32> buttons;

  /* holds info and buttons for the main menu.  not on child menus */
  StaticArray<OneMainMenuButton *, MAX_MAIN_MENU_ITEMS> main_menu_buttons;

  TabMenuDisplay *tab_display;

  /* holds pointer to array of menus info (must be sorted by MenuGroup) */
  PageItem const *pages;

  MenuTabIndex last_content;

  StaticString<256u> caption;

  // used to indicate initial state before tabs have changed
  bool setting_up;

  WndForm &form;
  const CallBackTableEntry *look_up_table;

public:
  /**
   * @param parent
   * @param Caption the page caption shown on the menu page
   * @param x, y Location of the tab bar. (frame of the content windows)
   * @param width, height.  Size of the tab bar
   * @param style
   * @return
   */
  TabMenuControl(ContainerWindow &parent,
                 WndForm &_form, const CallBackTableEntry *_look_up_table,
                 const DialogLook &look, const TCHAR * Caption,
                 PixelScalar x, PixelScalar y,
                 UPixelScalar width, UPixelScalar height,
                 const WindowStyle style = WindowStyle());
  ~TabMenuControl();

  const StaticArray<OneSubMenuButton *, 32> &GetTabButtons() {
    return buttons;
  }

public:
  /**
   * Initializes the menu and buids it from the Menuitem[] array
   *
   * @param menus[] Array of PageItem elements to be displayed in the menu
   * @param num_menus Size the menus array
   * @param main_menu_captions Array of captions for main menu items
   * @param num_menu_captions Aize of main_menu_captions array
   */
  void InitMenu(const PageItem menus[],
                unsigned num_menus,
                const TCHAR *main_menu_captions[],
                unsigned num_menu_captions);

  /**
   * @return true if currently displaying the menu page
   */
  gcc_pure
  bool IsCurrentPageTheMenu() const {
    return GetCurrentPage() == GetMenuPage();
  }

  /**
   * Sets the current page to the menu page
   * @return Page index of menu page
   */
  unsigned GotoMenuPage();

  unsigned GetNumMainMenuItems() const {
    return main_menu_buttons.size();
  }

  /**
   * Calculates and caches the size and position of ith sub menu button
   * All menus (landscape or portrait) are drawn vertically
   * @param i Index of button
   * @return Rectangle of button coordinates,
   *   or {0,0,0,0} if index out of bounds
   */
  gcc_pure
  const PixelRect &GetSubMenuButtonSize(unsigned i) const;

  /**
   * Calculates and caches the size and position of ith main menu button
   * All menus (landscape or portrait) are drawn vertically
   * @param i Index of button
   * @return Rectangle of button coordinates,
   *   or {0,0,0,0} if index out of bounds
   */
  gcc_pure
  const PixelRect &GetMainMenuButtonSize(unsigned i) const;

  /**
   * @param main_menu_index
   * @return pointer to button or NULL if index is out of range
   */
  gcc_pure
  const OneMainMenuButton *GetMainMenuButton(unsigned main_menu_index) const;

  /**
   * @param page
   * @return pointer to button or NULL if index is out of range
   */
  gcc_pure
  const OneSubMenuButton *GetSubMenuButton(unsigned page) const;

  /**
   * @param Pos position of pointer
   * @param mainIndex main menu whose submenu buttons are visible
   * @return MenuTabIndex w/ location of item
   */
  gcc_pure
  MenuTabIndex IsPointOverButton(RasterPoint Pos, unsigned mainIndex) const;

  /**
   * @return number of pages excluding the menu page
   */
  unsigned GetNumPages() const{
    return pager.GetTabCount() - 1;
  }

  /**
   *  returns menu item from data array of pages
   */
  const PageItem &GetPageItem(unsigned page) const {
    assert(page < GetNumPages());
    return pages[page];
  }

  /**
   * Looks up the page id from the menu table
   * based on the main menu and sub menu index parameters
   *
   * @MainMenuIndex Index from main menu
   * @SubMenuIndex Index within submenu
   * returns page number of selected sub menu item base on menus indexes
   *  or GetMenuPage() if index is not a valid page
   */
  gcc_pure
  int GetPageNum(MenuTabIndex i) const;

  /**
   * overloads from TabBarControl.
   */
  UPixelScalar GetTabHeight() const;

  /**
   * @return last content page shown (0 to (NumPages-1))
   * or MAIN_MENU_PAGE if no content pages have been shown
   */
  gcc_pure
  unsigned GetLastContentPage() const {
    return GetPageNum(last_content);
  }

  const StaticArray<OneMainMenuButton *, MAX_MAIN_MENU_ITEMS>
      &GetMainMenuButtons() { return main_menu_buttons; }

protected:
  /**
   * @returns Page Index being displayed (including NumPages for Menu)
   */
  unsigned GetCurrentPage() const {
    return pager.GetCurrentPage();
  }

protected:

  /**
   * @return virtual menu page -- one greater than size of the menu array
   */
  unsigned GetMenuPage() const {
    return GetNumPages();
  }

  TabMenuDisplay *GetTabMenuDisplay() {
    return tab_display;
  }

  /**
   * appends a submenu button to the buttons array and
   * loads it's XML file
   *
   * @param sub_menu_index Index of submenu item within in submenu
   * @param item The PageItem to be created
   * @param page Index to the page array
   */
  void CreateSubMenuItem(const unsigned sub_menu_index,
                         const PageItem &item,
                         const unsigned page);

  /** for each main menu item:
   *   adds to the MainMenuButtons array
   *   loops through all submenu items and loads them
   *
   * @param main_menu_caption The caption of the parent menu item
   * @param MainMenuIndex 0 to (MAX_SUB_MENUS-1)
   */
  void
  CreateSubMenu(const PageItem pages_in[], unsigned num_pages,
                const TCHAR *main_menu_caption,
                const unsigned main_menu_index);

  /**
   * @return Height of any item in Main or Sub menu
   */
  UPixelScalar GetMenuButtonHeight() const;

  /**
   * @return Width of any item in Main or Sub menu
   */
  UPixelScalar GetMenuButtonWidth() const;

  /**
   * @return for portrait mode, puts menu near vertical center of screen
   */
  UPixelScalar GetButtonVerticalOffset() const;

public:
  void NextPage();
  void PreviousPage();
  void SetCurrentPage(unsigned page);
  void SetCurrentPage(MenuTabIndex menuIndex);

  bool Save(bool &changed, bool &require_restart) {
    return pager.Save(changed, require_restart);
  }

  /**
   * sets page that determines which menu item is displayed as selected
   * @param page
   */
  void SetLastContentPage(unsigned page);

  /**
   * if displaying the menu page, will select/highlight the next menu item
   */
  void HighlightNextMenuItem();

  /**
   * if displaying the menu page, will select/highlight the previous menu item
   */
  void HighlightPreviousMenuItem();
};

class TabMenuDisplay : public PaintWindow {
  TabMenuControl &menu;
  const DialogLook &look;
  bool dragging; // tracks that mouse is down and captured
  bool drag_off_button; // set by mouse_move

public:
  /* used to track mouse down/up clicks */
  TabMenuControl::MenuTabIndex down_index;
  /* used to render which submenu is drawn and which item is highlighted */
  TabMenuControl::MenuTabIndex selected_index;

public:
  TabMenuDisplay(TabMenuControl &_menu, const DialogLook &look,
                 ContainerWindow &parent,
                 PixelScalar left, PixelScalar top,
                 UPixelScalar width, UPixelScalar height);

  void SetSelectedIndex(TabMenuControl::MenuTabIndex di);

  UPixelScalar GetTabHeight() const {
    return this->get_height();
  }

  UPixelScalar GetTabWidth() const {
    return this->get_width();
  }

  /**
   * Returns index of selected (highlighted) tab
   * @return
   */
  const TabMenuControl::MenuTabIndex GetSelectedIndex() { return selected_index; }

protected:
  TabMenuControl &GetTabMenuBar() {
    return menu;
  }

  void drag_end();

  /**
   * @return Rect of button holding down pointer capture
   */
  const PixelRect& GetDownButtonRC();

  virtual bool on_mouse_move(PixelScalar x, PixelScalar y, unsigned keys);
  virtual bool on_mouse_up(PixelScalar x, PixelScalar y);
  virtual bool on_mouse_down(PixelScalar x, PixelScalar y);
  virtual bool on_key_check(unsigned key_code) const;
  virtual bool on_key_down(unsigned key_code);
  /**
   * canvas is the tabmenu which is the full content window, no content
   * @param canvas
   * Todo: support icons and "ButtonOnly" style
   */
  virtual void on_paint(Canvas &canvas);

  virtual void on_killfocus();
  virtual void on_setfocus();

  /**
   * draw border around main menu
   */
  void PaintMainMenuBorder(Canvas &canvas);
  void PaintMainMenuItems(Canvas &canvas, const unsigned CaptionStyle);
  void PaintSubMenuBorder(Canvas &canvas, const OneMainMenuButton *butMain);
  void PaintSubMenuItems(Canvas &canvas, const unsigned CaptionStyle);
};

/**
 * class that holds the child menu button and info for the menu
 */
class OneSubMenuButton : public OneTabButton {
public:
  const TabMenuControl::MenuTabIndex menu;
  const unsigned page_index;

public:
  OneSubMenuButton(const TCHAR* _Caption, TabMenuControl::MenuTabIndex i,
                   unsigned _page_index)
    :OneTabButton(_Caption, false, NULL),
     menu(i),
     page_index(_page_index)
  {
  }
};

/**
 * class that holds the main menu button and info
 */
class OneMainMenuButton : public OneTabButton {
public:
  /* index to Pages array of first page in submenu */
  const unsigned first_page_index;

  /* index to Pages array of last page in submenu */
  const unsigned last_page_index;

  /* index of button in MainMenu */
  const unsigned main_menu_index;

  OneMainMenuButton(const TCHAR* _Caption,
                    unsigned _first_page_index,
                    unsigned _last_page_index,
                    unsigned _main_menu_index)
    :OneTabButton(_Caption, false, NULL),
     first_page_index(_first_page_index),
     last_page_index(_last_page_index),
     main_menu_index(_main_menu_index)
  {
  }

public:
  unsigned NumSubMenus() const { return last_page_index - first_page_index + 1; };
};
#endif
