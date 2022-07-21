#ifndef TRIODE_CLASS_A_H
#define TRIODE_CLASS_A_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "waveshaper.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 19
 */

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;
typedef struct dsp_callback_t *DSP_callback;

typedef struct triode_class_a_params_t {
  FTYPE output_gain;
  bool invert_output;
  bool enable_hpf;
  bool enable_lsf;
  FTYPE low_shelf_fc;
  FTYPE low_shelf_boost_cut_db;
  FTYPE hpf_fc;
  waveshaper_params shaper_params;
  DSP_callback shaper;
  DSP_callback hpf;
  DSP_callback lsf;
} triode_class_a_params;

void dsp_triode_class_a_set_params(dsp_state *state, triode_class_a_params params);
void dsp_triode_class_a_cleanup(DSP_callback dsp);

#endif
