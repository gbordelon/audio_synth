#include <math.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_detector.h"
#include "audio_filter.h"
#include "dsp.h"
#include "envelope_follower.h"

FTYPE
stereo_follower(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  double Lc = *L * 0.707 + *R * 0.707;
  double Rc = Lc;
  double thresh_val = state->envelope_follower.threshold;
  double detect_val = stereo_fx_chain(state->envelope_follower.d, &Lc, &Rc);
  double delta_val = detect_val - thresh_val;

  if (delta_val <= 0.0) {
    state->envelope_follower.filter.fc = state->envelope_follower.fc;
  } else {
    double modulator_val = delta_val * state->envelope_follower.sensitivity;
    state->envelope_follower.filter.fc = modulator_val * (state->envelope_follower.fc_max - state->envelope_follower.fc) + state->envelope_follower.fc;
  }
  dsp_audio_filter_set_params(&state->envelope_follower.f->state, state->envelope_follower.filter); 

  Lc = *L, Rc = *R;
  stereo_fx_chain(state->envelope_follower.f, &Lc, &Rc);
  *L = Lc;

  Lc = *L, Rc = *R;
  stereo_fx_chain(state->envelope_follower.f, &Rc, &Lc);
  *R = Rc;

  return control; // or detect_val?
}

void
dsp_envelope_follower_set_params(
  dsp_state *state,
  envelope_follower_params params)
{
  if (params.sample_rate < DEFAULT_SAMPLE_RATE) {
    params.sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE;
  }
  state->envelope_follower = params;
  dsp_audio_detector_set_params(&state->envelope_follower.d->state, params.detector);
  dsp_audio_filter_set_params(&state->envelope_follower.f->state, params.filter);
}

DSP_callback
dsp_init_envelope_follower(envelope_follower_params params)
{
  DSP_callback cb = dsp_init();
  params.f = dsp_init_audio_filter(params.filter);
  params.d = dsp_init_audio_detector(params.detector);

  cb->state.envelope_follower = params;
  //dsp_envelope_follower_set_params(&cb->state, params);
  dsp_set_stereo(cb, stereo_follower);

  return cb;
}

DSP_callback
dsp_init_envelope_follower_default()
{
  audio_detector_params detector = {
    .sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE,
    .attack_time = 20,
    .release_time = 200.0,
    .detect_mode = AUDIO_DETECTOR_MODE_RMS,
    .detect_db = false,
    .clamp_to_unity_max = false
  };

  // .fc is set in the sample processing function, per sample
  audio_filter_params filter = {
    .sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE,
    .q = 4.707,
    .boost_cut_db = -9.0,
    .alg = AF_LPF2
  };

  envelope_follower_params params = {
    .sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE,
    .detector = detector,
    .filter = filter,
    .fc = 200.0,
    .fc_max = DEFAULT_SAMPLE_RATE>>1,
    .threshold = pow(10.0, -6.0 / 20.0), // -12 dB
    .sensitivity = 0.3
  };

  return dsp_init_envelope_follower(params);
}
