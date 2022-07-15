#ifndef AUDIO_DELAY_H
#define AUDIO_DELAY_H

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

typedef struct audio_delay_params_t {
  audio_delay_algorithm alg;
  audio_delay_update_e update_type; // not used yet
  FTYPE wet_mix;
  FTYPE dry_mix;
  FTYPE feedback;
  FTYPE delay_samps_l;
  FTYPE delay_samps_r;
  FTYPE delay_ratio; // right is ratio * left
  Ringbuf bufs[2]; // 0 is left, 1 is right
} audio_delay_params;

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;
typedef struct dsp_callback_t *DSP_callback;

void dsp_audio_delay_set_params(dsp_state *state, audio_delay_params params);

void dsp_audio_delay_cleanup(DSP_callback cb);

#endif
