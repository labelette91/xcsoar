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

#include "TargetMapWindow.hpp"
#include "Look/TaskLook.hpp"
#include "Screen/Icon.hpp"
#include "Interface.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Screen/Layout.hpp"

void
TargetMapWindow::TargetPaintDrag(Canvas &canvas, const RasterPoint drag_last)
{
  task_look.target_icon.Draw(canvas, drag_last.x, drag_last.y);
}

bool
TargetMapWindow::TargetDragged(const int x, const int y)
{
  assert(task != NULL);

  GeoPoint gp = projection.ScreenToGeo(x, y);
  ProtectedTaskManager::ExclusiveLease task_manager(*task);
  if (!task_manager->TargetIsLocked(target_index))
    task_manager->TargetLock(target_index, true);

  task_manager->SetTarget(target_index, gp, true);
  return true;
}

bool
TargetMapWindow::isClickOnTarget(const RasterPoint pc)
{
  if (task == NULL)
    return false;

  ProtectedTaskManager::Lease task_manager(*task);
  if (!task_manager->HasTarget(target_index))
    return false;

  const GeoPoint gnull(Angle::Zero(), Angle::Zero());
  const GeoPoint& t = task_manager->GetLocationTarget(target_index, gnull);

  if (t == gnull)
    return false;

  const GeoPoint gp = projection.ScreenToGeo(pc.x, pc.y);
  if (projection.GeoToScreenDistance(gp.Distance(t)) <
      unsigned(Layout::Scale(10)))
    return true;

  return false;
}

bool
TargetMapWindow::isInSector(const int x, const int y)
{
  assert(task != NULL);

  GeoPoint gp = projection.ScreenToGeo(x, y);
  AircraftState a;
  a.location = gp;
  return task->IsInSector(target_index, a);
}
