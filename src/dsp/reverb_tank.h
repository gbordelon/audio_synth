#ifndef REVERB_TANK_H
#define REVERB_TANK_H

#include "../lib/macros.h"

#include "delay_apf.h"
#include "dsp.h"
#include "reverb_tank.h"
#include "simple_delay.h"
#include "simple_lpf.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */

#define NUM_BRANCHES 4

typedef enum {
  REVERB_THICK,
  REVERB_SPARSE
} reverb_density_e;

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;
typedef struct dsp_callback_t *DSP_callback;

typedef struct reverb_tank_params_t {
  FTYPE sample_rate;
  reverb_density_e density;

  // tweaker settings
  FTYPE apf_delay_max_ms;
  FTYPE apf_delay_weight; // [0,1]
  FTYPE fixed_delay_max_ms;
  FTYPE fixed_delay_weight; // [0,1]

  // direct control params
  FTYPE pre_delay_time_ms;
  FTYPE lpf_g; // damping [0,1]
  FTYPE k_rt; // reverb time [0,1]

  FTYPE dry_mix;
  FTYPE wet_mix;

  FTYPE low_shelf_fc;
  FTYPE low_shelf_boost_cut_db;
  FTYPE high_shelf_fc;
  FTYPE high_shelf_boost_cut_db;

  FTYPE apf_delay_weights[NUM_BRANCHES<<1];
  FTYPE fixed_delay_weights[NUM_BRANCHES];

  Simple_delay pre_delay;
  Simple_delay branch_delays[NUM_BRANCHES];
  Delay_apf nested_branch_delays[NUM_BRANCHES];
  Simple_lpf branch_lpfs[NUM_BRANCHES];

  DSP_callback shelving_filters[NUM_CHANNELS];
} reverb_tank_params;

void dsp_reverb_tank_set_params(dsp_state *state, reverb_tank_params params);
void dsp_reverb_tank_cleanup(DSP_callback cb);

#endif
