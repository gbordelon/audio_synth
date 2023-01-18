#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../lib/macros.h"

#include "delay_apf.h"
#include "reverb_tank.h"
#include "simple_delay.h"
#include "simple_lpf.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */
void
fx_unit_reverb_tank_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define state fx_unit_head[idx].state
  FTYPE global_fb = simple_delay_read(state.u.reverb_tank.branch_delays[NUM_BRANCHES-1]);
  FTYPE fb = global_fb * state.u.reverb_tank.k_rt;
  FTYPE xnL = src[FX_L];
  FTYPE xnR = src[FX_R];
  FTYPE xn_mono = 0.5 * (xnL + xnR); // 0.707 better ??
  FTYPE predelay_out = simple_delay_read(state.u.reverb_tank.pre_delay);
  simple_delay_write(state.u.reverb_tank.pre_delay, xn_mono);

  FTYPE input = predelay_out + fb;
  int i;
  for (i = 0; i < NUM_BRANCHES; i++) {
    FTYPE apf_out = delay_apf_process(state.u.reverb_tank.nested_branch_delays[i], input);
    FTYPE lpf_out = simple_lpf_process(state.u.reverb_tank.branch_lpfs[i], apf_out);
    FTYPE delay_out = state.u.reverb_tank.k_rt * simple_delay_read(state.u.reverb_tank.branch_delays[i]);
    simple_delay_write(state.u.reverb_tank.branch_delays[i], lpf_out);
    input = delay_out + predelay_out;
  }

  FTYPE weight = 0.707;
  FTYPE outL = 0.0;
  outL += weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[0], 23.0);
  outL -= weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[1], 41.0);
  outL += weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[2], 59.0);
  outL -= weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[3], 73.0);

  FTYPE outR = 0.0;
  outR -= weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[0], 29.0);
  outR += weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[1], 43.0);
  outR -= weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[2], 61.0);
  outR += weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[3], 79.0);

  if (state.u.reverb_tank.density == REVERB_THICK) {
    outL += weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[0], 31.0);
    outL -= weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[1], 47.0);
    outL += weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[2], 67.0);
    outL -= weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[3], 83.0);

    outR -= weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[0], 37.0);
    outR += weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[1], 53.0);
    outR -= weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[2], 71.0);
    outR += weight * simple_delay_read_at_percentage(state.u.reverb_tank.branch_delays[3], 89.0);
  }

  dst[FX_L] = outL;
  dst[FX_R] = outR;
  dst[FX_C] = src[FX_C];
#undef state
#undef src
#undef dst
}

void
fx_unit_reverb_tank_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_reverb_tank_set_params(state, params);

  int i;
  for (i = 0; i < NUM_BRANCHES; i++) {
    if (state->u.reverb_tank.branch_delays[i]) {
      simple_delay_reset(state->u.reverb_tank.branch_delays[i]);
    }
    if (state->u.reverb_tank.nested_branch_delays[i]) {
      delay_apf_reset(state->u.reverb_tank.nested_branch_delays[i]);
    }
  }
  if (state->u.reverb_tank.pre_delay) {
    simple_delay_reset(state->u.reverb_tank.pre_delay);
  }
}

void
reverb_tank_cleanup_helper(FX_unit_state state)
{
  int i;
  for (i = 0; i < NUM_BRANCHES; i++) {
    if (state->u.reverb_tank.branch_delays[i]) {
      simple_delay_cleanup(state->u.reverb_tank.branch_delays[i]);
    }
    if (state->u.reverb_tank.nested_branch_delays[i]) {
      delay_apf_cleanup(state->u.reverb_tank.nested_branch_delays[i]);
    }
    if (state->u.reverb_tank.branch_lpfs[i]) {
      simple_lpf_cleanup(state->u.reverb_tank.branch_lpfs[i]);
    }
  }
  if (state->u.reverb_tank.pre_delay) {
    simple_delay_cleanup(state->u.reverb_tank.pre_delay);
  }
}

void
fx_unit_reverb_tank_cleanup(FX_unit_state state)
{
  reverb_tank_cleanup_helper(state);

  fx_unit_head[state->u.reverb_tank.passthru]
    .state.f.cleanup(
      &fx_unit_head[state->u.reverb_tank.passthru].state);

  state->u.reverb_tank.passthru = FX_UNIT_IDX_NONE;

  fx_unit_head[state->u.reverb_tank.sum]
    .state.f.cleanup(
      &fx_unit_head[state->u.reverb_tank.sum].state);

  state->u.reverb_tank.sum = FX_UNIT_IDX_NONE;

  fx_unit_head[state->u.reverb_tank.shelving_filter]
    .state.f.cleanup(
      &fx_unit_head[state->u.reverb_tank.shelving_filter].state);

  state->u.reverb_tank.shelving_filter = FX_UNIT_IDX_NONE;
}

void
fx_unit_reverb_tank_set_params(FX_unit_state state, FX_unit_params params)
{
  int i, m;
  reverb_tank_cleanup_helper(state);

  state->u.reverb_tank.density = params->u.reverb_tank.density;
  state->u.reverb_tank.apf_delay_max_ms = params->u.reverb_tank.apf_delay_max_ms;
  state->u.reverb_tank.apf_delay_weight = params->u.reverb_tank.apf_delay_weight;
  state->u.reverb_tank.pre_delay_time_ms = params->u.reverb_tank.pre_delay_time_ms;

  state->u.reverb_tank.fixed_delay_max_ms = params->u.reverb_tank.fixed_delay_max_ms;
  state->u.reverb_tank.fixed_delay_weight = params->u.reverb_tank.fixed_delay_weight;

  state->u.reverb_tank.lpf_g = params->u.reverb_tank.lpf_g;
  state->u.reverb_tank.k_rt = params->u.reverb_tank.k_rt;
  memcpy(state->u.reverb_tank.apf_delay_weights,
         params->u.reverb_tank.apf_delay_weights,
         (NUM_BRANCHES<<1) * sizeof(FTYPE));
  memcpy(state->u.reverb_tank.fixed_delay_weights,
         params->u.reverb_tank.fixed_delay_weights,
         NUM_BRANCHES * sizeof(FTYPE));

  FTYPE global_max_apf_delay =
    state->u.reverb_tank.apf_delay_max_ms
    * state->u.reverb_tank.apf_delay_weight;
  FTYPE global_max_fixed_delay =
    state->u.reverb_tank.fixed_delay_max_ms
    * state->u.reverb_tank.fixed_delay_weight;

  FTYPE lfo_rates[NUM_BRANCHES] = { 0.15, 0.33, 0.57, 0.73 };
  for (i = m = 0; i < NUM_BRANCHES; i++) {
    // nested delays
    state->u.reverb_tank.nested_branch_delays[i] = delay_apf_init(
      0.9,
      0.5,
      0.0,
      global_max_apf_delay * state->u.reverb_tank.apf_delay_weights[m++],
      lfo_rates[i],
      1.0,
      0.3,
      params->sample_rate
    );
    state->u.reverb_tank.nested_branch_delays[i]->enable_lfo = true;
    state->u.reverb_tank.nested_branch_delays[i]->enable_lpf = false;
    state->u.reverb_tank.nested_branch_delays[i]->nested_apf = delay_apf_init(
      0.0,
      -0.5,
      0.0,
      global_max_apf_delay * state->u.reverb_tank.apf_delay_weights[m++],
      lfo_rates[i],
      1.0,
      0.3,
      params->sample_rate
    );
    state->u.reverb_tank.nested_branch_delays[i]->nested_apf->enable_lfo = true;
    state->u.reverb_tank.nested_branch_delays[i]->nested_apf->enable_lpf = false;

    // fixed delays
    state->u.reverb_tank.branch_delays[i] = simple_delay_init((uint32_t)(
      global_max_fixed_delay * state->u.reverb_tank.fixed_delay_weights[i]
      * 0.001 * params->sample_rate),
      params->sample_rate
    );

    // lpfs
    state->u.reverb_tank.branch_lpfs[i] = simple_lpf_init(state->u.reverb_tank.lpf_g);
  }

  // pre delay
  state->u.reverb_tank.pre_delay = simple_delay_init((uint32_t)(
    state->u.reverb_tank.pre_delay_time_ms * 0.001 * params->sample_rate),
    params->sample_rate
  );
}

FX_compound_unit
fx_compound_unit_reverb_tank_init(
  FX_unit_params params,
  FX_unit_params shelving_filter_params,
  FX_unit_params sum_params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_reverb_tank_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_reverb_tank_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_reverb_tank_reset;

  fx_unit_reverb_tank_reset(&fx_unit_head[idx].state, params);

  fx_unit_params pt_p = fx_unit_passthru_default();
  fx_unit_idx pt = fx_unit_passthru_init(&pt_p);

  fx_unit_idx sum = fx_unit_sum_init(sum_params);
//  fx_unit_sum_mix_set(sum, sum_params->u.sum.mix);

  fx_unit_idx filter = fx_unit_two_band_shelving_filter_init(shelving_filter_params);

  fx_unit_head[idx].state.u.reverb_tank.shelving_filter = filter;
  fx_unit_head[idx].state.u.reverb_tank.sum = sum;
  fx_unit_head[idx].state.u.reverb_tank.passthru = pt;

  // connect sum to passthru (dry) and filter (wet)
  fx_unit_parent_ref_add(sum, pt);
  fx_unit_parent_ref_add(sum, filter);

  // connect filter to reverb 
  fx_unit_parent_ref_add(filter, idx);

  // connect reverb to passthru
  fx_unit_parent_ref_add(idx, pt);

  FX_compound_unit rv = fx_compound_unit_init(4, 1);
  rv->units[0] = pt;
  rv->units[1] = idx;
  rv->units[2] = filter;
  rv->units[3] = sum;

  rv->heads[0] = pt;

  rv->tail = sum;
  
  return rv;
}

fx_unit_params
fx_unit_reverb_tank_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_REVERB_TANK;
  params.u.reverb_tank.density = REVERB_THICK;

  // tweaker settings
  params.u.reverb_tank.apf_delay_max_ms = 33.0;
  params.u.reverb_tank.apf_delay_weight = 0.85;
  params.u.reverb_tank.fixed_delay_max_ms = 81.0;
  params.u.reverb_tank.fixed_delay_weight = 1.0;
  // 25.0 for room or hall
  // 155.0 for larm room
  params.u.reverb_tank.pre_delay_time_ms = 25.0;

  // direct control
  // 0.3
  params.u.reverb_tank.lpf_g = 0.3; // [0, 1]
  // 0.75, larger for larger room
  params.u.reverb_tank.k_rt = 0.75;

  FTYPE arr1[] = { 0.317, 0.873, 0.477, 0.291, 0.993, 0.757, 0.179, 0.575 };
  FTYPE arr2[] = { 1.0, 0.873, 0.707, 0.667 };
  memcpy(params.u.reverb_tank.apf_delay_weights, arr1, (NUM_BRANCHES<<1) * sizeof(FTYPE));
  memcpy(params.u.reverb_tank.fixed_delay_weights, arr2, NUM_BRANCHES * sizeof(FTYPE));

  return params;
}

fx_unit_params
fx_unit_reverb_tank_shelving_filter_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_TWO_BAND_SHELVING_FILTER;

  // direct control
  params.u.two_band_shelving_filter.low_shelf_fc = 150.0;
  params.u.two_band_shelving_filter.low_shelf_boost_cut_db = -20.0;
  params.u.two_band_shelving_filter.high_shelf_fc = 4000.0;
  params.u.two_band_shelving_filter.high_shelf_boost_cut_db = -6.0;

  return params;
}

fx_unit_params
fx_unit_reverb_tank_sum_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_SUM;

  FTYPE *mix = calloc(2, sizeof(FTYPE));
  params.u.sum.mix = mix;

  // direct control
  mix[0] = pow(10.0, 0.0 / 20.0); // dry mix
  mix[1] = pow(10.0, -12.0 / 20.0); // wet mix

  return params;
}
