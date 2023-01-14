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
    _state.filter_params->u.audio_filter.fc = _state.fc;
  } else {
    FTYPE modulator_val = delta_val * _state.sensitivity;
    _state.filter_params->u.audio_filter.fc = modulator_val * (_state.fc_max - _state.fc) + _state.fc;
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

  state->u.envelope_follower.detector = FX_UNIT_IDX_NONE;
  state->u.envelope_follower.filter = FX_UNIT_IDX_NONE;
  state->u.envelope_follower.s2m = FX_UNIT_IDX_NONE;

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
}

fx_unit_idx
fx_unit_envelope_follower_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_envelope_follower_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_envelope_follower_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_envelope_follower_reset;

  fx_unit_envelope_follower_reset(&fx_unit_head[idx].state, params);

  fx_unit_idx s2m = fx_unit_s2m_init(params->u.envelope_follower.s2m_params);
  fx_unit_idx ad = fx_unit_audio_detector_init(params->u.envelope_follower.detector_params);
  fx_unit_idx af = fx_unit_audio_filter_init(params->u.envelope_follower.filter_params);

  fx_unit_head[idx].state.u.envelope_follower.s2m = s2m;
  fx_unit_head[idx].state.u.envelope_follower.detector = ad;
  fx_unit_head[idx].state.u.envelope_follower.filter = af;

  // connect s2m to ad
  fx_unit_add_parent_ref(ad, s2m);

  // connect ad to envelope follower
  fx_unit_add_parent_ref(idx, ad);

  // connect envelope follower to af
  fx_unit_add_parent_ref(af, idx);

  return idx;
}

fx_unit_params
fx_unit_envelope_follower_default()
{
  fx_unit_params env = {0}, af, ad, s2m;
  env.sample_rate = DEFAULT_SAMPLE_RATE;
  env.t = FX_UNIT_ENVELOPE_FOLLOWER;
  env.u.envelope_follower.fc = 200.0;
  env.u.envelope_follower.fc_max = ((unsigned long)DEFAULT_SAMPLE_RATE)>>1;
  env.u.envelope_follower.threshold = pow(10.0, -6.0 / 20.0); // -12 dB
  env.u.envelope_follower.sensitivity = 0.3;

  // .fc is set in the sample processing function, per frame
  af = fx_unit_audio_filter_default();
  af.u.audio_filter.q = 4.707;
  af.u.audio_filter.boost_cut_db = -9.0;
  af.u.audio_filter.alg = AF_LPF2;
  env.u.envelope_follower.filter_params = calloc(1, sizeof(fx_unit_params));
  memcpy(env.u.envelope_follower.filter_params, &af, sizeof(fx_unit_params));

  ad = fx_unit_audio_detector_default();
  ad.u.audio_detector.attack_time = 20;
  ad.u.audio_detector.release_time = 200.0;
  ad.u.audio_detector.detect_mode = AUDIO_DETECTOR_MODE_RMS;
  ad.u.audio_detector.detect_db = false;
  ad.u.audio_detector.clamp_to_unity_max = false;
  env.u.envelope_follower.detector_params = calloc(1, sizeof(fx_unit_params));
  memcpy(env.u.envelope_follower.detector_params, &ad, sizeof(fx_unit_params));

  s2m = fx_unit_s2m_default();
  s2m.u.s2m.left = true;
  env.u.envelope_follower.s2m_params = calloc(1, sizeof(fx_unit_params));
  memcpy(env.u.envelope_follower.s2m_params, &s2m, sizeof(fx_unit_params));

  return env;
}

void
fx_unit_envelope_follower_params_free(FX_unit_params env)
{
  free(env->u.envelope_follower.detector_params);
  //free(env->u.envelope_follower.filter_params); // copied these into the state
  free(env->u.envelope_follower.s2m_params);
}

fx_unit_idx
fx_unit_envelope_follower_set_parent(fx_unit_idx env, fx_unit_idx parent)
{
  // get idx for s2m and audio_filter
#define s1 fx_unit_head[env].state.u.envelope_follower

  // set parent for both idx and s2m
  fx_unit_add_parent_ref(env, parent);
  fx_unit_add_parent_ref(s1.s2m, parent);

  // return idx for audio_filter
  return s1.filter;
#undef s1
}
