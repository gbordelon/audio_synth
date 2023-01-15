#ifndef FX_COMB_FILTER_H
#define FX_COMB_FILTER_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "ringbuf.h"
#include "simple_lpf.h"

/*
 * Based on the description from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */

typedef struct fx_unit_comb_filter_state_t {
  FTYPE delay_ms;
  uint32_t delay_samps;
  FTYPE rt60_ms;
  uint32_t rt60_samps;

  FTYPE g;

  bool enable_lpf;
  bool interpolate;

  FTYPE buf_len_ms;
  uint32_t buf_len_samps;
  Ringbuf bufs[2];
  Simple_lpf lpf;
} fx_unit_comb_filter_state;

typedef fx_unit_comb_filter_state *FX_unit_comb_filter_state;

typedef struct fx_unit_comb_filter_params_t {
  FTYPE delay_ms;
  FTYPE rt60_ms;

  FTYPE lpf_g;

  bool enable_lpf;
  bool interpolate;
} fx_unit_comb_filter_params;

typedef struct fx_unit_comb_filter_params_t *FX_unit_comb_filter_params;

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_comb_filter_init(FX_unit_params params);
fx_unit_params fx_unit_comb_filter_default();

#endif
