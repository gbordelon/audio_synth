#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "ringbuf.h"
#include "audio_delay.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 14
 */

void
fx_unit_audio_delay_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define state (fx_unit_head[idx].state.u.audio_delay)
  FTYPE ynL = ringbuf_read(state.bufs[0], state.delay_samps_l);
  FTYPE ynR = ringbuf_read(state.bufs[1], state.delay_samps_r);
  FTYPE dnL = src[FX_L] + state.feedback * ynL;
  FTYPE dnR = src[FX_R] + state.feedback * ynR;

  if (state.alg == AD_NORMAL) {
    ringbuf_write(state.bufs[0], dnL); 
    ringbuf_write(state.bufs[1], dnR); 
  } else if (state.alg == AD_PINGPONG) {
    ringbuf_write(state.bufs[0], dnR); 
    ringbuf_write(state.bufs[1], dnL); 
  }

  dst[FX_L] = state.dry_mix * src[FX_L] + state.wet_mix * ynL;
  dst[FX_R] = state.dry_mix * src[FX_R] + state.wet_mix * ynR;
  dst[FX_C] = src[FX_C];

#undef state
#undef src
#undef dst
}

void
fx_unit_audio_delay_cleanup(FX_unit_state state)
{
  ringbuf_cleanup(state->u.audio_delay.bufs[0]);
  ringbuf_cleanup(state->u.audio_delay.bufs[1]);
}

void
fx_unit_audio_delay_set_params(FX_unit_state state, FX_unit_params params)
{
#define state2 (state->u.audio_delay)
#define params2 (params->u.audio_delay)
  Ringbuf rb;

  rb = state2.bufs[0];
  if (!rb || state2.delay_samps_l != params2.delay_samps_l) {
    if (rb) {
      ringbuf_resize(rb, params2.delay_samps_l * 2);
    } else {
      rb = ringbuf_init(params2.delay_samps_l * 2); // 2 secs of buffer
      state2.bufs[0] = rb;
    }
  }

  rb = state2.bufs[1];
  if (!rb || state2.delay_samps_r != params2.delay_samps_r) {
    if (rb) {
      ringbuf_resize(rb, params2.delay_samps_r * 2);
    } else {
      rb = ringbuf_init(params2.delay_samps_r * 2); // 2 secs of buffer
      state2.bufs[1] = rb;
    }
  }

/*
  if (params2.sample_rate < DEFAULT_SAMPLE_RATE) {
    params2.sample_rate = DEFAULT_SAMPLE_RATE;
  }
*/
  state2.alg = params2.alg;
  state2.update_type = params2.update_type;
  state2.wet_mix = params2.wet_mix;
  state2.dry_mix = params2.dry_mix;
  state2.feedback = params2.feedback;
  state2.delay_samps_l = params2.delay_samps_l;
  state2.delay_samps_r = params2.delay_samps_r;
  state2.delay_ratio = params2.delay_ratio;

#undef params2
#undef state2
}

void
fx_unit_audio_delay_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_audio_delay_set_params(state, params);

  ringbuf_reset(state->u.audio_delay.bufs[0]);
  ringbuf_reset(state->u.audio_delay.bufs[1]);
}

fx_unit_idx
fx_unit_audio_delay_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_audio_delay_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_audio_delay_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_audio_delay_reset;

  fx_unit_audio_delay_reset(&fx_unit_head[idx].state, params);
  return idx;
}

fx_unit_params
fx_unit_audio_delay_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_AUDIO_DELAY;
  params.u.audio_delay.alg = AD_PINGPONG;
  params.u.audio_delay.update_type = AD_LEFT_AND_RIGHT;
  params.u.audio_delay.wet_mix = pow(10.0, -12.0 / 20.0);
  params.u.audio_delay.dry_mix = pow(10.0, -3.0 / 20.0);
  params.u.audio_delay.feedback = 0.2;
  params.u.audio_delay.delay_samps_l = (1000.0 / 1000.0) * DEFAULT_SAMPLE_RATE;
  params.u.audio_delay.delay_samps_r = (1000.0 / 1000.0) * DEFAULT_SAMPLE_RATE;
  params.u.audio_delay.delay_ratio = 1.0;
  return params;
}
