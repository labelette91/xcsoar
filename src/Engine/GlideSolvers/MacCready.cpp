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

#include "MacCready.hpp"
#include <assert.h>
#include <algorithm>
#include "GlideState.hpp"
#include "GlidePolar.hpp"
#include "GlideResult.hpp"
#include "Navigation/Aircraft.hpp"
#include "Util/ZeroFinder.hpp"
#include "Util/Tolerances.hpp"

#define fixed_1mil fixed_int_constant(1000000)

#ifdef INSTRUMENT_TASK
// global, used for test harness
long count_mc = 0;
#endif

MacCready::MacCready(const GlidePolar &_glide_polar,
                     const fixed _cruise_efficiency)
  :glide_polar(_glide_polar), cruise_efficiency(_cruise_efficiency) {}

GlideResult 
MacCready::SolveVertical(const GlideState &task) const
{
  GlideResult result(task, glide_polar.GetVBestLD());

  // distance relation
  //   V*t_cr = W*(t_cl+t_cr)
  //     t_cr*(V-W)=W*t_cl
  //     t_cr = (W*t_cl)/(V-W)     .... (1)

  // height relation
  //   t_cl = (-dh+t_cr*S)/mc
  //     t_cl*mc = (-dh+(W*t_cl)/(V-W))    substitute (1)
  //     t_cl*mc*(V-W)= -dh*(V-W)+W*t_cl
  //     t_cl*(mc*(V-W)-W) = -dh*(V-W) .... (2)

  if (!negative(task.altitude_difference)) {
    // immediate solution
    result.height_climb = fixed_zero;
    result.height_glide = fixed_zero;
    result.time_elapsed = fixed_zero;
    result.validity = GlideResult::Validity::RESULT_OK;
    return result;
  }
  
  const fixed v = glide_polar.GetVBestLD() * cruise_efficiency;
  const fixed denom1 = v - task.wind.norm;

  if (!positive(denom1)) {
    result.validity = GlideResult::Validity::RESULT_WIND_EXCESSIVE;
    return result;
  }
  const fixed denom2 = glide_polar.GetMC() * denom1 - task.wind.norm;
  if (!positive(denom2)) {
    result.validity = GlideResult::Validity::RESULT_MACCREADY_INSUFFICIENT;
    return result;
  }

  // from (2)
  const fixed time_climb = -task.altitude_difference * denom1 / denom2;
  // from (1)
  const fixed time_cruise = task.wind.norm * time_climb / denom1; 

  result.time_elapsed = time_cruise + time_climb;
  result.height_climb = -task.altitude_difference;
  result.height_glide = fixed_zero;
  result.validity = GlideResult::Validity::RESULT_OK;

  return result;
}

GlideResult
MacCready::Solve(const GlidePolar &glide_polar, const GlideState &task)
{
#ifdef INSTRUMENT_TASK
  count_mc++;
#endif
  MacCready mac(glide_polar, glide_polar.GetCruiseEfficiency());
  return mac.Solve(task);
}

GlideResult
MacCready::SolveSink(const GlidePolar &glide_polar, const GlideState &task,
                      const fixed sink_rate)
{
#ifdef INSTRUMENT_TASK
  count_mc++;
#endif
  MacCready mac(glide_polar, glide_polar.GetCruiseEfficiency());
  return mac.SolveSink(task, sink_rate);
}

GlideResult
MacCready::SolveCruise(const GlideState &task) const
{
  // cruise speed for current MC (m/s)
  const fixed mc_speed = glide_polar.GetVBestLD();

  GlideResult result(task, mc_speed);

  // sink rate at current MC speed (m/s)
  const fixed mc_sink_rate = glide_polar.GetSBestLD();

  // MC value (m/s)
  const fixed mc = glide_polar.GetMC();

  // Inverse MC value (s/m)
  const fixed inv_mc = glide_polar.GetInvMC();

  /*
      |      rho = S / MC
      *..                       cruise speed
   MC |  ´--..                 /
    --+-------´´*-..-----------+------------
      |        /    ´´--..     | S
      |       /   ..---...´´-..|
      |      /  .´        ´´´--*.
      |     /                    ´´-.
      |    /                         ´.
      |   /
      |  resulting speed
  */

  // Sink rate divided by MC value
  // same as (cruise speed - resulting speed) / resulting speed
  const fixed rho = mc_sink_rate * inv_mc;

  // quotient of cruise speed over resulting speed (> 1.0)
  const fixed rho_plus_one = fixed_one + rho;

  // quotient of resulting speed over cruise speed (0 .. 1)
  const fixed inv_rho_plus_one = fixed_one / rho_plus_one;

  const fixed estimated_speed =
      task.CalcAverageSpeed(mc_speed * cruise_efficiency * inv_rho_plus_one);
  if (!positive(estimated_speed)) {
    result.validity = GlideResult::Validity::RESULT_WIND_EXCESSIVE;
    result.vector.distance = fixed_zero;
    return result;
  }

  fixed time_climb_drift = fixed_zero;
  fixed distance_with_climb_drift = task.vector.distance;

  // Calculate additional distance_with_climb_drift/time due to wind drift while circling
  if (negative(task.altitude_difference)) {
    time_climb_drift = -task.altitude_difference * inv_mc;
    distance_with_climb_drift = task.DriftedDistance(time_climb_drift);
  }

  // Estimated time to finish the task
  const fixed estimated_time = distance_with_climb_drift / estimated_speed;
  // Estimated time in cruise
  const fixed time_cruise = estimated_time * inv_rho_plus_one;
  // Estimated time in climb (including wind drift while circling)
  const fixed time_climb = time_cruise * rho + time_climb_drift;

  const fixed sink_glide = time_cruise * mc_sink_rate;

  result.time_elapsed = estimated_time;
  result.height_climb = time_climb * mc;
  result.height_glide = sink_glide;
  result.altitude_difference -= sink_glide;
  result.effective_wind_speed *= rho_plus_one;

  result.validity = GlideResult::Validity::RESULT_OK;

  return result;
}

GlideResult
MacCready::SolveGlide(const GlideState &task, const fixed v_set,
                      const fixed sink_rate, const bool allow_partial) const
{
  // spend a lot of time in this function, so it should be quick!

  GlideResult result(task, v_set);

  // distance relation
  //   V*V=Vn*Vn+W*W-2*Vn*W*cos(theta)
  //     Vn*Vn-2*Vn*W*cos(theta)+W*W-V*V=0  ... (1)

  const fixed estimated_speed = task.CalcAverageSpeed(v_set * cruise_efficiency);
  if (!positive(estimated_speed)) {
    result.validity = GlideResult::Validity::RESULT_WIND_EXCESSIVE;
    result.vector.distance = fixed_zero;
    return result;
  }

  result.validity = GlideResult::Validity::RESULT_OK;

  if (allow_partial) {
    const fixed Vndh = estimated_speed * task.altitude_difference;

    // S/Vn > dh/task.Distance
    if (sink_rate * task.vector.distance > Vndh) {
      if (negative(task.altitude_difference))
        // insufficient height, and can't climb
        result.vector.distance = fixed_zero;
      else
        // frac*task.Distance;
        result.vector.distance = Vndh / sink_rate;
    }
  }

  const fixed time_cruise = result.vector.distance / estimated_speed;
  result.time_elapsed = time_cruise;
  result.height_climb = fixed_zero;
  result.height_glide = time_cruise * sink_rate;
  result.altitude_difference -= result.height_glide;
  result.distance_to_final = fixed_zero;

  return result;
}

GlideResult
MacCready::SolveGlide(const GlideState &task, const fixed v_set,
                       const bool allow_partial) const
{
  const fixed sink_rate = glide_polar.SinkRate(v_set);
  return SolveGlide(task, v_set, sink_rate, allow_partial);
}

GlideResult
MacCready::SolveSink(const GlideState &task, const fixed sink_rate) const
{
  const fixed height_offset = fixed_1mil;
  GlideState virtual_task = task;
  virtual_task.altitude_difference += height_offset;
  GlideResult result = SolveGlide(virtual_task, glide_polar.GetVBestLD(),
                                  sink_rate);
  result.altitude_difference -= height_offset;
  return result;
}

GlideResult
MacCready::Solve(const GlideState &task) const
{
  if (!glide_polar.IsValid()) {
    /* can't solve without a valid GlidePolar() */
    GlideResult result;
    result.Reset();
    return result;
  }

  if (!positive(task.vector.distance))
    return SolveVertical(task);

  if (!positive(glide_polar.GetMC()))
    // whole task must be glide
    return OptimiseGlide(task, false);

  if (negative(task.altitude_difference))
    // whole task climb-cruise
    return SolveCruise(task);

  // task partial climb-cruise, partial glide

  // calc first final glide part
  GlideResult result_fg = OptimiseGlide(task, true);
  if (result_fg.validity == GlideResult::Validity::RESULT_OK &&
      !positive(task.vector.distance - result_fg.vector.distance))
    // whole task final glided
    return result_fg;

  // climb-cruise remainder of way

  GlideState sub_task = task;
  sub_task.vector.distance -= result_fg.vector.distance;
  sub_task.altitude_difference -= result_fg.height_glide;

  GlideResult result_cc = SolveCruise(sub_task);
  result_fg.Add(result_cc);

  return result_fg;
}

/**
 * Class used to find VOpt for a MacCready setting, for final glide
 * calculations.  Intended to be used temporarily only.
 */
class MacCreadyVopt: public ZeroFinder
{
  GlideResult res;
  const GlideState &task;
  const MacCready &mac;
  const fixed inv_mc;
  const bool allow_partial;

public:
  /**
   * Constructor
   *
   * @param _task Task to solve for
   * @param _mac MacCready object to use for search
   * @param vmin Min speed for search range
   * @param vmax Max speed for search range
   * @param _allow_partial Whether to allow partial solutions or not
   *
   * @return Initialised object (not yet searched)
   */
  MacCreadyVopt(const GlideState &_task, const MacCready &_mac,
                const fixed _inv_mc, const fixed & vmin, const fixed & vmax,
                const bool _allow_partial) :
    ZeroFinder(vmin, vmax, fixed(TOLERANCE_MC_OPT_GLIDE)),
    task(_task),
    mac(_mac),
    inv_mc(_inv_mc),
    allow_partial(_allow_partial)
    {
    }

  /**
   * Function to optimise in search
   *
   * \note the f(x) is magnified because with fixed, find_min can
   *   fail with too small df/dx
   *
   * @param V cruise true air speed (m/s)
   * @return Virtual speed (m/s) of flight
   */
  fixed
  f(const fixed v)
  {
    res = mac.SolveGlide(task, v, allow_partial);
    return res.CalcVInvSpeed(inv_mc) * fixed_360;
  }
  
  /**
   * Perform search for best cruise speed and return Result
   * @return Glide solution (optimum)
   */
  GlideResult
  Result(const fixed &v_init)
  {
    fixed v = find_min(v_init);
    return mac.SolveGlide(task, v, allow_partial);
  }
};

GlideResult
MacCready::OptimiseGlide(const GlideState &task, const bool allow_partial) const
{
  MacCreadyVopt mc_vopt(task, *this, glide_polar.GetInvMC(),
                       glide_polar.GetVMin(), glide_polar.GetVMax(),
                       allow_partial);

  return mc_vopt.Result(glide_polar.GetVMin());
}

/*
  // distance relation

  (W*(1+rho))**2+((1+rho)*Vn)**2-2*W*(1+rho)*Vn*(1+rho)*costheta-V*V

subs rho=(gamma*Vn+S)/mc
-> rho = S/mc
   k = 1+rho = (S+mc)/mc
    rho = (S+M)/M-1

*/
