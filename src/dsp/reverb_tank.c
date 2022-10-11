#include <stdio.h>

#include "../lib/macros.h"

#include "delay_apf.h"
#include "dsp.h"
#include "reverb_tank.h"
#include "simple_delay.h"
#include "simple_lpf.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */
FTYPE
stereo_reverb(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  FTYPE global_fb = simple_delay_read(state->reverb_tank.branch_delays[NUM_BRANCHES-1]);
  FTYPE fb = global_fb * state->reverb_tank.k_rt;
  FTYPE xnL = *L;
  FTYPE xnR = *R;
  FTYPE xn_mono = 0.5 * (xnL + xnR);
  FTYPE predelay_out = simple_delay_read(state->reverb_tank.pre_delay);
  simple_delay_write(state->reverb_tank.pre_delay, xn_mono);

  FTYPE input = predelay_out + fb;
  int i;
  for (i = 0; i < NUM_BRANCHES; i++) {
    FTYPE apf_out = delay_apf_process(state->reverb_tank.nested_branch_delays[i], input);
    FTYPE lpf_out = simple_lpf_process(state->reverb_tank.branch_lpfs[i], apf_out);
    FTYPE delay_out = state->reverb_tank.k_rt * simple_delay_read(state->reverb_tank.branch_delays[i]);
    simple_delay_write(state->reverb_tank.branch_delays[i], lpf_out);
    input = delay_out + predelay_out;
  }

  FTYPE weight = 0.707;
  FTYPE outL = 0.0;
  outL += weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[0], 23.0);
  outL -= weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[1], 41.0);
  outL += weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[2], 59.0);
  outL -= weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[3], 73.0);

  FTYPE outR = 0.0;
  outR -= weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[0], 29.0);
  outR += weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[1], 43.0);
  outR -= weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[2], 61.0);
  outR += weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[3], 79.0);

  if (state->reverb_tank.density == REVERB_THICK) {
    outL += weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[0], 31.0);
    outL -= weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[1], 47.0);
    outL += weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[2], 67.0);
    outL -= weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[3], 83.0);

    outR -= weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[0], 37.0);
    outR += weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[1], 53.0);
    outR -= weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[2], 71.0);
    outR += weight * simple_delay_read_at_percentage(state->reverb_tank.branch_delays[3], 89.0);
  }

  FTYPE tank_out_L = outL, tank_out_R = outR;
  stereo_fx_chain(state->reverb_tank.shelving_filters[0], &tank_out_L, &tank_out_R);
  stereo_fx_chain(state->reverb_tank.shelving_filters[1], &tank_out_L, &tank_out_R);

  FTYPE dry = state->reverb_tank.dry_mix;
  FTYPE wet = state->reverb_tank.wet_mix;

  *L = dry * xnL + wet * tank_out_L;
  *R = dry * xnR + wet * tank_out_R;
  return control;
}

void
dsp_reverb_tank_reset(DSP_callback cb)
{
  int i;
  for (i = 0; i < NUM_BRANCHES; i++) {
    if (cb->state.reverb_tank.branch_delays[i]) {
      simple_delay_reset(cb->state.reverb_tank.branch_delays[i]);
    }
    if (cb->state.reverb_tank.nested_branch_delays[i]) {
      delay_apf_reset(cb->state.reverb_tank.nested_branch_delays[i]);
    }
  }
  if (cb->state.reverb_tank.pre_delay) {
    simple_delay_reset(cb->state.reverb_tank.pre_delay);
  }
}

void
reverb_tank_cleanup_helper(dsp_state *state)
{
  int i;
  for (i = 0; i < NUM_BRANCHES; i++) {
    if (state->reverb_tank.branch_delays[i]) {
      simple_delay_cleanup(state->reverb_tank.branch_delays[i]);
    }
    if (state->reverb_tank.nested_branch_delays[i]) {
      delay_apf_cleanup(state->reverb_tank.nested_branch_delays[i]);
    }
    if (state->reverb_tank.branch_lpfs[i]) {
      simple_lpf_cleanup(state->reverb_tank.branch_lpfs[i]);
    }
  }
  for (i = 0; i < NUM_CHANNELS; i++) {
    if (state->reverb_tank.shelving_filters[i]) {
      dsp_cleanup(state->reverb_tank.shelving_filters[i]);
    }
  }
  if (state->reverb_tank.pre_delay) {
    simple_delay_cleanup(state->reverb_tank.pre_delay);
  }
}

void
dsp_reverb_tank_set_params(
    dsp_state *state,
    reverb_tank_params params)
{
  int i, m;
  reverb_tank_cleanup_helper(state);

  if (params.sample_rate < DEFAULT_SAMPLE_RATE) {
    params.sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE;
  }
  state->reverb_tank = params;

  FTYPE global_max_apf_delay =
    state->reverb_tank.apf_delay_max_ms
    * state->reverb_tank.apf_delay_weight;
  FTYPE global_max_fixed_delay =
    state->reverb_tank.fixed_delay_max_ms
    * state->reverb_tank.fixed_delay_weight;

  FTYPE lfo_rates[NUM_BRANCHES] = { 0.15, 0.33, 0.57, 0.73 };
  for (i = m = 0; i < NUM_BRANCHES; i++) {
    // nested delays
    state->reverb_tank.nested_branch_delays[i] = delay_apf_init(
      0.9,
      0.5,
      0.0,
      global_max_apf_delay * state->reverb_tank.apf_delay_weights[m++],
      lfo_rates[i],
      1.0,
      0.3,
      params.sample_rate
    );
    state->reverb_tank.nested_branch_delays[i]->enable_lfo = true;
    state->reverb_tank.nested_branch_delays[i]->enable_lpf = false;
    state->reverb_tank.nested_branch_delays[i]->nested_apf = delay_apf_init(
      0.0,
      -0.5,
      0.0,
      global_max_apf_delay * state->reverb_tank.apf_delay_weights[m++],
      lfo_rates[i],
      1.0,
      0.3,
      params.sample_rate
    );
    state->reverb_tank.nested_branch_delays[i]->nested_apf->enable_lfo = true;
    state->reverb_tank.nested_branch_delays[i]->nested_apf->enable_lpf = false;

    // fixed delays
    state->reverb_tank.branch_delays[i] = simple_delay_init((uint32_t)(
      global_max_fixed_delay * state->reverb_tank.fixed_delay_weights[i]
      * 0.001 * params.sample_rate),
      params.sample_rate
    );

    // lpfs
    state->reverb_tank.branch_lpfs[i] = simple_lpf_init(state->reverb_tank.lpf_g);
  }

  // shelving filters
  two_band_shelving_filter_params fp = {
    .sample_rate = params.sample_rate,
    .low_shelf_fc = state->reverb_tank.low_shelf_fc, 
    .low_shelf_boost_cut_db = state->reverb_tank.low_shelf_boost_cut_db,
    .high_shelf_fc = state->reverb_tank.high_shelf_fc, 
    .high_shelf_boost_cut_db = state->reverb_tank.high_shelf_boost_cut_db,
  };
  for (i = 0; i < NUM_CHANNELS; i++) {
    state->reverb_tank.shelving_filters[i] = dsp_init_two_band_shelving_filter(fp);
  }
  state->reverb_tank.shelving_filters[1]->fn_type = DSP_MONO_R;

  // pre delay
  state->reverb_tank.pre_delay = simple_delay_init((uint32_t)(
    params.pre_delay_time_ms * 0.001 * params.sample_rate),
    params.sample_rate
  );
}

DSP_callback
dsp_init_reverb_tank(reverb_tank_params params)
{
  DSP_callback cb = dsp_init();
  dsp_reverb_tank_set_params(&cb->state, params);
  dsp_set_stereo(cb, stereo_reverb);

  return cb;
}

DSP_callback
dsp_init_reverb_tank_default()
{
  reverb_tank_params params = {
    .density = REVERB_THICK,
    // tweaker settings
    .apf_delay_max_ms = 33.0,
    .apf_delay_weight = 0.85,
    .fixed_delay_max_ms = 81.0,
    .fixed_delay_weight = 1.0,
    .pre_delay_time_ms = 25.0,

    // direct control    
    .lpf_g = 0.3,
    .k_rt = 0.75,
    .dry_mix = pow(10.0, 0.0 / 20.0),
    .wet_mix = pow(10.0, -12.0 / 20.0),
    .low_shelf_fc = 150.0,
    .low_shelf_boost_cut_db = -20.0,
    .high_shelf_fc = 4000.0,
    .high_shelf_boost_cut_db = -6.0,

    // experimentally determined. probably shouldn't tweak
    .apf_delay_weights = { 0.317, 0.873, 0.477, 0.291, 0.993, 0.757, 0.179, 0.575 },
    .fixed_delay_weights = { 1.0, 0.873, 0.707, 0.667 },
  };
  DSP_callback cb = dsp_init_reverb_tank(params);

  return cb;
}

void
dsp_reverb_tank_cleanup(DSP_callback cb)
{
  reverb_tank_cleanup_helper(&cb->state);
  dsp_cleanup(cb);
}
