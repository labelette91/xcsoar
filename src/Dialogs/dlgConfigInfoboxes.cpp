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

#include "Dialogs/dlgConfigInfoboxes.hpp"
#include "Dialogs/Message.hpp"
#include "Dialogs/Dialogs.h"
#include "Dialogs/TextEntry.hpp"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "Form/Edit.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Fonts.hpp"
#include "Screen/SingleWindow.hpp"
#include "DataField/Enum.hpp"
#include "Compiler.h"
#include "InfoBoxes/InfoBoxSettings.hpp"
#include "InfoBoxes/InfoBoxLayout.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "InfoBoxes/Content/Factory.hpp"
#include "Compiler.h"

#include <assert.h>
#include <cstdio>
#include <algorithm>

class InfoBoxPreview : public PaintWindow {
protected:
  virtual bool on_mouse_down(PixelScalar x, PixelScalar y);
  virtual bool on_mouse_double(PixelScalar x, PixelScalar y);
  virtual void on_paint(Canvas &canvas);
};

static InfoBoxSettings::Panel data;
static WndForm *wf = NULL;
static InfoBoxSettings::Panel clipboard;
static unsigned clipboard_size;
static InfoBoxLayout::Layout info_box_layout;
static InfoBoxPreview previews[InfoBoxSettings::Panel::MAX_CONTENTS];
static unsigned current_preview;

static WndButton *buttonPanelName;
static WndProperty *edit_select;
static WndProperty *edit_content;
static WndButton *buttonPaste;

static void
RefreshPasteButton()
{
  buttonPaste->set_enabled(clipboard_size > 0);
}

static void
RefreshEditContent()
{
  DataFieldEnum &df = *(DataFieldEnum *)edit_content->GetDataField();
  df.Set(data.contents[current_preview]);
  edit_content->RefreshDisplay();
}

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static void
OnCopy(gcc_unused WndButton &button)
{
  clipboard = data;
  clipboard_size = InfoBoxSettings::Panel::MAX_CONTENTS;

  RefreshPasteButton();
}

static void
OnPaste(gcc_unused WndButton &button)
{
  if (clipboard_size == 0)
    return;

  if(MessageBoxX(_("Overwrite?"), _("InfoBox paste"),
                 MB_YESNO | MB_ICONQUESTION) != IDYES)
    return;

  for (unsigned item = 0; item < clipboard_size; item++) {
    InfoBoxFactory::t_InfoBox content = clipboard.contents[item];
    if (content >= InfoBoxFactory::NUM_TYPES)
      continue;

    data.contents[item] = content;
    previews[item].invalidate();
  }

  RefreshEditContent();
}

static void
SetCurrentInfoBox(unsigned _current_preview)
{
  assert(_current_preview < info_box_layout.count);

  if (_current_preview == current_preview)
    return;

  previews[current_preview].invalidate();
  current_preview = _current_preview;
  previews[current_preview].invalidate();

  DataFieldEnum &df = *(DataFieldEnum *)edit_select->GetDataField();
  df.Set(current_preview);
  edit_select->RefreshDisplay();

  RefreshEditContent();
}

static void
OnSelectAccess(DataField *Sender, DataField::DataAccessKind_t Mode)
{
  const DataFieldEnum &dfe = (const DataFieldEnum &)*Sender;

  SetCurrentInfoBox(dfe.GetAsInteger());
}

static void
OnContentAccess(DataField *Sender, DataField::DataAccessKind_t Mode)
{
  const DataFieldEnum &dfe = (const DataFieldEnum &)*Sender;

  data.contents[current_preview] = (InfoBoxFactory::t_InfoBox)dfe.GetAsInteger();
  previews[current_preview].invalidate();
}

bool
InfoBoxPreview::on_mouse_down(PixelScalar x, PixelScalar y)
{
  SetCurrentInfoBox(this - previews);
  return true;
}

bool
InfoBoxPreview::on_mouse_double(PixelScalar x, PixelScalar y)
{
  edit_content->BeginEditing();
  return true;
}

void
InfoBoxPreview::on_paint(Canvas &canvas)
{
  const unsigned i = this - previews;
  const bool is_current = i == current_preview;

  if (is_current)
    canvas.clear(COLOR_BLACK);
  else
    canvas.ClearWhite();

  canvas.SelectHollowBrush();
  canvas.SelectBlackPen();
  canvas.Rectangle(0, 0, canvas.get_width() - 1, canvas.get_height() - 1);

  InfoBoxFactory::t_InfoBox type = data.contents[i];
  const TCHAR *caption = type < InfoBoxFactory::NUM_TYPES
    ? InfoBoxFactory::GetCaption(type)
    : NULL;
  if (caption == NULL)
    caption = _("Invalid");
  else
    caption = gettext(caption);

  canvas.Select(Fonts::title);
  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(is_current ? COLOR_WHITE : COLOR_BLACK);
  canvas.text(2, 2, caption);
}

static void
OnContentHelp(WindowControl *Sender)
{
  WndProperty *wp = (WndProperty*)Sender;
  InfoBoxFactory::t_InfoBox type = (InfoBoxFactory::t_InfoBox)wp->GetDataField()->GetAsInteger();
  if (type >= InfoBoxFactory::NUM_TYPES)
    return;

  const TCHAR *name = InfoBoxFactory::GetName(type);
  if (name == NULL)
    return;

  TCHAR caption[100];
  _stprintf(caption, _T("%s: %s"), _("InfoBox"), gettext(name));

  const TCHAR *text = InfoBoxFactory::GetDescription(type);
  if (text == NULL)
    text = _("No help available on this item");
  else
    text = gettext(text);

  dlgHelpShowModal(wf->GetMainWindow(), caption, text);
}


static void
UpdatePanelName()
{
  const unsigned BUFFER_LENGTH = data.name.MAX_SIZE + 32;
  TCHAR caption[BUFFER_LENGTH];

  _sntprintf(caption, BUFFER_LENGTH, _T("%s: %s"), _("Name"),
             gettext(data.name));
  buttonPanelName->SetCaption(caption);
}


static void
OnNameAccess(WndButton &button)
{
  if (buttonPanelName) {
    TCHAR buffer[data.name.MAX_SIZE];
    _tcscpy(buffer, data.name);
    if (dlgTextEntryShowModal(*(SingleWindow *)button.get_root_owner(), buffer,
                              data.name.MAX_SIZE,
                              _("Panel name"))) {
      data.name = buffer;
      UpdatePanelName();
    }
  }
}

#ifdef _WIN32_WCE

static bool
OnKeyDown(WndForm &sender, unsigned key_code)
{
  DataFieldEnum *dfe;

  /* map the Altair hardware buttons */
  switch (key_code){
  case VK_UP:
    dfe = (DataFieldEnum *)edit_select->GetDataField();
    dfe->Dec();
    edit_select->RefreshDisplay();
    return true;

  case VK_DOWN:
    dfe = (DataFieldEnum *)edit_select->GetDataField();
    dfe->Inc();
    edit_select->RefreshDisplay();
    return true;

  case VK_LEFT:
    dfe = (DataFieldEnum *)edit_content->GetDataField();
    dfe->Dec();
    edit_content->RefreshDisplay();
    return true;

  case VK_RIGHT:
    dfe = (DataFieldEnum *)edit_content->GetDataField();
    dfe->Inc();
    edit_content->RefreshDisplay();
    return true;

  case VK_APP1:
    OnNameAccess(*buttonPaste);
    return true;

  case '6':
    sender.SetModalResult(mrOK);
    return true;

  case '7':
    OnCopy(*buttonPaste);
    return true;

  case '8':
    OnPaste(*buttonPaste);
    return true;

  default:
    return false;
  }
}

#endif

bool
dlgConfigInfoboxesShowModal(SingleWindow &parent,
                            const DialogLook &dialog_look,
                            InfoBoxLayout::Geometry geometry,
                            InfoBoxSettings::Panel &data_r,
                            bool allow_name_change)
{
  current_preview = 0;
  data = data_r;

  PixelRect rc = parent.get_client_rect();
  wf = new WndForm(parent, dialog_look, rc);

#ifdef _WIN32_WCE
  if (IsAltair())
    wf->SetKeyDownNotify(OnKeyDown);
#endif

  ContainerWindow &client_area = wf->GetClientAreaWindow();
  rc = client_area.get_client_rect();

  InflateRect(&rc, Layout::FastScale(-2), Layout::FastScale(-2));
  info_box_layout = InfoBoxLayout::Calculate(rc, geometry);

  WindowStyle preview_style;
  preview_style.enable_double_clicks();
  for (unsigned i = 0; i < info_box_layout.count; ++i) {
    rc = info_box_layout.positions[i];
    previews[i].set(client_area, rc.left, rc.top,
                    rc.right - rc.left, rc.bottom - rc.top,
                    preview_style);
  }

  rc = info_box_layout.remaining;

  WindowStyle style;
  style.control_parent();

  EditWindowStyle edit_style;
  edit_style.vertical_center();
  edit_style.tab_stop();

  if (IsEmbedded() || Layout::scale_1024 < 2048)
    /* sunken edge doesn't fit well on the tiny screen of an
       embedded device */
    edit_style.border();
  else
    edit_style.sunken_edge();

  PixelRect control_rc = rc;
  control_rc.right -= Layout::FastScale(2);

  const UPixelScalar height = Layout::Scale(22);
  const UPixelScalar caption_width = Layout::Scale(60);

  ButtonWindowStyle button_style;
  button_style.tab_stop();

  control_rc.bottom = control_rc.top + height;
  buttonPanelName =
    new WndButton(client_area, dialog_look, _T(""),
                  control_rc, button_style, OnNameAccess);
  buttonPanelName->set_enabled(allow_name_change);
  UpdatePanelName();

  control_rc.top = control_rc.bottom;
  control_rc.bottom = control_rc.top + height;

  edit_select = new WndProperty(client_area, dialog_look, _("InfoBox"),
                                rc, caption_width,
                                style, edit_style,
                                NULL);

  DataFieldEnum *dfe = new DataFieldEnum(OnSelectAccess);
  for (unsigned i = 0; i < info_box_layout.count; ++i) {
    TCHAR label[32];
    _stprintf(label, _T("%u"), i + 1);
    dfe->addEnumText(label, i);
  }

  edit_select->SetDataField(dfe);

  control_rc.top += height;
  control_rc.bottom += height;

  edit_content = new WndProperty(client_area, dialog_look, _("Content"),
                                 rc, caption_width,
                                 style, edit_style,
                                 NULL);

  dfe = new DataFieldEnum(OnContentAccess);
  for (unsigned i = InfoBoxFactory::MIN_TYPE_VAL; i < InfoBoxFactory::NUM_TYPES; i++) {
    const TCHAR *name = InfoBoxFactory::GetName((InfoBoxFactory::t_InfoBox) i);
    if (name != NULL)
      dfe->addEnumText(gettext(name), i);
  }

  dfe->Sort(0);

  edit_content->SetDataField(dfe);
  edit_content->SetOnHelpCallback(OnContentHelp);

  RefreshEditContent();

  const UPixelScalar button_width = Layout::Scale(60);
  const UPixelScalar button_height = Layout::Scale(28);

  PixelRect button_rc = rc;
  button_rc.right = button_rc.left + button_width;
  button_rc.top = button_rc.bottom - button_height;

  WndButton *close_button =
    new WndButton(client_area, dialog_look, _("Close"),
                  button_rc, button_style, OnCloseClicked);

  button_rc.left += button_width + Layout::Scale(2);
  button_rc.right += button_width + Layout::Scale(2);
  WndButton *copy_button =
    new WndButton(client_area, dialog_look, _("Copy"),
                  button_rc, button_style, OnCopy);

  button_rc.left += button_width + Layout::Scale(2);
  button_rc.right += button_width + Layout::Scale(2);
  buttonPaste =
    new WndButton(client_area, dialog_look, _("Paste"),
                  button_rc, button_style, OnPaste);

  RefreshPasteButton();

  int result = wf->ShowModal();

  delete wf;
  delete buttonPanelName;
  delete edit_select;
  delete edit_content;
  delete close_button;
  delete copy_button;
  delete buttonPaste;

  bool changed = false;
  if (result == mrOK) {
    for (unsigned i = 0; i < InfoBoxSettings::Panel::MAX_CONTENTS; ++i)
      if (data.contents[i] != data_r.contents[i])
        changed = true;
    changed |= (_tcscmp(data.name, data_r.name) != 0);

    if (changed)
      data_r = data;
  }

  return changed;
}
