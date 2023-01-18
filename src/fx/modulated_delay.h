#ifndef FX_MODULATED_DELAY_H
#define FX_MODULATED_DELAY_H

#include <stdlib.h>

#include "../lib/macros.h"

#include "audio_delay.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 15
 */

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef struct fx_compound_unit_t *FX_compound_unit;
typedef int16_t fx_unit_idx;

typedef enum {
  MD_FLANGER,
  MD_CHORUS,
  MD_VIBRATO
} modulated_delay_algorithm;

typedef struct fx_unit_modulated_delay_state_t {
  fx_unit_idx audio_delay;
  fx_unit_idx control_joiner;
  fx_unit_idx signal_source;
  fx_unit_idx passthru;
  modulated_delay_algorithm alg;
  FTYPE lfo_depth;
  FTYPE min_delay_samps;
  FTYPE max_depth_samps;
  FX_unit_params ad_params;
} fx_unit_modulated_delay_state;

typedef fx_unit_modulated_delay_state *FX_unit_modulated_delay_state;

typedef struct fx_unit_modulated_delay_params_t {
  modulated_delay_algorithm alg;
  FTYPE lfo_depth;
  FTYPE min_delay_ms;
  FTYPE max_depth_ms;
  FX_unit_params ad_params;
} fx_unit_modulated_delay_params;

typedef fx_unit_modulated_delay_params *FX_unit_modulated_delay_params;

FX_compound_unit fx_compound_unit_modulated_delay_init(FX_unit_params params, FX_unit_params delay_params, FX_unit_params signal_source_params);

fx_unit_params fx_unit_modulated_delay_default();
fx_unit_params fx_unit_modulated_delay_audio_delay_default();
fx_unit_params fx_unit_modulated_delay_signal_source_default();

fx_unit_params fx_unit_modulated_delay_chorus_default();
fx_unit_params fx_unit_modulated_delay_audio_delay_chorus_default();
fx_unit_params fx_unit_modulated_delay_signal_source_chorus_default();

fx_unit_params fx_unit_modulated_delay_flanger_default();
fx_unit_params fx_unit_modulated_delay_audio_delay_flanger_default();
fx_unit_params fx_unit_modulated_delay_signal_source_flanger_default();

fx_unit_params fx_unit_modulated_delay_vibrato_default();
fx_unit_params fx_unit_modulated_delay_audio_delay_vibrato_default();
fx_unit_params fx_unit_modulated_delay_signal_source_vibrato_default();

void fx_unit_modulated_delay_params_free(FX_unit_params params);

#endif
