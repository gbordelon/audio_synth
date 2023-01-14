#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_detector.h"
#include "fx.h"

extern FX_unit fx_unit_head;

void
fx_unit_audio_detector_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define state (fx_unit_head[idx].state.u.audio_detector)
  dst[FX_L] = src[FX_L]; // pass through
  dst[FX_R] = src[FX_R]; // pass through
  FTYPE input = src[FX_L];

  if (state.detect_mode == AUDIO_DETECTOR_MODE_MS ||
      state.detect_mode == AUDIO_DETECTOR_MODE_RMS) {
    input *= input;
  }

  FTYPE current_envelope = 0.0;
  if (input > state.previous_envelope) {
    current_envelope = state.attack_time * (state.previous_envelope - input) + input;
  } else {
    current_envelope = state.release_time * (state.previous_envelope - input) + input;
  }

  if (state.clamp_to_unity_max) {
    current_envelope = fmin(current_envelope, 1.0);
  }
  current_envelope = fmax(current_envelope, 0.0);

  state.previous_envelope = current_envelope;

  if (state.detect_mode == AUDIO_DETECTOR_MODE_RMS) {
    current_envelope = pow(current_envelope, 0.5);
  }

  if (!state.detect_db) {
    dst[FX_C] = current_envelope;
    return;
  }

  if (current_envelope <= 0) {
    dst[FX_C] = -96.0;
    return;
  }

  dst[FX_C] = 20.0 * log10(current_envelope);
#undef state
#undef src
#undef dst
}

void
fx_unit_audio_detector_cleanup(FX_unit_state state)
{
  // do nothing
}

void
fx_unit_audio_detector_set_params(FX_unit_state state, FX_unit_params params)
{
  state->u.audio_detector.attack_time = exp(AUDIO_DETECTOR_ENVELOPE_ANALOG_TC / (params->u.audio_detector.attack_time * params->sample_rate * 0.001));
  state->u.audio_detector.release_time = exp(AUDIO_DETECTOR_ENVELOPE_ANALOG_TC / (params->u.audio_detector.release_time * params->sample_rate * 0.001));
  state->u.audio_detector.detect_mode = params->u.audio_detector.detect_mode;
  state->u.audio_detector.detect_db = params->u.audio_detector.detect_db;
  state->u.audio_detector.clamp_to_unity_max = params->u.audio_detector.clamp_to_unity_max;
  state->u.audio_detector.previous_envelope = 0.0;
}

void
fx_unit_audio_detector_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_audio_detector_set_params(state, params);
}

fx_unit_idx
fx_unit_audio_detector_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_audio_detector_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_audio_detector_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_audio_detector_reset;

  fx_unit_audio_detector_reset(&fx_unit_head[idx].state, params);
  return idx;
}

fx_unit_params
fx_unit_audio_detector_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_AUDIO_DETECTOR;
  params.u.audio_detector.attack_time = 1.0;
  params.u.audio_detector.release_time = 1.0;
  params.u.audio_detector.detect_mode = AUDIO_DETECTOR_MODE_PEAK;
  params.u.audio_detector.detect_db = false;
  params.u.audio_detector.clamp_to_unity_max = true;

  return params;
}
