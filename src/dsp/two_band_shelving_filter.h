#ifndef TWO_BAND_SHELVING_FILTER_H
#define TWO_BAND_SHELVING_FILTER_H

#include "../lib/macros.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;
typedef struct dsp_callback_t *DSP_callback;

typedef struct two_band_shelving_filter_params_t {
  FTYPE low_shelf_fc;
  FTYPE low_shelf_boost_cut_db;
  FTYPE high_shelf_fc;
  FTYPE high_shelf_boost_cut_db;
  DSP_callback filter_head;
} two_band_shelving_filter_params;

void dsp_two_band_shelving_filter_set_params(dsp_state *state, two_band_shelving_filter_params params);
void dsp_two_band_shelving_filter_cleanup(DSP_callback cb);

#endif
