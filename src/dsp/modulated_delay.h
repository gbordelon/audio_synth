#ifndef MODULATED_DELAY_H
#define MODULATED_DELAY_H

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#include "audio_delay.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 15
 */

typedef enum {
  MD_FLANGER,
  MD_CHORUS,
  MD_VIBRATO
} modulated_delay_algorithm;

typedef struct modulated_delay_params_t {
  FTYPE sample_rate;
  modulated_delay_algorithm alg;
  audio_delay_params adp;
  Ugen lfo;
  FTYPE lfo_freq;
  FTYPE lfo_depth;
  FTYPE min_delay_samps;
  FTYPE max_depth_samps;
  DSP_callback ad;
} modulated_delay_params;

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;
typedef struct dsp_callback_t *DSP_callback;

void dsp_modulated_delay_set_params(dsp_state *state, modulated_delay_params params);

void dsp_modulated_delay_reset(DSP_callback cb);

void dsp_modulated_delay_cleanup(DSP_callback cb);

#endif
