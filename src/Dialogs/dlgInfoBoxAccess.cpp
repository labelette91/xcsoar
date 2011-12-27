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
#include "Dialogs/dlgInfoBoxAccess.hpp"
#include "Dialogs/Internal.hpp"
#include "UIGlobals.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Components.hpp"
#include "Look/Look.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "InfoBoxes/InfoBoxLayout.hpp"
#include "Form/TabBar.hpp"
#include "Form/Form.hpp"
#include "Form/Panel.hpp"
#include "Form/PanelWidget.hpp"

#include <assert.h>
#include <stdio.h>

class CloseInfoBoxAccess : public PanelWidget {
protected:
  /**
   * The parent form that needs to be closed
   */
  WndForm &wf;
public:
  CloseInfoBoxAccess(WndForm &_wf) :
    wf(_wf) {
  }
  virtual bool Click();
  virtual void ReClick();
};

class SwitchInfoBox : public PanelWidget {
protected:

  /**
   * The parent form that needs to be closed
   * after the SwitchInfoBox popup routine is called
   */
  WndForm &wf;

  /**
   * id of the InfoBox
   */
  int id;
public:
  SwitchInfoBox(int _id, WndForm &_wf) :
    wf(_wf), id(_id) {
  }
  virtual bool Click();
  virtual void ReClick();
};

static WndForm *wf = NULL;

static TabBarControl* wTabBar = NULL;

void
dlgInfoBoxAccessShowModeless(const int id)
{
  dlgInfoBoxAccess::dlgInfoBoxAccessShowModeless(id);
}

void
dlgInfoBoxAccess::dlgInfoBoxAccessShowModeless(const int id)
{
  // check for another instance of this window
  if (wf != NULL) return;
  assert (id > -1);

  const InfoBoxContent::DialogContent *dlgContent;
  dlgContent = InfoBoxManager::GetDialogContent(id);

  const DialogLook &look = UIGlobals::GetDialogLook();

  PixelRect form_rc = InfoBoxManager::layout.remaining;
  form_rc.top = form_rc.bottom - Layout::Scale(107);

  wf = new WndForm(UIGlobals::GetMainWindow(), look, form_rc);

  WindowStyle tab_style;
  tab_style.control_parent();
  ContainerWindow &client_area = wf->GetClientAreaWindow();
  const PixelRect rc = client_area.get_client_rect();
  wTabBar = new TabBarControl(client_area, look, rc.left, rc.top,
                              rc.right - rc.left, Layout::Scale(45),
                              tab_style, Layout::landscape);

  if (dlgContent != NULL) {
    for (int i = 0; i < dlgContent->PANELSIZE; i++) {
      assert(dlgContent->Panels[i].load != NULL);

      Widget *widget = dlgContent->Panels[i].load(id);

      if (widget == NULL)
        continue;

      wTabBar->AddTab(widget, gettext(dlgContent->Panels[i].name));
    }
  }

  if (!wTabBar->GetTabCount()) {
    form_rc.top = form_rc.bottom - Layout::Scale(58);
    wf->move(form_rc.left, form_rc.top, form_rc.right - form_rc.left, form_rc.bottom - form_rc.top);

    Widget *wSwitch = new SwitchInfoBox(id, *wf);
    wTabBar->AddTab(wSwitch, _("Switch InfoBox"));
  }

  Widget *wClose = new CloseInfoBoxAccess(*wf);
  wTabBar->AddTab(wClose, _("Close"));

  InfoBoxSettings &settings = CommonInterface::SetUISettings().info_boxes;
  const unsigned panel_index = InfoBoxManager::GetCurrentPanel();
  InfoBoxSettings::Panel &panel = settings.panels[panel_index];
  const InfoBoxFactory::t_InfoBox old_type = panel.contents[id];

  StaticString<32> buffer;
  buffer = gettext(InfoBoxFactory::GetName(old_type));

  wf->SetCaption(buffer);
  wf->ShowModeless();

  bool changed = false, require_restart = false;
  wTabBar->Save(changed, require_restart);

  delete wTabBar;
  delete wf;
  // unset wf because wf is still static and public
  wf = NULL;
}

bool
dlgInfoBoxAccess::OnClose()
{
  wf->SetModalResult(mrOK);
  return true;
}

bool
CloseInfoBoxAccess::Click()
{
  ReClick();
  return false;
}

void
CloseInfoBoxAccess::ReClick()
{
  wf.SetModalResult(mrOK);
}

bool
SwitchInfoBox::Click()
{
  ReClick();
  return false;
}

void
SwitchInfoBox::ReClick()
{
  InfoBoxManager::ShowInfoBoxPicker(id);
  wf.SetModalResult(mrOK);
}
