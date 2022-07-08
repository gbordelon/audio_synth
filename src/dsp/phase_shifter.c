#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_detector.h"
#include "audio_filter.h"
#include "dsp.h"
#include "phase_shifter.h"

const double apf_minF[PHASER_STAGES] = {
  16.0,
  33.0,
  48.0,
  98.0,
  160.0,
  260.0
};
const double apf_maxF[PHASER_STAGES] = {
  1600.0,
  3300.0,
  4800.0,
  9800.0,
  16000.0,
  20480.0
};

FTYPE
mono_phase_shifter(FTYPE *L, dsp_state *state, FTYPE control)
{
  double lfo_out = ugen_sample_mod(state->phase_shifter.lfo, 0.0);
  lfo_out *= state->phase_shifter.lfo_scale;

  // set fc for all 6 apfs
  int i;
  for (i = 0; i < PHASER_STAGES; i++) {
    audio_filter_params params = state->phase_shifter.apfs[i]->state.audio_filter; 
    params.fc = apf_minF[i] + (apf_maxF[i] - apf_minF[i]) * 0.5 * (lfo_out + 1.0);
    dsp_audio_filter_set_params(&state->phase_shifter.apfs[i]->state, params);
  }
  
  // get G values from all apfs
  double gamma1 = biquad_get_G(&state->phase_shifter.apfs[5]->state.audio_filter.biquad);
  double gamma2 = biquad_get_G(&state->phase_shifter.apfs[4]->state.audio_filter.biquad) * gamma1;
  double gamma3 = biquad_get_G(&state->phase_shifter.apfs[3]->state.audio_filter.biquad) * gamma2;
  double gamma4 = biquad_get_G(&state->phase_shifter.apfs[2]->state.audio_filter.biquad) * gamma3;
  double gamma5 = biquad_get_G(&state->phase_shifter.apfs[1]->state.audio_filter.biquad) * gamma4;
  double gamma6 = biquad_get_G(&state->phase_shifter.apfs[0]->state.audio_filter.biquad) * gamma5;

  double K = state->phase_shifter.intensity;
  double alpha0 = 1.0 / (1.0 + K * gamma6);

  double Sn = gamma5 * biquad_get_S(&state->phase_shifter.apfs[0]->state.audio_filter.biquad)
            + gamma4 * biquad_get_S(&state->phase_shifter.apfs[1]->state.audio_filter.biquad)
            + gamma3 * biquad_get_S(&state->phase_shifter.apfs[2]->state.audio_filter.biquad)
            + gamma2 * biquad_get_S(&state->phase_shifter.apfs[3]->state.audio_filter.biquad)
            + gamma1 * biquad_get_S(&state->phase_shifter.apfs[4]->state.audio_filter.biquad)
            +          biquad_get_S(&state->phase_shifter.apfs[5]->state.audio_filter.biquad); 

  double Lc = alpha0 * (*L - K * Sn);
  double Rc = Lc;
  stereo_fx_chain(state->phase_shifter.apfs[0], &Lc, &Rc);

  *L = 0.707 * *L + 0.707 * Lc;
  return control; // or detect_val?
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
    params.apfs[i] = dsp_init_audio_filter(params.filters[i]);
  }

  for (i = PHASER_STAGES - 1; i > 0; i--) {
    dsp_add_to_chain(params.apfs[i], params.apfs[i - 1]);
  }

  cb->state.phase_shifter = params;
  cb->state.phase_shifter.lfo = ugen_init_sin(params.lfo_rate);

  dsp_set_mono_left(cb, mono_phase_shifter);

  return cb;
}

DSP_callback
dsp_init_phase_shifter_default()
{
  phase_shifter_params params = {
    .lfo_rate = 0.5,
    .lfo_scale = 0.7,
    .intensity = 0.85,
    .quad_phase_lfo = false
  };

  int i;
  for (i = 0; i < PHASER_STAGES; i++) {
    // .fc set by lfo in sample processing fn
    params.filters[i].alg = AF_APF1;
  }

  return dsp_init_phase_shifter(params);
}

void
dsp_phase_shifter_cleanup(DSP_callback ps)
{
  ugen_cleanup(ps->state.phase_shifter.lfo);
  dsp_cleanup(ps->state.phase_shifter.apfs[0]); // chaining should cleanup the rest
  dsp_cleanup(ps);
}
