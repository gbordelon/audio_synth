#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "ringbuf.h"
#include "comb_filter.h"
#include "simple_lpf.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 * Based on the description from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */
void
fx_unit_comb_filter_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define state (fx_unit_head[idx].state.u.comb_filter)
  FTYPE frac, yn2, yn1;
  int i;
  for (i = 0; i < 2; i++) {
    yn1 = ringbuf_read(state.bufs[i], state.delay_samps);

    frac = 0.5;

    if (state.enable_lpf) {
      yn1 = simple_lpf_process(state.lpf, yn1);
    }

    ringbuf_write(state.bufs[i], yn1 * state.g + src[i]);

    if (!state.interpolate) {
      dst[i] = yn1;
      continue;
    }

    yn2 = ringbuf_read(state.bufs[i], state.delay_samps + 1);
    if (state.enable_lpf) {
      yn2 = simple_lpf_process(state.lpf, yn2);
    }

    dst[i] = (1.0 - frac) * yn1 + (frac) * yn2;
  }
  dst[FX_C] = src[FX_C]; // pass thru

#undef state
#undef src
#undef dst
}

void
fx_unit_comb_filter_cleanup(FX_unit_state state)
{
  ringbuf_cleanup(state->u.comb_filter.bufs[FX_L]);
  ringbuf_cleanup(state->u.comb_filter.bufs[FX_R]);
  simple_lpf_cleanup(state->u.comb_filter.lpf);
}

// TODO resizeable buffer
void
fx_unit_comb_filter_set_params(FX_unit_state state, FX_unit_params params)
{
#define state2 (state->u.comb_filter)
#define params2 (params->u.comb_filter)
  state2.delay_ms = params2.delay_ms;
  state2.delay_samps = (uint32_t)params->sample_rate * params2.delay_ms / 1000.0;
  state2.rt60_ms = params2.rt60_ms;
  state2.rt60_samps = (uint32_t)params->sample_rate * params2.rt60_ms / 1000.0;

  state2.g = pow(10.0, -3.0 * state2.delay_ms / state2.rt60_ms);

  state2.buf_len_samps = state2.delay_samps;
  state2.buf_len_ms = state2.delay_ms;
  state2.bufs[0] = ringbuf_init(state2.buf_len_samps);
  state2.bufs[1] = ringbuf_init(state2.buf_len_samps);

  state2.lpf = simple_lpf_init(params2.lpf_g);

#undef params2
#undef state2
}

void
fx_unit_comb_filter_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_comb_filter_set_params(state, params);

  ringbuf_reset(state->u.comb_filter.bufs[0]);
  ringbuf_reset(state->u.comb_filter.bufs[1]);
}

fx_unit_idx
fx_unit_comb_filter_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_comb_filter_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_comb_filter_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_comb_filter_reset;

  fx_unit_comb_filter_reset(&fx_unit_head[idx].state, params);
  return idx;
}

FX_compound_unit
fx_compound_unit_comb_filter_init(FX_unit_params params)
{
  FX_compound_unit rv = fx_compound_unit_init(1, 1);
  rv->tail = rv->units[0] = rv->heads[0] = fx_unit_comb_filter_init(params);
  return rv;
}

fx_unit_params
fx_unit_comb_filter_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_COMB_FILTER;
  params.u.comb_filter.delay_ms = 10.0;
  params.u.comb_filter.rt60_ms = 500.0;
  params.u.comb_filter.lpf_g = 0.8;
  params.u.comb_filter.enable_lpf = true;
  params.u.comb_filter.interpolate = true;

  return params;
}
