#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_filter.h"
#include "biquad.h"
#include "phase_shifter.h"
#include "fx.h"

extern FX_unit fx_unit_head;

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

void
fx_unit_phase_shifter_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define _apf_params fx_unit_head[idx].state.u.phase_shifter.apf_params
#define _apf_state(x) fx_unit_head[fx_unit_head[idx].state.u.phase_shifter.apf[(x)]].state
#define apf_biquad(x,y) fx_unit_head[fx_unit_head[idx].state.u.phase_shifter.apf[(x)]].state.u.audio_filter.biquad[(y)]
  FTYPE K = fx_unit_head[idx].state.u.phase_shifter.intensity;

  int i, j;
  FTYPE lfo_out[2];
  // UGEN_PHASE_NORM == 0, UGEN_PHASE_QUAD == 1
  lfo_out[0] = 0.5 * (1.0 + fx_unit_head[fx_unit_head[idx].state.u.phase_shifter.lfo].state.u.signal_source.ugen_triphase[0]);
  lfo_out[1] = 0.5 * (1.0 + fx_unit_head[fx_unit_head[idx].state.u.phase_shifter.lfo].state.u.signal_source.ugen_triphase[1]);

  // set fc for all 6 apfs
  for (i = 0; i < PHASER_STAGES; i++) {
    // 0 is minF, 1 is maxF, 0.5 should be halfway between
    // halfway in between should be minF + 0.5 * (maxF - minF)
    for (j = FX_L; j <= FX_R; j++) {
      _apf_params->u.audio_filter.fc[j] = apf_minF[i] + (apf_maxF[i] - apf_minF[i]) * lfo_out[j];
    }
    fx_unit_audio_filter_set_params(&_apf_state(i), _apf_params);
  }

  for (i = FX_L; i <= FX_R; i++) {
    // get G values from all apfs
    FTYPE gamma1 =          biquad_get_G(&(apf_biquad(5,i)));
    FTYPE gamma2 = gamma1 * biquad_get_G(&(apf_biquad(4,i)));
    FTYPE gamma3 = gamma2 * biquad_get_G(&(apf_biquad(3,i)));
    FTYPE gamma4 = gamma3 * biquad_get_G(&(apf_biquad(2,i)));
    FTYPE gamma5 = gamma4 * biquad_get_G(&(apf_biquad(1,i)));
    FTYPE gamma6 = gamma5 * biquad_get_G(&(apf_biquad(0,i)));

    FTYPE alpha0 = 1.0 / (1.0 + K * gamma6);

    FTYPE Sn = gamma5 * biquad_get_S(&(apf_biquad(0,i)))
             + gamma4 * biquad_get_S(&(apf_biquad(1,i)))
             + gamma3 * biquad_get_S(&(apf_biquad(2,i)))
             + gamma2 * biquad_get_S(&(apf_biquad(3,i)))
             + gamma1 * biquad_get_S(&(apf_biquad(4,i)))
             +          biquad_get_S(&(apf_biquad(5,i)));

    dst[i] = alpha0 * (src[i] - K * Sn);
  }
#undef apf_biquad
#undef _apf_state
#undef _apf_params
#undef src
#undef dst
}

void
fx_unit_phase_shifter_cleanup(FX_unit_state state)
{
  int i;
  for (i = 0; i < PHASER_STAGES; i++) {
    fx_unit_head[state->u.phase_shifter.apf[i]]
      .state.f.cleanup(
        &fx_unit_head[state->u.phase_shifter.apf[i]].state);

    state->u.phase_shifter.apf[i] = FX_UNIT_IDX_NONE;
  }

  fx_unit_head[state->u.phase_shifter.passthru]
    .state.f.cleanup(
      &fx_unit_head[state->u.phase_shifter.passthru].state);

  state->u.phase_shifter.passthru = FX_UNIT_IDX_NONE;

  fx_unit_head[state->u.phase_shifter.control_joiner]
    .state.f.cleanup(
      &fx_unit_head[state->u.phase_shifter.control_joiner].state);

  state->u.phase_shifter.control_joiner = FX_UNIT_IDX_NONE;

  fx_unit_head[state->u.phase_shifter.lfo]
    .state.f.cleanup(
      &fx_unit_head[state->u.phase_shifter.lfo].state);

  state->u.phase_shifter.lfo = FX_UNIT_IDX_NONE;

  fx_unit_head[state->u.phase_shifter.sum]
    .state.f.cleanup(
      &fx_unit_head[state->u.phase_shifter.sum].state);

  state->u.phase_shifter.sum = FX_UNIT_IDX_NONE;

  free(state->u.phase_shifter.apf_params);
}

void
fx_unit_phase_shifter_set_params(FX_unit_state state, FX_unit_params params)
{
  state->u.phase_shifter.intensity = params->u.phase_shifter.intensity;
}

void
fx_unit_phase_shifter_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_phase_shifter_set_params(state, params);

  // TODO reset dependents
}

fx_unit_idx
fx_unit_phase_shifter_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_phase_shifter_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_phase_shifter_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_phase_shifter_reset;

  // alloc params
  fx_unit_head[idx].state.u.phase_shifter.apf_params = calloc(1, sizeof(fx_unit_params));
  fx_unit_params p = fx_unit_audio_filter_default();
  p.u.audio_filter.alg = AF_APF1;
  memcpy(fx_unit_head[idx].state.u.phase_shifter.apf_params, &p, sizeof(fx_unit_params));

  fx_unit_phase_shifter_reset(&fx_unit_head[idx].state, params);
  return idx;
}

FX_compound_unit
fx_compound_unit_phase_shifter_init(FX_unit_params phase_shifter_p, FX_unit_params sig_p)
{
  // phase_shifter
  fx_unit_idx idx = fx_unit_phase_shifter_init(phase_shifter_p);

  // control joiner
  fx_unit_params cj_p = fx_unit_control_joiner_default();
  fx_unit_idx cj = fx_unit_control_joiner_init(&cj_p);

  // signal genenerator
  // control signal generator has no parent
  fx_unit_idx sg = fx_unit_signal_source_init(sig_p);

  // passthru
  fx_unit_params pt_p = fx_unit_passthru_default();
  fx_unit_idx pt = fx_unit_passthru_init(&pt_p);

  // summer
  fx_unit_params sum_p = fx_unit_sum_default();
  fx_unit_idx sum = fx_unit_sum_init(&sum_p);

  // 6 filters
  int i;
  for (i = 0; i < PHASER_STAGES; i++) {
    fx_unit_head[idx].state.u.phase_shifter.apf[i] =
      fx_unit_audio_filter_init(fx_unit_head[idx].state.u.phase_shifter.apf_params);
  }

  fx_unit_head[idx].state.u.phase_shifter.lfo = sg;
  fx_unit_head[idx].state.u.phase_shifter.control_joiner = cj;
  fx_unit_head[idx].state.u.phase_shifter.passthru = pt;
  fx_unit_head[idx].state.u.phase_shifter.sum = sum;

  FX_compound_unit rv = fx_compound_unit_init(11, 1);
  rv->units[0] = pt;
  rv->units[1] = cj;
  rv->units[2] = sg;
  rv->units[3] = idx;
  rv->units[4] = fx_unit_head[idx].state.u.phase_shifter.apf[0];
  rv->units[5] = fx_unit_head[idx].state.u.phase_shifter.apf[1];
  rv->units[6] = fx_unit_head[idx].state.u.phase_shifter.apf[2];
  rv->units[7] = fx_unit_head[idx].state.u.phase_shifter.apf[3];
  rv->units[8] = fx_unit_head[idx].state.u.phase_shifter.apf[4];
  rv->units[9] = fx_unit_head[idx].state.u.phase_shifter.apf[5];
  rv->units[10] = sum;

  // phase_shifter has control joiner as parent
  fx_unit_parent_ref_add(idx, cj);
  // control joiner has 2 parents
  fx_unit_parent_ref_add(cj, pt);
  fx_unit_parent_ref_add(cj, sg);

  // apf chain
  fx_unit_parent_ref_add(fx_unit_head[idx].state.u.phase_shifter.apf[0], idx);
  fx_unit_parent_ref_add(fx_unit_head[idx].state.u.phase_shifter.apf[1],
    fx_unit_head[idx].state.u.phase_shifter.apf[0]);
  fx_unit_parent_ref_add(fx_unit_head[idx].state.u.phase_shifter.apf[2],
    fx_unit_head[idx].state.u.phase_shifter.apf[1]);
  fx_unit_parent_ref_add(fx_unit_head[idx].state.u.phase_shifter.apf[3],
    fx_unit_head[idx].state.u.phase_shifter.apf[2]);
  fx_unit_parent_ref_add(fx_unit_head[idx].state.u.phase_shifter.apf[4],
    fx_unit_head[idx].state.u.phase_shifter.apf[3]);
  fx_unit_parent_ref_add(fx_unit_head[idx].state.u.phase_shifter.apf[5],
    fx_unit_head[idx].state.u.phase_shifter.apf[4]);
  fx_unit_parent_ref_add(sum, fx_unit_head[idx].state.u.phase_shifter.apf[5]);

  fx_unit_parent_ref_add(sum, idx);

  FTYPE *mix = calloc(2, sizeof(FTYPE));
  mix[0] = 0.707;
  mix[1] = 0.707;
  fx_unit_sum_mix_set(sum, mix);
  rv->heads[0] = pt;

  // sum is output for compound obj
  rv->tail = sum;

  return rv;
}

fx_unit_params
fx_unit_phase_shifter_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_PHASE_SHIFTER;
  params.u.phase_shifter.intensity = 0.85;

  return params;
}

fx_unit_params
fx_unit_phase_shifter_signal_source_default()
{
  fx_unit_params ss = {0};
  ss.sample_rate = DEFAULT_SAMPLE_RATE;
  ss.t = FX_UNIT_SIGNAL_SOURCE;
  ss.u.signal_source.t = FX_SIGNAL_UGEN;
  ss.u.signal_source.d = FX_SIGNAL_C;
  ss.u.signal_source.u.ugen = ugen_init_sin(0.2, DEFAULT_SAMPLE_RATE);
  FTYPE scale = 0.7;
  ugen_set_scale(ss.u.signal_source.u.ugen, -scale, scale);

  return ss;
}
