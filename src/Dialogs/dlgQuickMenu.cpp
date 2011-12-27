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

#include "Dialogs/Dialogs.h"
#include "Form/GridView.hpp"
#include "Form/CustomButton.hpp"
#include "Input/InputEvents.hpp"
#include "Screen/Key.h"
#include "Screen/SingleWindow.hpp"
#include "Form/Form.hpp"
#include "Util/TrivialArray.hpp"
#include "Util/Macros.hpp"
#include "Menu/ButtonLabel.hpp"
#include "UIGlobals.hpp"

#include <stdio.h>

static WndForm *wf;
static GridView *grid_view;

static TrivialArray<Window *, GridView::MAX_ITEMS> buttons;
static TrivialArray<unsigned, GridView::MAX_ITEMS> events;

static void
SetFormCaption()
{
  StaticString<32> buffer;
  unsigned pageSize = grid_view->GetNumColumns() * grid_view->GetNumRows();
  unsigned lastPage = buttons.size() / pageSize;
  buffer.Format(_T("Quick Menu  %d/%d"),
                grid_view->GetCurrentPage() + 1, lastPage + 1);
  wf->SetCaption(buffer);
}

static void
SetFormDefaultFocus()
{
  unsigned numColumns = grid_view->GetNumColumns();
  unsigned pageSize = numColumns * grid_view->GetNumRows();
  unsigned lastPage = buttons.size() / pageSize;
  unsigned currentPage = grid_view->GetCurrentPage();
  unsigned currentPageSize = currentPage == lastPage
    ? buttons.size() % pageSize
    : pageSize;
  unsigned centerCol = currentPageSize < numColumns
    ? currentPageSize / 2
    : numColumns / 2;
  unsigned centerRow = currentPageSize / numColumns / 2;
  unsigned centerPos = currentPage
    * pageSize + centerCol + centerRow * numColumns;

  if (centerPos < buttons.size()) {
    if (wf->is_visible()) {
      buttons[centerPos]->set_focus();
      grid_view->RefreshLayout();
    } else if (buttons[centerPos]->is_enabled())
      wf->SetDefaultFocus(buttons[centerPos]);
  }
}

static bool
FormKeyDown(gcc_unused WndForm &Sender, unsigned key_code)
{
  switch (key_code) {
  case VK_LEFT:
    grid_view->MoveFocus(GridView::Direction::LEFT);
    break;

  case VK_RIGHT:
    grid_view->MoveFocus(GridView::Direction::RIGHT);
    break;

  case VK_UP:
    grid_view->MoveFocus(GridView::Direction::UP);
    break;

  case VK_DOWN:
    grid_view->MoveFocus(GridView::Direction::DOWN);
    break;

  case VK_MENU:
    grid_view->ShowNextPage();
    SetFormDefaultFocus();
    break;

  default:
    return false;
  }

  SetFormCaption();
  return true;
}

static void
OnButtonNextClicked(gcc_unused WndButton &sender)
{
  grid_view->ShowNextPage();
  SetFormDefaultFocus();
}

static void
OnButtonClicked(gcc_unused WndButton &sender)
{
  signed focusPos = grid_view->GetIndexOfItemInFocus();

  wf->SetModalResult(mrOK);

  if (focusPos != -1) {
    wf->hide();
    unsigned event = events[focusPos];
    if (event > 0)
      InputEvents::ProcessEvent(event);
  }
}

void
dlgQuickMenuShowModal(SingleWindow &parent)
{
  const Menu *menu = InputEvents::GetMenu(_T("RemoteStick"));
  if (menu == NULL)
    return;

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();

  WindowStyle dialogStyle;
  dialogStyle.hide();
  dialogStyle.control_parent();

  wf = new WndForm(parent, dialog_look, parent.get_client_rect(),
                   _T("Quick Menu"), dialogStyle);

  ContainerWindow &client_area = wf->GetClientAreaWindow();

  PixelRect r = client_area.get_client_rect();

  WindowStyle grid_view_style;
  grid_view_style.control_parent();

  grid_view = new GridView(client_area,
                           r.left, r.top,
                           r.right - r.left, r.bottom - r.top,
                           dialog_look, grid_view_style);

  WindowStyle buttonStyle;
  buttonStyle.tab_stop();

  for (unsigned i = 0; i < menu->MAX_ITEMS; ++i) {
    if (buttons.full())
      continue;

    const MenuItem &menuItem = (*menu)[i];
    if (!menuItem.IsDefined())
      continue;

    TCHAR buffer[100];
    ButtonLabel::Expanded expanded =
      ButtonLabel::Expand(menuItem.label, buffer, ARRAY_SIZE(buffer));
    if (!expanded.visible)
      continue;

    PixelRect button_rc;
    button_rc.left = 0;
    button_rc.top = 0;
    button_rc.right = 80;
    button_rc.bottom = 30;
    void (*ptOnButtonClicked)(gcc_unused WndButton &sender);
    if ( _tcsicmp(menuItem.label  , _T("Next") ) == 0 )
        ptOnButtonClicked =  OnButtonNextClicked ;
    else
        ptOnButtonClicked =  OnButtonClicked ;
    	
    WndButton *button =
      new WndCustomButton(*grid_view, dialog_look, expanded.text,
                          button_rc, buttonStyle, ptOnButtonClicked);
    button->set_enabled(expanded.enabled);

    buttons.append(button);
    events.append(menuItem.event);
  }

  grid_view->SetItems(buttons);
  SetFormDefaultFocus();
  SetFormCaption();

  wf->SetKeyDownNotify(FormKeyDown);

  wf->ShowModal();

  for (auto it = buttons.begin(), end = buttons.end(); it != end; ++it)
    delete *it;

  buttons.clear();
  events.clear();

  delete wf;
  delete grid_view;
}
