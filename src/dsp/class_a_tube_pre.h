#ifndef CLASS_A_TUBE_PRE_H
#define CLASS_A_TUBE_PRE_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "triode_class_a.h"
#include "two_band_shelving_filter.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 19
 */
#define NUM_TUBES 4

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;
typedef struct dsp_callback_t *DSP_callback;

typedef struct class_a_tube_pre_params_t {
  FTYPE input_level;
  FTYPE output_level;
  triode_class_a_params tube_params;
  two_band_shelving_filter_params filter_params;
  DSP_callback triodes[NUM_TUBES];
  DSP_callback shelving_filter;
} class_a_tube_pre_params;

void dsp_class_a_tube_pre_set_params(dsp_state *state, class_a_tube_pre_params params);

#endif
