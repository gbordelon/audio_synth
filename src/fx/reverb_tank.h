#ifndef FX_REVERB_TANK_H
#define FX_REVERB_TANK_H

#include "../lib/macros.h"

#include "delay_apf.h"
#include "simple_delay.h"
#include "simple_lpf.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef struct fx_compound_unit_t *FX_compound_unit;
typedef struct fx_unit_state_t *FX_unit_state;
typedef int16_t fx_unit_idx;

#define NUM_BRANCHES 4

typedef enum {
  REVERB_THICK,
  REVERB_SPARSE
} reverb_density_e;

typedef struct fx_unit_reverb_tank_state_t {
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

  FTYPE apf_delay_weights[NUM_BRANCHES<<1];
  FTYPE fixed_delay_weights[NUM_BRANCHES];

  Simple_delay pre_delay;
  Simple_delay branch_delays[NUM_BRANCHES];
  Delay_apf nested_branch_delays[NUM_BRANCHES];
  Simple_lpf branch_lpfs[NUM_BRANCHES];

  fx_unit_idx shelving_filter;
  fx_unit_idx sum;
  fx_unit_idx passthru;
} fx_unit_reverb_tank_state;


typedef fx_unit_reverb_tank_state *FX_unit_reverb_tank_state;

typedef struct fx_unit_reverb_tank_params_t {
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

  // experimentally determined. probably shouldn't tweak
  FTYPE apf_delay_weights[NUM_BRANCHES<<1];
  FTYPE fixed_delay_weights[NUM_BRANCHES];
} fx_unit_reverb_tank_params;

typedef struct fx_unit_reverb_tank_params_t *FX_unit_reverb_tank_params;

fx_unit_idx fx_unit_reverb_tank_init(FX_unit_params params);
FX_compound_unit fx_compound_unit_reverb_tank_init(FX_unit_params params, FX_unit_params shelving_filter_params, FX_unit_params sum_params);
fx_unit_params fx_unit_reverb_tank_default();
void fx_unit_reverb_tank_set_params(FX_unit_state state, FX_unit_params params);

fx_unit_params fx_unit_reverb_tank_default();
fx_unit_params fx_unit_reverb_tank_shelving_filter_default();
fx_unit_params fx_unit_reverb_tank_sum_default();

#endif
