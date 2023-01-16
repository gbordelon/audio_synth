#ifndef FX_AUDIO_DELAY_H
#define FX_AUDIO_DELAY_H

#include "../lib/macros.h"

#include "ringbuf.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 14
 */

typedef enum {
  AD_NORMAL,
  AD_PINGPONG
} audio_delay_algorithm;

typedef enum {
  AD_LEFT_AND_RIGHT,
  AD_LEFT_PLUS_RATIO
} audio_delay_update_e;

typedef struct fx_unit_audio_delay_state_t {
  audio_delay_algorithm alg;
  audio_delay_update_e update_type; // not used yet
  FTYPE wet_mix;
  FTYPE dry_mix;
  FTYPE feedback;
  FTYPE delay_samps_l;
  FTYPE delay_samps_r;
  FTYPE delay_ratio; // right is ratio * left
  Ringbuf bufs[2]; // 0 is left, 1 is right
} fx_unit_audio_delay_state;

typedef fx_unit_audio_delay_state *FX_unit_audio_delay_state;

typedef struct fx_unit_audio_delay_params_t {
  audio_delay_algorithm alg;
  audio_delay_update_e update_type; // not used yet
  FTYPE wet_mix;
  FTYPE dry_mix;
  FTYPE feedback;
  FTYPE delay_samps_l;
  FTYPE delay_samps_r;
  FTYPE delay_ratio; // right is ratio * left
} fx_unit_audio_delay_params;

typedef struct fx_unit_audio_delay_params_t *FX_unit_audio_delay_params;

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef struct fx_compound_unit_t *FX_compound_unit;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_audio_delay_init(FX_unit_params params);
FX_compound_unit fx_compound_unit_audio_delay_init(FX_unit_params params);
fx_unit_params fx_unit_audio_delay_default();

#endif
