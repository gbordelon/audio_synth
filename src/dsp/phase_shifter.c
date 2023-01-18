#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_detector.h"
#include "audio_filter.h"
#include "dsp.h"
#include "phase_shifter.h"

const double dsp_apf_minF[PHASER_STAGES] = {
  16.0,
  33.0,
  48.0,
  98.0,
  160.0,
  260.0
};
const double dsp_apf_maxF[PHASER_STAGES] = {
  1600.0,
  3300.0,
  4800.0,
  9800.0,
  16000.0,
  20480.0
};

void
helper(FTYPE *xn, FTYPE lfo_out, dsp_state *state)
{
  // TODO this calls calculate_filter_coeffs which is hugely expensive
  // there are only 1024 lfo outputs. how about 6 * 1024 all-pass filters?
  // prepare them all beforehand then switch which set is to be used.
  // set fc for all 6 apfs
  int i;
  for (i = 0; i < PHASER_STAGES; i++) {
    audio_filter_params params = state->phase_shifter.apfs[i]->state.audio_filter; 
    params.fc = dsp_apf_minF[i] + (dsp_apf_maxF[i] - dsp_apf_minF[i]) * 0.5 * (lfo_out + 1.0);
    dsp_audio_filter_set_params(&state->phase_shifter.apfs[i]->state, params);
  }

  // get G values from all apfs
  double gamma1 =          biquad_get_G(&state->phase_shifter.apfs[5]->state.audio_filter.biquad);
  double gamma2 = gamma1 * biquad_get_G(&state->phase_shifter.apfs[4]->state.audio_filter.biquad);
  double gamma3 = gamma2 * biquad_get_G(&state->phase_shifter.apfs[3]->state.audio_filter.biquad);
  double gamma4 = gamma3 * biquad_get_G(&state->phase_shifter.apfs[2]->state.audio_filter.biquad);
  double gamma5 = gamma4 * biquad_get_G(&state->phase_shifter.apfs[1]->state.audio_filter.biquad);
  double gamma6 = gamma5 * biquad_get_G(&state->phase_shifter.apfs[0]->state.audio_filter.biquad);

  double K = state->phase_shifter.intensity;
  double alpha0 = 1.0 / (1.0 + K * gamma6);

  double Sn = gamma5 * biquad_get_S(&state->phase_shifter.apfs[0]->state.audio_filter.biquad)
            + gamma4 * biquad_get_S(&state->phase_shifter.apfs[1]->state.audio_filter.biquad)
            + gamma3 * biquad_get_S(&state->phase_shifter.apfs[2]->state.audio_filter.biquad)
            + gamma2 * biquad_get_S(&state->phase_shifter.apfs[3]->state.audio_filter.biquad)
            + gamma1 * biquad_get_S(&state->phase_shifter.apfs[4]->state.audio_filter.biquad)
            +          biquad_get_S(&state->phase_shifter.apfs[5]->state.audio_filter.biquad); 

  double Lc = alpha0 * (*xn - K * Sn);
  double Rc = Lc;
  stereo_fx_chain(state->phase_shifter.apfs[0], &Lc, &Rc);

  *xn = 0.707 * *xn + 0.707 * Lc;
}

FTYPE
stereo_phase_shifter(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  triphase lfo_out_tri;
  ugen_sample_fast_triphase(state->phase_shifter.lfo, 0.0, lfo_out_tri);
  double lfo_out = state->phase_shifter.lfo_scale * lfo_out_tri[UGEN_PHASE_NORM];
  helper(L, lfo_out, state);

  lfo_out = state->phase_shifter.lfo_scale * lfo_out_tri[UGEN_PHASE_QUAD];
  helper(R, lfo_out, state);

  return control;
}

void
dsp_phase_shifter_set_params(
  dsp_state *state,
  phase_shifter_params params)
{
  Ugen lfo = state->phase_shifter.lfo;
  state->phase_shifter = params;
  state->phase_shifter.lfo = lfo;

  int i;
  for (i = 0; i < PHASER_STAGES; i++) {
    dsp_audio_filter_set_params(&state->phase_shifter.apfs[i]->state, params.filters[i]);
  }
}

DSP_callback
dsp_init_phase_shifter(phase_shifter_params params)
{
  DSP_callback cb = dsp_init();
  int i;
  for (i = 0; i < PHASER_STAGES; i++) {
    if (params.filters[i].sample_rate < DEFAULT_SAMPLE_RATE) {
      params.filters[i].sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE;
    }
    params.apfs[i] = dsp_init_audio_filter(params.filters[i]);
  }

  for (i = PHASER_STAGES - 1; i > 0; i--) {
    dsp_add_to_chain(params.apfs[i], params.apfs[i - 1]);
  }

  if (params.sample_rate < DEFAULT_SAMPLE_RATE) {
    params.sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE;
  }
  cb->state.phase_shifter = params;
  cb->state.phase_shifter.lfo = ugen_init_sin(params.lfo_rate, params.sample_rate);

  dsp_set_stereo(cb, stereo_phase_shifter);

  return cb;
}

DSP_callback
dsp_init_phase_shifter_default()
{
  phase_shifter_params params = {
    .sample_rate = DEFAULT_SAMPLE_RATE,
    .lfo_rate = 0.5,
    .lfo_scale = 0.7,
    .intensity = 0.85,
    .quad_phase_lfo = false
  };

  int i;
  for (i = 0; i < PHASER_STAGES; i++) {
    // .fc set by lfo in sample processing fn
    params.filters[i].alg = AF_APF1;
    params.filters[i].sample_rate = DEFAULT_SAMPLE_RATE;
  }

  return dsp_init_phase_shifter(params);
}

void
dsp_phase_shifter_reset(DSP_callback cb)
{
  ugen_reset_phase(cb->state.phase_shifter.lfo);
}

void
dsp_phase_shifter_cleanup(DSP_callback cb)
{
  ugen_cleanup(cb->state.phase_shifter.lfo);
  dsp_cleanup(cb->state.phase_shifter.apfs[0]); // chaining should cleanup the rest
  dsp_cleanup(cb);
}
