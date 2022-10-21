#include <stdio.h>

#include <math.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "audio_detector.h"
#include "dsp.h"

FTYPE
mono_detector(FTYPE *L, dsp_state *state, FTYPE control)
{
  FTYPE input = fabs(*L);

  if (state->audio_detector.detect_mode == AUDIO_DETECTOR_MODE_MS ||
      state->audio_detector.detect_mode == AUDIO_DETECTOR_MODE_RMS) {
    input *= input;
  }

  FTYPE current_envelope = 0.0;
  if (input > state->audio_detector.previous_envelope) {
    current_envelope = state->audio_detector.attack_time * (state->audio_detector.previous_envelope - input) + input;
  } else {
    current_envelope = state->audio_detector.release_time * (state->audio_detector.previous_envelope - input) + input;
  }

  if (state->audio_detector.clamp_to_unity_max) {
    current_envelope = fmin(current_envelope, 1.0);
  }
  current_envelope = fmax(current_envelope, 0.0);

  state->audio_detector.previous_envelope = current_envelope;

  if (state->audio_detector.detect_mode == AUDIO_DETECTOR_MODE_RMS) {
    current_envelope = pow(current_envelope, 0.5);
  }

  if (!state->audio_detector.detect_db) {
    return current_envelope;
  }

  if (current_envelope <= 0) {
    return -96.0;
  }

  return 20.0 * log10(current_envelope);
}

void
dsp_audio_detector_set_params(
    dsp_state *state,
    audio_detector_params params)
{
  if (params.sample_rate < DEFAULT_SAMPLE_RATE) {
    params.sample_rate = DEFAULT_SAMPLE_RATE;
  }
  state->audio_detector.sample_rate = params.sample_rate;
  state->audio_detector.attack_time = exp(AUDIO_DETECTOR_ENVELOPE_ANALOG_TC / (params.attack_time * params.sample_rate * 0.001));
  state->audio_detector.release_time = exp(AUDIO_DETECTOR_ENVELOPE_ANALOG_TC / (params.release_time * params.sample_rate * 0.001));
  state->audio_detector.detect_mode = params.detect_mode;
  state->audio_detector.detect_db = params.detect_db;
  state->audio_detector.clamp_to_unity_max = params.clamp_to_unity_max;
  state->audio_detector.previous_envelope = 0.0;
}

DSP_callback
dsp_init_audio_detector(audio_detector_params params)
{
  DSP_callback cb = dsp_init();

  dsp_audio_detector_set_params(&cb->state, params);
  dsp_set_mono_left(cb, mono_detector);

  return cb;
}

DSP_callback
dsp_init_audio_detector_default()
{
  audio_detector_params params = {
    .sample_rate = DEFAULT_SAMPLE_RATE,
    .attack_time = 1.0,
    .release_time = 1.0,
    .detect_mode = AUDIO_DETECTOR_MODE_PEAK,
    .detect_db = false,
    .clamp_to_unity_max = true
  };

  return dsp_init_audio_detector(params);
}

void
dsp_audio_detector_reset(DSP_callback cb)
{
  cb->state.audio_detector.previous_envelope = 0.0;
}
