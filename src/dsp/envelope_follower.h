#ifndef ENVELOPE_FOLLOWER_H
#define ENVELOPE_FOLLOWER_H

#include "../lib/macros.h"

#include "audio_detector.h"
#include "audio_filter.h"
#include "dsp.h"

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;
typedef struct dsp_callback_t *DSP_callback;

typedef struct envelope_follower_t {
  audio_detector_params detector;
  DSP_callback d;
  audio_filter_params filter;
  DSP_callback f;
  double fc;
  double threshold; // linear, not dB
  double sensitivity; // 0.25 to 10.0
} envelope_follower_params;

void dsp_envelope_follower_set_params(dsp_state *state, envelope_follower_params params);

#endif
