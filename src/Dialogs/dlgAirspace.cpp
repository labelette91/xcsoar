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

#include "Dialogs/Airspace.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/Internal.hpp"
#include "Dialogs/Dialogs.h"
#include "Profile/Profile.hpp"
#include "Profile/AirspaceConfig.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Screen/Layout.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Airspace/ProtectedAirspaceWarningManager.hpp"
#include "Airspace/AirspaceClass.hpp"
#include "Engine/Airspace/AirspaceWarningManager.hpp"
#include "Components.hpp"
#include "Computer/GlideComputer.hpp"

#include <assert.h>

static WndForm *wf = NULL;
static WndListFrame *wAirspaceList = NULL;

static bool colormode = false;

static void
OnAirspacePaintListItem(Canvas &canvas, const PixelRect rc, unsigned i)
{
  assert(i < AIRSPACECLASSCOUNT);

  const AirspaceComputerSettings &computer =
    CommonInterface::GetComputerSettings().airspace;
  const AirspaceRendererSettings &renderer =
    CommonInterface::GetMapSettings().airspace;
  const AirspaceLook &look = CommonInterface::main_window.GetLook().map.airspace;

  PixelScalar w0 = rc.right - rc.left - Layout::FastScale(4);

  PixelScalar w1 = canvas.CalcTextWidth(_("Warn")) + Layout::FastScale(10);
  PixelScalar w2 = canvas.CalcTextWidth(_("Display")) + Layout::FastScale(10);
  PixelScalar x0 = w0 - w1 - w2;

  if (colormode) {
    canvas.SelectWhitePen();
#ifndef HAVE_HATCHED_BRUSH
    canvas.Select(look.solid_brushes[renderer.colours[i]]);
#else
#ifdef HAVE_ALPHA_BLEND
    if (renderer.transparency && AlphaBlendAvailable()) {
      canvas.Select(look.solid_brushes[renderer.colours[i]]);
    } else {
#endif
      canvas.SetTextColor(look.colors[renderer.colours[i]]);
      canvas.SetBackgroundColor(Color(0xFF, 0xFF, 0xFF));
      canvas.Select(look.brushes[renderer.brushes[i]]);
#ifdef HAVE_ALPHA_BLEND
    }
#endif
#endif
    canvas.Rectangle(rc.left + x0, rc.top + Layout::FastScale(2),
        rc.right - Layout::FastScale(2), rc.bottom - Layout::FastScale(2));
  } else {
    if (computer.warnings.class_warnings[i])
      canvas.text(rc.left + w0 - w1 - w2, rc.top + Layout::FastScale(2),
                  _("Warn"));

    if (renderer.display[i])
      canvas.text(rc.left + w0 - w2, rc.top + Layout::FastScale(2),
                  _("Display"));
  }

  canvas.text_clipped(rc.left + Layout::FastScale(2),
      rc.top + Layout::FastScale(2), x0 - Layout::FastScale(10),
                      AirspaceClassAsText((AirspaceClass)i, false));
}

static bool changed = false;

static void
OnAirspaceListEnter(unsigned ItemIndex)
{
  assert(ItemIndex < AIRSPACECLASSCOUNT);

  AirspaceComputerSettings &computer =
    CommonInterface::SetComputerSettings().airspace;
  AirspaceRendererSettings &renderer =
    CommonInterface::SetMapSettings().airspace;

  if (colormode) {
    int c = dlgAirspaceColoursShowModal();
    if (c >= 0) {
      renderer.colours[ItemIndex] = c;
      ActionInterface::SendMapSettings();
      Profile::SetAirspaceColor(ItemIndex, renderer.colours[ItemIndex]);
      changed = true;

      AirspaceLook &look =
        CommonInterface::main_window.SetLook().map.airspace;
      look.Initialise(renderer);
    }

#ifdef HAVE_HATCHED_BRUSH
#ifdef HAVE_ALPHA_BLEND
    if (!renderer.transparency || !AlphaBlendAvailable()) {
#endif
      int p = dlgAirspacePatternsShowModal();
      if (p >= 0) {
        renderer.brushes[ItemIndex] = p;
        ActionInterface::SendMapSettings();
        Profile::SetAirspaceBrush(ItemIndex, renderer.brushes[ItemIndex]);
        changed = true;
      }
#ifdef HAVE_ALPHA_BLEND
    }
#endif
#endif
  } else {
    renderer.display[ItemIndex] = !renderer.display[ItemIndex];
    if (!renderer.display[ItemIndex])
      computer.warnings.class_warnings[ItemIndex] =
        !computer.warnings.class_warnings[ItemIndex];

    Profile::SetAirspaceMode(ItemIndex);
    changed = true;
  }

  wAirspaceList->invalidate();

  ActionInterface::SendMapSettings();
}

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static void
OnLookupClicked(gcc_unused WndButton &Sender)
{
  dlgAirspaceSelect(airspace_database, GetAirspaceWarnings());
}

static gcc_constexpr_data CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnLookupClicked),
  DeclareCallBackEntry(NULL)
};

void
dlgAirspaceShowModal(bool coloredit)
{
  colormode = coloredit;

  wf = LoadDialog(CallBackTable, XCSoarInterface::main_window,
                  !Layout::landscape ? _T("IDR_XML_AIRSPACE_L") :
                                       _T("IDR_XML_AIRSPACE"));
  assert(wf != NULL);

  wAirspaceList = (WndListFrame*)wf->FindByName(_T("frmAirspaceList"));
  assert(wAirspaceList != NULL);
  wAirspaceList->SetActivateCallback(OnAirspaceListEnter);
  wAirspaceList->SetPaintItemCallback(OnAirspacePaintListItem);
  wAirspaceList->SetLength(AIRSPACECLASSCOUNT);

  changed = false;

  wf->ShowModal();

  // now retrieve back the properties...
  if (changed) {
    if (!colormode && glide_computer != NULL) {
      ProtectedAirspaceWarningManager::ExclusiveLease awm(glide_computer->GetAirspaceWarnings());
      awm->SetConfig(CommonInterface::SetComputerSettings().airspace.warnings);
    }

    Profile::Save();
  }

  delete wf;
}
