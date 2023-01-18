#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_filter.h"
#include "two_band_shelving_filter.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */
void
fx_unit_two_band_shelving_filter_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
  memcpy(dst, src, 3 * sizeof(FTYPE));
//printf("%f %f %f\n", src[0], src[1], src[2]);
#undef src
#undef dst
}

void
fx_unit_two_band_shelving_filter_cleanup(FX_unit_state state)
{
  // cleanup both filters
  fx_unit_head[state->u.two_band_shelving_filter.low_apf]
    .state.f.cleanup(
      &fx_unit_head[state->u.two_band_shelving_filter.low_apf].state);
  fx_unit_head[state->u.two_band_shelving_filter.high_apf]
    .state.f.cleanup(
      &fx_unit_head[state->u.two_band_shelving_filter.high_apf].state);

  state->u.two_band_shelving_filter.low_apf = FX_UNIT_IDX_NONE;
  state->u.two_band_shelving_filter.high_apf = FX_UNIT_IDX_NONE;
}

void
fx_unit_two_band_shelving_filter_set_params(FX_unit_state state, FX_unit_params params)
{
  state->u.two_band_shelving_filter.low_shelf_fc = params->u.two_band_shelving_filter.low_shelf_fc;
  state->u.two_band_shelving_filter.low_shelf_boost_cut_db = params->u.two_band_shelving_filter.low_shelf_boost_cut_db;
  state->u.two_band_shelving_filter.high_shelf_fc = params->u.two_band_shelving_filter.high_shelf_fc;
  state->u.two_band_shelving_filter.high_shelf_boost_cut_db = params->u.two_band_shelving_filter.high_shelf_boost_cut_db;
}

void
fx_unit_two_band_shelving_filter_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_two_band_shelving_filter_set_params(state, params);

  // TODO reset dependents
/*
  fx_unit_head[state->u.two_band_shelving_filter.low_apf]
    .state.f.reset(
      &fx_unit_head[state->u.two_band_shelving_filter.low_apf].state, );
  fx_unit_head[state->u.two_band_shelving_filter.high_apf]
    .state.f.reset(
      &fx_unit_head[state->u.two_band_shelving_filter.high_apf].state, );
*/
}

fx_unit_idx
fx_unit_two_band_shelving_filter_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_two_band_shelving_filter_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_two_band_shelving_filter_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_two_band_shelving_filter_reset;

  fx_unit_two_band_shelving_filter_reset(&fx_unit_head[idx].state, params);
  return idx;
}

FX_compound_unit
fx_compound_unit_two_band_shelving_filter_init(FX_unit_params params)
{
  // two_band_shelving_filterner
  fx_unit_idx idx = fx_unit_two_band_shelving_filter_init(params);

  // lpf
  fx_unit_params lpf_p = fx_unit_audio_filter_default();
  lpf_p.u.audio_filter.alg = AF_LowShelf;
  lpf_p.u.audio_filter.q[0] = lpf_p.u.audio_filter.q[1] = 0.707; // -3dB
  lpf_p.u.audio_filter.fc[0] = lpf_p.u.audio_filter.fc[1] = params->u.two_band_shelving_filter.low_shelf_fc;
  lpf_p.u.audio_filter.boost_cut_db[0] = lpf_p.u.audio_filter.boost_cut_db[1] = params->u.two_band_shelving_filter.low_shelf_boost_cut_db;
  fx_unit_idx lpf = fx_unit_audio_filter_init(&lpf_p);

  // hpf
  fx_unit_params hpf_p = fx_unit_audio_filter_default();
  hpf_p.u.audio_filter.alg = AF_HiShelf;
  hpf_p.u.audio_filter.q[0] = hpf_p.u.audio_filter.q[1] = 0.707; // -3dB
  hpf_p.u.audio_filter.fc[0] = hpf_p.u.audio_filter.fc[1] = params->u.two_band_shelving_filter.high_shelf_fc;
  hpf_p.u.audio_filter.boost_cut_db[0] = hpf_p.u.audio_filter.boost_cut_db[1] = params->u.two_band_shelving_filter.high_shelf_boost_cut_db;
  fx_unit_idx hpf = fx_unit_audio_filter_init(&hpf_p);

  fx_unit_head[idx].state.u.two_band_shelving_filter.low_apf = lpf;
  fx_unit_head[idx].state.u.two_band_shelving_filter.high_apf = hpf;

  FX_compound_unit rv = fx_compound_unit_init(3, 1);
  rv->units[0] = idx;
  rv->units[1] = lpf;
  rv->units[2] = hpf;

  fx_unit_parent_ref_add(hpf, lpf);
  fx_unit_parent_ref_add(lpf, idx);

  rv->heads[0] = idx;

  rv->tail = hpf;

  return rv;
}

fx_unit_params
fx_unit_two_band_shelving_filter_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_TWO_BAND_SHELVING_FILTER;
  params.u.two_band_shelving_filter.low_shelf_fc = 400.0;
  params.u.two_band_shelving_filter.low_shelf_boost_cut_db = 3.0;
  params.u.two_band_shelving_filter.high_shelf_fc = 4000.0;
  params.u.two_band_shelving_filter.high_shelf_boost_cut_db = -3.0;

  return params;
}
