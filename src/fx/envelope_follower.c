#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_detector.h"
#include "audio_filter.h"
#include "envelope_follower.h"
#include "stereo2mono.h"
#include "fx.h"

extern FX_unit fx_unit_head;

void
fx_unit_envelope_follower_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[1]].output_buffer.lrc)
#define ad_control (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define _state (fx_unit_head[idx].state.u.envelope_follower)
  FTYPE thresh_val = _state.threshold;
  FTYPE detect_val = ad_control[FX_C];
  FTYPE delta_val = detect_val - thresh_val;

  if (delta_val <= 0.0) {
    _state.filter_params->u.audio_filter.fc[0] = _state.filter_params->u.audio_filter.fc[1] = _state.fc;
  } else {
    FTYPE modulator_val = delta_val * _state.sensitivity;
    _state.filter_params->u.audio_filter.fc[0] =  _state.filter_params->u.audio_filter.fc[1] = modulator_val * (_state.fc_max - _state.fc) + _state.fc;
  }
  dst[FX_L] = src[FX_L];
  dst[FX_R] = src[FX_R];
  fx_unit_audio_filter_set_params(&fx_unit_head[_state.filter].state, _state.filter_params);

#undef _state
#undef ad_control
#undef src
#undef dst
}

void
fx_unit_envelope_follower_cleanup(FX_unit_state state)
{
  // cleanup detector, filter, s2m
  fx_unit_head[state->u.envelope_follower.detector]
    .state.f.cleanup(
      &fx_unit_head[state->u.envelope_follower.detector].state);
  fx_unit_head[state->u.envelope_follower.filter]
    .state.f.cleanup(
      &fx_unit_head[state->u.envelope_follower.filter].state);
  fx_unit_head[state->u.envelope_follower.s2m]
    .state.f.cleanup(
      &fx_unit_head[state->u.envelope_follower.s2m].state);
  fx_unit_head[state->u.envelope_follower.passthru]
    .state.f.cleanup(
      &fx_unit_head[state->u.envelope_follower.passthru].state);

  state->u.envelope_follower.detector = FX_UNIT_IDX_NONE;
  state->u.envelope_follower.filter = FX_UNIT_IDX_NONE;
  state->u.envelope_follower.s2m = FX_UNIT_IDX_NONE;
  state->u.envelope_follower.passthru = FX_UNIT_IDX_NONE;

  free(state->u.envelope_follower.filter_params);
}

void
fx_unit_envelope_follower_set_params(FX_unit_state state, FX_unit_params params)
{
  state->u.envelope_follower.fc = params->u.envelope_follower.fc;
  state->u.envelope_follower.fc_max = params->u.envelope_follower.fc_max;
  state->u.envelope_follower.threshold = params->u.envelope_follower.threshold;
  state->u.envelope_follower.sensitivity = params->u.envelope_follower.sensitivity;
  state->u.envelope_follower.filter_params = params->u.envelope_follower.filter_params;
}

void
fx_unit_envelope_follower_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_envelope_follower_set_params(state, params);

  // TODO reset dependents
}

FX_compound_unit
fx_compound_unit_envelope_follower_init(
  FX_unit_params params,
  FX_unit_params filter_params,
  FX_unit_params detector_params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_envelope_follower_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_envelope_follower_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_envelope_follower_reset;

  params->u.envelope_follower.filter_params = calloc(1, sizeof(fx_unit_params));
  memcpy(params->u.envelope_follower.filter_params, filter_params, sizeof(fx_unit_params));

  fx_unit_envelope_follower_reset(&fx_unit_head[idx].state, params);

  fx_unit_params s2m_params = fx_unit_s2m_default();
  s2m_params.u.s2m.left = true;
  fx_unit_idx s2m = fx_unit_s2m_init(&s2m_params);
  fx_unit_idx ad = fx_unit_audio_detector_init(detector_params);
  fx_unit_idx af = fx_unit_audio_filter_init(filter_params);

  // passthru
  fx_unit_params pt_p = fx_unit_passthru_default();
  fx_unit_idx pt = fx_unit_passthru_init(&pt_p);

  fx_unit_head[idx].state.u.envelope_follower.s2m = s2m;
  fx_unit_head[idx].state.u.envelope_follower.detector = ad;
  fx_unit_head[idx].state.u.envelope_follower.filter = af;
  fx_unit_head[idx].state.u.envelope_follower.passthru = pt;

  // connect s2m to ad
  fx_unit_parent_ref_add(ad, s2m);

  // connect ad to envelope follower
  fx_unit_parent_ref_add(idx, ad);

  // connect envelope follower to af
  fx_unit_parent_ref_add(af, idx);

  // connect passthru to envelope follower
  fx_unit_parent_ref_add(idx, pt);

  FX_compound_unit rv = fx_compound_unit_init(5, 2);
  rv->units[0] = pt;
  rv->units[1] = s2m;
  rv->units[2] = idx;
  rv->units[3] = ad;
  rv->units[4] = af;

  rv->heads[0] = s2m;
  rv->heads[1] = pt;

  rv->tail = af;
  
  return rv;
}

fx_unit_params
fx_unit_envelope_follower_default()
{
  fx_unit_params env = {0};
  env.sample_rate = DEFAULT_SAMPLE_RATE;
  env.t = FX_UNIT_ENVELOPE_FOLLOWER;
  env.u.envelope_follower.fc = 200.0;
  env.u.envelope_follower.fc_max = ((unsigned long)DEFAULT_SAMPLE_RATE)>>1;
  env.u.envelope_follower.threshold = pow(10.0, -24.0 / 20.0); // -12 dB
  env.u.envelope_follower.sensitivity = 0.3;

  return env;
}

fx_unit_params
fx_unit_envelope_follower_audio_filter_default()
{
  // .fc is set in the sample processing function, per frame
  fx_unit_params af = fx_unit_audio_filter_default();
  af.u.audio_filter.q[0] = af.u.audio_filter.q[1] = 4.707;
  af.u.audio_filter.boost_cut_db[0] = af.u.audio_filter.boost_cut_db[1] = -9.0;
  af.u.audio_filter.alg = AF_LPF2;
  return af;
}

fx_unit_params
fx_unit_envelope_follower_audio_detector_default()
{
  fx_unit_params ad = fx_unit_audio_detector_default();
  ad.u.audio_detector.attack_time = 20;
  ad.u.audio_detector.release_time = 200.0;
  ad.u.audio_detector.detect_mode = AUDIO_DETECTOR_MODE_RMS;
  ad.u.audio_detector.detect_db = false;
  ad.u.audio_detector.clamp_to_unity_max = false;
  return ad;
}
