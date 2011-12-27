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
#include "Airspaces.hpp"
#include "AirspaceVisitor.hpp"
#include "AirspaceIntersectionVisitor.hpp"
#include "Atmosphere/Pressure.hpp"
#include "Navigation/Aircraft.hpp"
#include "Navigation/Geometry/GeoVector.hpp"

#ifdef INSTRUMENT_TASK
extern unsigned n_queries;
extern long count_intersections;
#endif

class AirspacePredicateVisitorAdapter {
  const AirspacePredicate *predicate;
  AirspaceVisitor *visitor;

public:
  AirspacePredicateVisitorAdapter(const AirspacePredicate &_predicate,
                                  AirspaceVisitor &_visitor)
    :predicate(&_predicate), visitor(&_visitor) {}

  void operator()(Airspace as) {
    AbstractAirspace &aas = *as.get_airspace();
    if (predicate->condition(aas))
      visitor->Visit(as);
  }
};

void 
Airspaces::visit_within_range(const GeoPoint &loc, 
                              const fixed range,
                              AirspaceVisitor& visitor,
                              const AirspacePredicate &predicate) const
{
  if (empty()) return; // nothing to do

  Airspace bb_target(loc, task_projection);
  int mrange = task_projection.project_range(loc, range);
  AirspacePredicateVisitorAdapter adapter(predicate, visitor);
  airspace_tree.visit_within_range(bb_target, -mrange, adapter);

#ifdef INSTRUMENT_TASK
  n_queries++;
#endif
}

class IntersectingAirspaceVisitorAdapter {
  GeoPoint start, end;
  const FlatRay *ray;
  AirspaceIntersectionVisitor *visitor;

public:
  IntersectingAirspaceVisitorAdapter(const GeoPoint &_loc,
                                     const GeoPoint &_end,
                                     const FlatRay &_ray,
                                     AirspaceIntersectionVisitor &_visitor)
    :start(_loc), end(_end), ray(&_ray), visitor(&_visitor) {}

  void operator()(Airspace as) {
    if (as.intersects(*ray) &&
        visitor->set_intersections(as.Intersects(start, end)))
      visitor->Visit(as);
  }
};

void 
Airspaces::VisitIntersecting(const GeoPoint &loc, const GeoPoint &end,
                             AirspaceIntersectionVisitor& visitor) const
{
  if (empty()) return; // nothing to do

  FlatRay ray(task_projection.project(loc), task_projection.project(end));

  const GeoPoint c = loc.Middle(end);
  Airspace bb_target(c, task_projection);
  int mrange = task_projection.project_range(c, loc.Distance(end) / 2);
  IntersectingAirspaceVisitorAdapter adapter(loc, end, ray, visitor);
  airspace_tree.visit_within_range(bb_target, -mrange, adapter);

#ifdef INSTRUMENT_TASK
  n_queries++;
#endif
}

// SCAN METHODS

struct AirspacePredicateAdapter {
  const AirspacePredicate &condition;

  AirspacePredicateAdapter(const AirspacePredicate &_condition)
    :condition(_condition) {}

  bool operator()(const Airspace &as) const {
    return condition(*as.get_airspace());
  }
};

const Airspace *
Airspaces::find_nearest(const GeoPoint &location,
                        const AirspacePredicate &condition) const
{
  if (empty())
    return NULL;

  const Airspace bb_target(location, task_projection);
  const int mrange = task_projection.project_range(location, fixed(30000));
  const AirspacePredicateAdapter predicate(condition);
  std::pair<AirspaceTree::const_iterator, AirspaceTree::distance_type> found =
    airspace_tree.find_nearest_if(bb_target, BBDist(0, mrange), predicate);

  return found.first != airspace_tree.end()
    ? &*found.first
    : NULL;
}

const Airspaces::AirspaceVector
Airspaces::scan_nearest(const GeoPoint &location,
                        const AirspacePredicate &condition) const 
{
  if (empty()) return AirspaceVector(); // nothing to do

  Airspace bb_target(location, task_projection);

  std::pair<AirspaceTree::const_iterator, AirspaceTree::distance_type>
    found = airspace_tree.find_nearest(bb_target);

#ifdef INSTRUMENT_TASK
  n_queries++;
#endif

  AirspaceVector res;
  if (found.first != airspace_tree.end()) {
    // also should do scan_range with range = 0 since there
    // could be more than one with zero dist
    if (found.second.is_zero()) {
      return scan_range(location, fixed_zero, condition);
    } else {
      if (condition(*found.first->get_airspace()))
        res.push_back(*found.first);
    }
  }

  return res;
}

const Airspaces::AirspaceVector
Airspaces::scan_range(const GeoPoint &location,
                      const fixed range,
                      const AirspacePredicate &condition) const
{
  if (empty()) return AirspaceVector(); // nothing to do

  Airspace bb_target(location, task_projection);
  int mrange = task_projection.project_range(location, range);
  
  std::deque< Airspace > vectors;
  airspace_tree.find_within_range(bb_target, -mrange, std::back_inserter(vectors));

#ifdef INSTRUMENT_TASK
  n_queries++;
#endif

  AirspaceVector res;

  for (auto v = vectors.begin(); v != vectors.end(); ++v) {
    if (!condition(*v->get_airspace()))
      continue;

    if (fixed((*v).Distance(bb_target)) > range)
      continue;

    if ((*v).inside(location) || positive(range))
      res.push_back(*v);
  }

  return res;
}

const Airspaces::AirspaceVector
Airspaces::find_inside(const AircraftState &state,
                       const AirspacePredicate &condition) const
{
  Airspace bb_target(state.location, task_projection);

  AirspaceVector vectors;
  airspace_tree.find_within_range(bb_target, 0, std::back_inserter(vectors));

#ifdef INSTRUMENT_TASK
  n_queries++;
#endif

  for (auto v = vectors.begin(); v != vectors.end();) {

#ifdef INSTRUMENT_TASK
    count_intersections++;
#endif
    
    if (!condition(*v->get_airspace()) || !(*v).inside(state))
      vectors.erase(v);
    else
      ++v;
  }

  return vectors;
}

void 
Airspaces::optimise()
{
  if (!m_owner || task_projection.update_fast()) {
    // dont update task_projection if not owner!

    // task projection changed, so need to push items back onto stack
    // to re-build airspace envelopes

    for (auto it = airspace_tree.begin(); it != airspace_tree.end(); ++it)
      tmp_as.push_back(it->get_airspace());

    airspace_tree.clear();
  }

  if (!tmp_as.empty()) {
    while (!tmp_as.empty()) {
      Airspace as(*tmp_as.front(), task_projection);
      airspace_tree.insert(as);
      tmp_as.pop_front();
    }
    airspace_tree.optimise();
  }
}

void 
Airspaces::insert(AbstractAirspace* asp)
{
  if (!asp)
    // nothing to add
    return;

  // reset QNH to zero so set_pressure_levels will be triggered next update
  // this allows for airspaces to be add at any time
  m_QNH = fixed_zero;

  // reset day to all so set_activity will be triggered next update
  // this allows for airspaces to be add at any time
  m_day.set_all();

  if (m_owner) {
    if (empty())
      task_projection.reset(asp->GetCenter());

    task_projection.scan_location(asp->GetCenter());
  }

  tmp_as.push_back(asp);
}

void
Airspaces::clear()
{
  // delete temporaries in case they were added without an optimise() call
  while (!tmp_as.empty()) {
    if (m_owner) {
      AbstractAirspace *aa = tmp_as.front();
      delete aa;
    }
    tmp_as.pop_front();
  }

  // delete items in the tree
  if (m_owner) {
    for (auto v = airspace_tree.begin(); v != airspace_tree.end(); ++v) {
      Airspace a = *v;
      a.destroy();
    }
  }

  // then delete the tree
  airspace_tree.clear();
}

unsigned
Airspaces::size() const
{
  return airspace_tree.size();
}

bool
Airspaces::empty() const
{
  return airspace_tree.empty() && tmp_as.empty();
}

Airspaces::~Airspaces()
{
  clear();
}

void 
Airspaces::set_flight_levels(const AtmosphericPressure &press)
{
  if (press.GetHectoPascal() != m_QNH) {
    m_QNH = press.GetHectoPascal();

    for (auto v = airspace_tree.begin(); v != airspace_tree.end(); ++v)
      v->set_flight_level(press);
  }
}

void
Airspaces::set_activity(const AirspaceActivity mask)
{
  if (!mask.equals(m_day)) {
    m_day = mask;

    for (auto v = airspace_tree.begin(); v != airspace_tree.end(); ++v)
      v->set_activity(mask);
  }
}

Airspaces::AirspaceTree::const_iterator
Airspaces::begin() const
{
  return airspace_tree.begin();
}

Airspaces::AirspaceTree::const_iterator
Airspaces::end() const
{
  return airspace_tree.end();
}

Airspaces::Airspaces(const Airspaces& master,
  bool owner):
  m_QNH(master.m_QNH),
  m_day(master.m_day),
  m_owner(owner),
  task_projection(master.task_projection)
{
}

void
Airspaces::clear_clearances()
{
  for (auto v = airspace_tree.begin(); v != airspace_tree.end(); ++v)
    v->clear_clearance();
}


bool
Airspaces::synchronise_in_range(const Airspaces& master,
                                const GeoPoint &location,
                                const fixed range,
                                const AirspacePredicate &condition)
{
  bool changed = false;
  const AirspaceVector contents_master = master.scan_range(location, range, condition);
  AirspaceVector contents_self;
  contents_self.reserve(max(airspace_tree.size(), contents_master.size()));

  task_projection = master.task_projection; // ensure these are up to date

  for (auto t = airspace_tree.begin(); t != airspace_tree.end(); ++t)
    contents_self.push_back(*t);

  // find items to add
  for (auto v = contents_master.begin(); v != contents_master.end(); ++v) {
    const AbstractAirspace* other = v->get_airspace();

    bool found = false;
    for (auto s = contents_self.begin(); s != contents_self.end(); ++s) {
      const AbstractAirspace* self = s->get_airspace();
      if (self == other) {
        found = true;
        contents_self.erase(s);
        break;
      }
    }
    if (!found && other->IsActive()) {
      insert(v->get_airspace());
      changed = true;
    }
  }
  // anything left in the self list are items that were not in the query,
  // so delete them --- including the clearances!
  for (auto v = contents_self.begin(); v != contents_self.end();) {
    bool found = false;
    for (auto t = airspace_tree.begin(); t != airspace_tree.end(); ) {
      if (t->get_airspace() == v->get_airspace()) {
        AirspaceTree::const_iterator new_t = t;
        ++new_t;
        airspace_tree.erase_exact(*t);
        t = new_t;
        found = true;
      } else {
        ++t;
      }
    }
    assert(found);
    v->clear_clearance();
    v = contents_self.erase(v);
    changed = true;
  }
  if (changed)
    optimise();
  return changed;
}

void
Airspaces::visit_inside(const GeoPoint &loc,
                        AirspaceVisitor& visitor) const
{
  if (empty()) return; // nothing to do

  Airspace bb_target(loc, task_projection);
  AirspaceVector vectors;
  airspace_tree.find_within_range(bb_target, 0, std::back_inserter(vectors));

  for (auto v = vectors.begin(); v != vectors.end(); ++v) {
    if ((*v).inside(loc))
      visitor.Visit(*v);
  }
}

