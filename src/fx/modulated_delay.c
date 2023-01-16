#include <math.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_delay.h"
#include "modulated_delay.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 15
 */

void
fx_unit_modulated_delay_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define _state (fx_unit_head[idx].state.u.modulated_delay)
#define _params (fx_unit_head[idx].state.u.modulated_delay.ad_params->u.audio_delay)
#define triphase (fx_unit_head[fx_unit_head[idx].state.u.modulated_delay.signal_source].state.u.signal_source.ugen_triphase)

  FTYPE depth = _state.lfo_depth;
  FTYPE mod_min = _state.min_delay_samps; 
  FTYPE mod_max = _state.min_delay_samps + _state.max_depth_samps;
  FTYPE mod_val;

  if (_state.alg == MD_FLANGER) {
    mod_val = bipolar_to_unipolar(depth * triphase[UGEN_PHASE_NORM]);
    _params.delay_samps_l = unipolar_modulation_from_min(mod_val, mod_min, mod_max);
    mod_val = bipolar_to_unipolar(depth * triphase[UGEN_PHASE_INV]);
    _params.delay_samps_r = unipolar_modulation_from_min(mod_val, mod_min, mod_max);
  } else if (_state.alg == MD_VIBRATO) {
    mod_val = depth * triphase[UGEN_PHASE_NORM];
    _params.delay_samps_l = bipolar_modulation(mod_val, mod_min, mod_max);
    _params.delay_samps_r = bipolar_modulation(mod_val, mod_min, mod_max);
  } else {
    mod_val = depth * triphase[UGEN_PHASE_NORM];
    _params.delay_samps_l = bipolar_modulation(mod_val, mod_min, mod_max);
    mod_val = depth * triphase[UGEN_PHASE_INV];
    _params.delay_samps_r = bipolar_modulation(mod_val, mod_min, mod_max);
  }
  fx_unit_audio_delay_set_params(&fx_unit_head[_state.audio_delay].state, _state.ad_params);

  memcpy(dst, src, 3 * sizeof(FTYPE));

#undef triphase
#undef _params
#undef _state
#undef src
#undef dst
}

void
fx_unit_modulated_delay_cleanup(FX_unit_state state)
{
  // cleanup delay, signal source, control joiner, passthru
  fx_unit_head[state->u.modulated_delay.audio_delay]
    .state.f.cleanup(
      &fx_unit_head[state->u.modulated_delay.audio_delay].state);
  fx_unit_head[state->u.modulated_delay.signal_source]
    .state.f.cleanup(
      &fx_unit_head[state->u.modulated_delay.signal_source].state);
  fx_unit_head[state->u.modulated_delay.control_joiner]
    .state.f.cleanup(
      &fx_unit_head[state->u.modulated_delay.control_joiner].state);
  fx_unit_head[state->u.modulated_delay.passthru]
    .state.f.cleanup(
      &fx_unit_head[state->u.modulated_delay.passthru].state);

  state->u.modulated_delay.audio_delay = FX_UNIT_IDX_NONE;
  state->u.modulated_delay.signal_source = FX_UNIT_IDX_NONE;
  state->u.modulated_delay.control_joiner = FX_UNIT_IDX_NONE;
  state->u.modulated_delay.passthru = FX_UNIT_IDX_NONE;

  free(state->u.modulated_delay.ad_params);
}

void
fx_unit_modulated_delay_set_params(FX_unit_state state, FX_unit_params params)
{
  state->u.modulated_delay.alg = params->u.modulated_delay.alg;
  state->u.modulated_delay.lfo_depth = params->u.modulated_delay.lfo_depth;
  state->u.modulated_delay.min_delay_samps = params->u.modulated_delay.min_delay_ms * state->sample_rate / 1000.0;
  state->u.modulated_delay.max_depth_samps = params->u.modulated_delay.max_depth_ms * state->sample_rate / 1000.0;
  state->u.modulated_delay.ad_params = params->u.modulated_delay.ad_params;
}

void
fx_unit_modulated_delay_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_modulated_delay_set_params(state, params);

  // TODO reset dependents
}

FX_compound_unit
fx_compound_unit_modulated_delay_init(
  FX_unit_params params,
  FX_unit_params audio_delay_params,
  FX_unit_params signal_source_params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_modulated_delay_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_modulated_delay_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_modulated_delay_reset;

  params->u.modulated_delay.ad_params = calloc(1, sizeof(fx_unit_params));
  memcpy(params->u.modulated_delay.ad_params, audio_delay_params, sizeof(fx_unit_params));

  fx_unit_modulated_delay_reset(&fx_unit_head[idx].state, params);

  fx_unit_idx ss = fx_unit_signal_source_init(signal_source_params);
  fx_unit_idx ad = fx_unit_audio_delay_init(audio_delay_params);

  // control_joiner
  fx_unit_params cj_p = fx_unit_control_joiner_default();
  fx_unit_idx cj = fx_unit_control_joiner_init(&cj_p);

  // passthru
  fx_unit_params pt_p = fx_unit_passthru_default();
  fx_unit_idx pt = fx_unit_passthru_init(&pt_p);

  fx_unit_head[idx].state.u.modulated_delay.signal_source = ss;
  fx_unit_head[idx].state.u.modulated_delay.audio_delay = ad;
  fx_unit_head[idx].state.u.modulated_delay.control_joiner = cj;
  fx_unit_head[idx].state.u.modulated_delay.passthru = pt;

  // connect ad to md
  fx_unit_parent_ref_add(ad, idx);

  // connect cj to md
  fx_unit_parent_ref_add(idx, cj);

  // connect pt to cj
  fx_unit_parent_ref_add(cj, pt);

  // connect ss to cj
  fx_unit_parent_ref_add(cj, ss);

  FX_compound_unit rv = fx_compound_unit_init(5, 1);
  rv->units[0] = pt;
  rv->units[1] = cj;
  rv->units[2] = ss;
  rv->units[3] = idx;
  rv->units[4] = ad;

  rv->heads[0] = pt;

  rv->tail = ad;
  
  return rv;
}

fx_unit_params
fx_unit_modulated_delay_chorus_default()
{
  fx_unit_params md = {0};
  md.sample_rate = DEFAULT_SAMPLE_RATE;
  md.t = FX_UNIT_MODULATED_DELAY;
  md.u.modulated_delay.alg = MD_CHORUS;
  md.u.modulated_delay.lfo_depth = 0.50;
  md.u.modulated_delay.min_delay_ms = 10.0;
  md.u.modulated_delay.max_depth_ms = 30.0;
  
  return md;
}

fx_unit_params
fx_unit_modulated_delay_audio_delay_chorus_default()
{
  fx_unit_params ad = {0};
  ad.sample_rate = DEFAULT_SAMPLE_RATE;
  ad.t = FX_UNIT_AUDIO_DELAY;
  ad.u.audio_delay.alg = AD_NORMAL;
  ad.u.audio_delay.wet_mix = pow(10.0, -3.0 / 20.0);
  ad.u.audio_delay.dry_mix = pow(10.0, 0.0 / 20.0);
  ad.u.audio_delay.feedback = 0.0;
  ad.u.audio_delay.delay_ratio = 1.0;
  // delay_samps gets set by the modulated delay

  return ad;
}

fx_unit_params
fx_unit_modulated_delay_signal_source_chorus_default()
{
  fx_unit_params ss = {0};
  ss.sample_rate = DEFAULT_SAMPLE_RATE;
  ss.t = FX_UNIT_SIGNAL_SOURCE;
  ss.u.signal_source.t = FX_SIGNAL_UGEN;
  ss.u.signal_source.d = FX_SIGNAL_C;
  ss.u.signal_source.u.ugen = ugen_init_tri(0.2, DEFAULT_SAMPLE_RATE);
  ugen_set_cr(ss.u.signal_source.u.ugen);
  
  return ss;
}

fx_unit_params
fx_unit_modulated_delay_flanger_default()
{
  fx_unit_params md = {0};
  md.sample_rate = DEFAULT_SAMPLE_RATE;
  md.t = FX_UNIT_MODULATED_DELAY;
  md.u.modulated_delay.alg = MD_FLANGER;
  md.u.modulated_delay.lfo_depth = 0.50;
  md.u.modulated_delay.min_delay_ms = 0.1;
  md.u.modulated_delay.max_depth_ms = 7.0;
  
  return md;
}

fx_unit_params
fx_unit_modulated_delay_audio_delay_flanger_default()
{
  fx_unit_params ad = {0};
  ad.sample_rate = DEFAULT_SAMPLE_RATE;
  ad.t = FX_UNIT_AUDIO_DELAY;
  ad.u.audio_delay.alg = AD_NORMAL;
  ad.u.audio_delay.wet_mix = pow(10.0, -3.0 / 20.0);
  ad.u.audio_delay.dry_mix = pow(10.0, -3.0 / 20.0);
  ad.u.audio_delay.feedback = 0.50; // [-1, 1]
  ad.u.audio_delay.delay_ratio = 1.0;
  // delay_samps gets set by the modulated delay

  return ad;
}

fx_unit_params
fx_unit_modulated_delay_signal_source_flanger_default()
{
  fx_unit_params ss = {0};
  ss.sample_rate = DEFAULT_SAMPLE_RATE;
  ss.t = FX_UNIT_SIGNAL_SOURCE;
  ss.u.signal_source.t = FX_SIGNAL_UGEN;
  ss.u.signal_source.d = FX_SIGNAL_C;
  ss.u.signal_source.u.ugen = ugen_init_tri(0.2, DEFAULT_SAMPLE_RATE);
  ugen_set_cr(ss.u.signal_source.u.ugen);
  
  return ss;
}

fx_unit_params
fx_unit_modulated_delay_vibrato_default()
{
  fx_unit_params md = {0};
  md.sample_rate = DEFAULT_SAMPLE_RATE;
  md.t = FX_UNIT_MODULATED_DELAY;
  md.u.modulated_delay.alg = MD_VIBRATO;
  md.u.modulated_delay.lfo_depth = 0.02;
  md.u.modulated_delay.min_delay_ms = 0.0;
  md.u.modulated_delay.max_depth_ms = 70.0;
  
  return md;
}

fx_unit_params
fx_unit_modulated_delay_audio_delay_vibrato_default()
{
  fx_unit_params ad = {0};
  ad.sample_rate = DEFAULT_SAMPLE_RATE;
  ad.t = FX_UNIT_AUDIO_DELAY;
  ad.u.audio_delay.alg = AD_NORMAL;
  ad.u.audio_delay.wet_mix = pow(10.0, 0.0 / 20.0);
  ad.u.audio_delay.dry_mix = pow(10.0, -96.0 / 20.0);
  ad.u.audio_delay.feedback = 0.0;
  ad.u.audio_delay.delay_ratio = 1.0;
  // delay_samps gets set by the modulated delay

  return ad;
}

fx_unit_params
fx_unit_modulated_delay_signal_source_vibrato_default()
{
  fx_unit_params ss = {0};
  ss.sample_rate = DEFAULT_SAMPLE_RATE;
  ss.t = FX_UNIT_SIGNAL_SOURCE;
  ss.u.signal_source.t = FX_SIGNAL_UGEN;
  ss.u.signal_source.d = FX_SIGNAL_C;
  ss.u.signal_source.u.ugen = ugen_init_sin(0.2, DEFAULT_SAMPLE_RATE);
  ugen_set_cr(ss.u.signal_source.u.ugen);
  
  return ss;
}

fx_unit_params
fx_unit_modulated_delay_default()
{
  return fx_unit_modulated_delay_chorus_default();
}

fx_unit_params
fx_unit_modulated_delay_audio_delay_default()
{
  return fx_unit_modulated_delay_audio_delay_chorus_default();
}

fx_unit_params
fx_unit_modulated_delay_signal_source_default()
{
  return fx_unit_modulated_delay_signal_source_chorus_default();
}
