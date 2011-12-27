/* Copyright_License {

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

#include "Task/MapTaskManager.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "Engine/Task/TaskEvents.hpp"
#include "Dialogs/Internal.hpp"
#include "Protection.hpp"

static const TaskBehaviour&
GetTaskBehaviour()
{
  return XCSoarInterface::GetComputerSettings().task;
}

static MapTaskManager::TaskEditResult
AppendToTask(OrderedTask *task, const Waypoint &waypoint)
{
  if (task->TaskSize()==0)
    return MapTaskManager::NOTASK;

  int i = task->TaskSize() - 1;
  // skip all finish points
  while (i >= 0) {
    const OrderedTaskPoint *tp = task->get_tp(i);
    if (tp == NULL)
      break;

    if (tp->successor_allowed()) {
      ++i;
      break;
    }

    --i;
  }

  const AbstractTaskFactory &factory = task->GetFactory();
  OrderedTaskPoint *task_point =
      (OrderedTaskPoint *)factory.createIntermediate(waypoint);
  if (task_point == NULL)
    return MapTaskManager::UNMODIFIED;

  bool success = i >= 0 ? task->Insert(*task_point, i) : task->Append(*task_point);
  delete task_point;

  if (!success)
    return MapTaskManager::UNMODIFIED;

  if (!task->CheckTask())
    return MapTaskManager::INVALID;

  return MapTaskManager::SUCCESS;
}

static MapTaskManager::TaskEditResult
MutateFromGoto(OrderedTask *task, const Waypoint &finish_waypoint,
               const Waypoint &start_waypoint)
{
  const AbstractTaskFactory &factory = task->GetFactory();
  OrderedTaskPoint *start_point =
      (OrderedTaskPoint *)factory.createStart(start_waypoint);
  if (start_point == NULL)
    return MapTaskManager::UNMODIFIED;

  bool success = task->Append(*start_point);
  delete start_point;
  if (!success)
    return MapTaskManager::UNMODIFIED;

  OrderedTaskPoint *finish_point =
      (OrderedTaskPoint *)factory.createFinish(finish_waypoint);
  if (finish_point == NULL)
    return MapTaskManager::UNMODIFIED;

  success = task->Append(*finish_point);
  delete finish_point;

  if (!success)
    return MapTaskManager::UNMODIFIED;

  return MapTaskManager::MUTATED_FROM_GOTO;
}

MapTaskManager::TaskEditResult
MapTaskManager::AppendToTask(const Waypoint &waypoint)
{
  assert(protected_task_manager != NULL);
  TaskEvents task_events;
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  TaskEditResult result = MapTaskManager::UNMODIFIED;
  if (task_manager->GetOrderedTask().CheckTask()) {
    OrderedTask *task = task_manager->Clone(task_events,
                                            GetTaskBehaviour(),
                                            task_manager->GetGlidePolar());
    result = AppendToTask(task, waypoint);
    if (result == SUCCESS)
      task_manager->Commit(*task);
    delete task;
  } else { // ordered task invalid
    switch (task_manager->GetMode()) {
    case TaskManager::MODE_NULL:
    case TaskManager::MODE_ABORT:
    case TaskManager::MODE_ORDERED:
      result = task_manager->DoGoto(waypoint) ? MapTaskManager::MUTATED_TO_GOTO :
                              MapTaskManager::UNMODIFIED;
      break;
    case TaskManager::MODE_GOTO:
    {
      OrderedTask *task = task_manager->Clone(task_events,
                                              GetTaskBehaviour(),
                                              task_manager->GetGlidePolar());
      const TaskWaypoint *OldGotoTWP = task_manager->GetActiveTaskPoint();
      if (!OldGotoTWP)
        break;

      const Waypoint &OldGotoWp = OldGotoTWP->GetWaypoint();
      result = MutateFromGoto(task, waypoint, OldGotoWp);
      if (result == MUTATED_FROM_GOTO)
        task_manager->Commit(*task);

      delete task;
      break;
    }
    default:
      break;
    }
  }
  return result;
}

static MapTaskManager::TaskEditResult
InsertInTask(OrderedTask *task, const Waypoint &waypoint)
{
  if (task->TaskSize()==0)
    return MapTaskManager::NOTASK;

  int i = task->GetActiveIndex();
  /* skip all start points */
  while (true) {
    if (i >= (int)task->TaskSize())
      return MapTaskManager::UNMODIFIED;

    const OrderedTaskPoint *task_point = task->get_tp(i);
    if (task_point == NULL || task_point->predecessor_allowed())
      break;

    ++i;
  }

  const AbstractTaskFactory &factory = task->GetFactory();
  OrderedTaskPoint *task_point =
      (OrderedTaskPoint *)factory.createIntermediate(waypoint);
  if (task_point == NULL)
    return MapTaskManager::UNMODIFIED;

  bool success = task->Insert(*task_point, i);
  delete task_point;
  if (!success)
    return MapTaskManager::UNMODIFIED;
  if (!task->CheckTask())
    return MapTaskManager::INVALID;
  return MapTaskManager::SUCCESS;
}

MapTaskManager::TaskEditResult
MapTaskManager::InsertInTask(const Waypoint &waypoint)
{
  assert(protected_task_manager != NULL);
  TaskEvents task_events;
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  TaskEditResult result = MapTaskManager::UNMODIFIED;
  if (task_manager->GetOrderedTask().CheckTask()) {
    OrderedTask *task = task_manager->Clone(task_events,
                                            GetTaskBehaviour(),
                                            task_manager->GetGlidePolar());

    result = InsertInTask(task, waypoint);
    if (result == SUCCESS)
      task_manager->Commit(*task);
    delete task;
  } else { // ordered task invalid
    switch (task_manager->GetMode()) {
    case TaskManager::MODE_NULL:
    case TaskManager::MODE_ABORT:
    case TaskManager::MODE_ORDERED:
      result = task_manager->DoGoto(waypoint) ? MapTaskManager::MUTATED_TO_GOTO :
                              MapTaskManager::UNMODIFIED;
      break;
    case TaskManager::MODE_GOTO:
    {
      OrderedTask *task = task_manager->Clone(task_events,
                                              GetTaskBehaviour(),
                                              task_manager->GetGlidePolar());
      const TaskWaypoint *OldGotoTWP = task_manager->GetActiveTaskPoint();
      if (!OldGotoTWP)
        break;
      const Waypoint &OldGotoWp = OldGotoTWP->GetWaypoint();
      result = MutateFromGoto(task, OldGotoWp, waypoint);
      if (result == MUTATED_FROM_GOTO)
        task_manager->Commit(*task);
      delete task;
      break;
    }
    default:
      break;
    }
  }
  return result;
}

static MapTaskManager::TaskEditResult
ReplaceInTask(OrderedTask *task, const Waypoint &waypoint)
{
  if (task->TaskSize()==0)
    return MapTaskManager::NOTASK;

  unsigned i = task->GetActiveIndex();
  if (i >= task->TaskSize())
    return MapTaskManager::UNMODIFIED;

  task->Relocate(i, waypoint);

  if (!task->CheckTask())
    return MapTaskManager::INVALID;

  return MapTaskManager::SUCCESS;
}

MapTaskManager::TaskEditResult
MapTaskManager::ReplaceInTask(const Waypoint &waypoint)
{
  assert(protected_task_manager != NULL);
  TaskEvents task_events;
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  OrderedTask *task = task_manager->Clone(task_events,
                                          GetTaskBehaviour(),
                                          task_manager->GetGlidePolar());

  TaskEditResult result = ReplaceInTask(task, waypoint);
  if (result == SUCCESS)
    task_manager->Commit(*task);

  delete task;
  return result;
}

static int
GetIndexInTask(const OrderedTask &task, const Waypoint &waypoint)
{
  if (task.TaskSize() == 0)
    return -1;

  unsigned i = task.GetActiveIndex();
  if (i >= task.TaskSize())
    return -1;

  int TPindex = -1;
  for (unsigned i = task.TaskSize(); i--;) {
    const OrderedTaskPoint &tp = task.GetPoint(i);

    if (tp.GetWaypoint() == waypoint) {
      TPindex = i;
      break;
    }
  }
  return TPindex;
}

int
MapTaskManager::GetIndexInTask(const Waypoint &waypoint)
{
  assert(protected_task_manager != NULL);
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  if (task_manager->GetMode() == TaskManager::MODE_ORDERED) {
    const OrderedTask &task = task_manager->GetOrderedTask();
    return GetIndexInTask(task, waypoint);
  }
  return -1;
}

static MapTaskManager::TaskEditResult
RemoveFromTask(OrderedTask *task, const Waypoint &waypoint)
{
  if (task->TaskSize()==0)
    return MapTaskManager::NOTASK;

  int i = GetIndexInTask(*task, waypoint);
  if (i >= 0)
    task->GetFactory().remove(i);

  // if finish was removed
  if (i == (int)task->TaskSize())
    task->GetFactory().CheckAddFinish();

  if (i == -1)
    return MapTaskManager::UNMODIFIED;

  if (!task->CheckTask())
    return MapTaskManager::INVALID;

  return MapTaskManager::SUCCESS;
}

MapTaskManager::TaskEditResult
MapTaskManager::RemoveFromTask(const Waypoint &wp)
{
  assert(protected_task_manager != NULL);
  TaskEvents task_events;
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  OrderedTask *task = task_manager->Clone(task_events,
                                          GetTaskBehaviour(),
                                          task_manager->GetGlidePolar());

  TaskEditResult result = RemoveFromTask(task, wp);
  if (result == SUCCESS)
    task_manager->Commit(*task);

  delete task;
  return result;
}
