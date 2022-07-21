#include <math.h>
#include <string.h>

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#include "audio_delay.h"
#include "dsp.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 15
 */

FTYPE
stereo_effect(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  FTYPE lfo_out[4] = {0};
  ugen_sample_fast_triphase(state->modulated_delay.lfo, 0.0, lfo_out);
  audio_delay_params params = state->modulated_delay.adp;

  FTYPE depth = state->modulated_delay.lfo_depth;
  FTYPE mod_min = state->modulated_delay.min_delay_samps; 
  FTYPE mod_max = state->modulated_delay.min_delay_samps + state->modulated_delay.max_depth_samps;
  FTYPE mod_val;

  if (state->modulated_delay.alg == MD_FLANGER) {
    mod_val = bipolar_to_unipolar(depth * lfo_out[UGEN_PHASE_NORM]);
    params.delay_samps_l = unipolar_modulation_from_min(mod_val, mod_min, mod_max);
    mod_val = bipolar_to_unipolar(depth * lfo_out[UGEN_PHASE_INV]);
    params.delay_samps_r = unipolar_modulation_from_min(mod_val, mod_min, mod_max);
  } else if (state->modulated_delay.alg == MD_VIBRATO) {
    mod_val = depth * lfo_out[UGEN_PHASE_NORM];
    params.delay_samps_l = bipolar_modulation(mod_val, mod_min, mod_max);
    params.delay_samps_r = bipolar_modulation(mod_val, mod_min, mod_max);
  } else {
    mod_val = depth * lfo_out[UGEN_PHASE_NORM];
    params.delay_samps_l = bipolar_modulation(mod_val, mod_min, mod_max);
    mod_val = depth * lfo_out[UGEN_PHASE_INV];
    params.delay_samps_r = bipolar_modulation(mod_val, mod_min, mod_max);
  }
  dsp_audio_delay_set_params(&state->modulated_delay.ad->state, params);
  stereo_fx_chain(state->modulated_delay.ad, L, R);

  return control;
}

void
dsp_modulated_delay_set_params(
    dsp_state *state,
    modulated_delay_params params)
{
  if (state->modulated_delay.lfo) {
    ugen_cleanup(state->modulated_delay.lfo);
  }

  DSP_callback ad = state->modulated_delay.ad;
  state->modulated_delay = params;

  if (state->modulated_delay.sample_rate < DEFAULT_SAMPLE_RATE) {
    state->modulated_delay.sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE;
  }

  if (params.alg == MD_VIBRATO) {
    state->modulated_delay.lfo = ugen_init_sin(params.lfo_freq, state->modulated_delay.sample_rate);
  } else {
    state->modulated_delay.lfo = ugen_init_tri(params.lfo_freq, state->modulated_delay.sample_rate);
  }

  if (ad) {
    dsp_audio_delay_set_params(&ad->state, params.adp);
    state->modulated_delay.ad = ad;
  } else {
    state->modulated_delay.ad = dsp_init_audio_delay(params.adp);
  }
}

DSP_callback
dsp_init_modulated_delay(modulated_delay_params params)
{
  DSP_callback cb = dsp_init();
  dsp_modulated_delay_set_params(&cb->state, params);
  dsp_set_stereo(cb, stereo_effect);

  return cb;
}

DSP_callback
dsp_init_modulated_delay_flanger_default()
{
  audio_delay_params adp = {
    .alg = AD_NORMAL,
    .wet_mix = pow(10.0, -3.0 / 20.0),
    .dry_mix = pow(10.0, -3.0 / 20.0),
    .feedback = 0.50, // [-1, 1]
    .delay_ratio = 1.0
  };
  modulated_delay_params params = {
    .sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE,
    .alg = MD_FLANGER,
    .adp = adp,
    .lfo_freq = 0.2,
    .lfo_depth = 0.50,
    .min_delay_samps = 0.1 / 1000.0 * (FTYPE)DEFAULT_SAMPLE_RATE,
    .max_depth_samps = 7.0 / 1000.0 * (FTYPE)DEFAULT_SAMPLE_RATE,
  };
  DSP_callback cb = dsp_init_modulated_delay(params);

  return cb;
}

DSP_callback
dsp_init_modulated_delay_chorus_default()
{
  audio_delay_params adp = {
    .alg = AD_NORMAL,
    .wet_mix = pow(10.0, -3.0 / 20.0),
    .dry_mix = pow(10.0, 0.0 / 20.0),
    .feedback = 0.0,
    .delay_ratio = 1.0
  };
  modulated_delay_params params = {
    .sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE,
    .alg = MD_CHORUS,
    .adp = adp,
    .lfo_freq = 0.2,
    .lfo_depth = 0.50,
    .min_delay_samps = 10.0 / 1000.0 * (FTYPE)DEFAULT_SAMPLE_RATE,
    .max_depth_samps = 30.0 / 1000.0 * (FTYPE)DEFAULT_SAMPLE_RATE,
  };
  DSP_callback cb = dsp_init_modulated_delay(params);

  return cb;
}

DSP_callback
dsp_init_modulated_delay_vibrato_default()
{
  audio_delay_params ad = {
    .alg = AD_NORMAL,
    .wet_mix = pow(10.0, 0.0 / 20.0),
    .dry_mix = pow(10.0, -96.0 / 20.0),
    .feedback = 0.0,
    .delay_ratio = 1.0
  };
  modulated_delay_params params = {
    .sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE,
    .alg = MD_VIBRATO,
    .adp = ad,
    .lfo_freq = 0.2,
    .lfo_depth = 0.50,
    .min_delay_samps = 0.0 / 1000.0 * (FTYPE)DEFAULT_SAMPLE_RATE,
    .max_depth_samps = 7.0 / 1000.0 * (FTYPE)DEFAULT_SAMPLE_RATE,
  };
  DSP_callback cb = dsp_init_modulated_delay(params);

  return cb;
}

void
dsp_modulated_delay_cleanup(DSP_callback cb)
{
  ugen_cleanup(cb->state.modulated_delay.lfo);
  dsp_audio_delay_cleanup(cb->state.modulated_delay.ad);
  dsp_cleanup(cb);
}
