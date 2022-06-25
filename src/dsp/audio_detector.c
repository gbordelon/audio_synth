#include <stdio.h>

#include <math.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "audio_detector.h"
#include "dsp.h"

FTYPE
mono_detector(FTYPE *L, dsp_state *state, FTYPE control)
{
  double input = fabs(*L);

  if (state->audio_detector.detect_mode == AUDIO_DETECTOR_MODE_MS ||
      state->audio_detector.detect_mode == AUDIO_DETECTOR_MODE_RMS) {
    input *= input;
  }

  double current_envelope = 0.0;
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
    FTYPE attack_time_ms,
    FTYPE release_time_ms,
    detect_mode_e detect_mode,
    bool detect_db,
    bool clamp_to_unity_max)
{
  state->audio_detector.attack_time = exp(AUDIO_DETECTOR_ENVELOPE_ANALOG_TC / (attack_time_ms * DEFAULT_SAMPLE_RATE * 0.001));
  state->audio_detector.release_time = exp(AUDIO_DETECTOR_ENVELOPE_ANALOG_TC / (release_time_ms * DEFAULT_SAMPLE_RATE * 0.001));
  state->audio_detector.detect_mode = detect_mode;
  state->audio_detector.detect_db = detect_db;
  state->audio_detector.clamp_to_unity_max = clamp_to_unity_max;
  state->audio_detector.previous_envelope = 0.0;
}

void
dsp_audio_detector_set_mono_left(DSP_callback cb)
{
  cb->fn_type = DSP_MONO_L;
  cb->fn_u.mono = mono_detector;
}

void
dsp_audio_detector_set_mono_right(DSP_callback cb)
{
  cb->fn_type = DSP_MONO_R;
  cb->fn_u.mono = mono_detector;
}

DSP_callback
dsp_init_audio_detector(FTYPE attack_time_ms, FTYPE release_time_ms, detect_mode_e detect_mode, bool detect_db, bool clamp_to_unity_max)
{
  DSP_callback cb = dsp_init();
  dsp_audio_detector_set_params(&cb->state, attack_time_ms, release_time_ms, detect_mode, detect_db, clamp_to_unity_max);
  dsp_audio_detector_set_mono_left(cb);

  return cb;
}

DSP_callback
dsp_init_audio_detector_default()
{
  return dsp_init_audio_detector(1.0, 1.0, AUDIO_DETECTOR_MODE_PEAK, false, true);
}
