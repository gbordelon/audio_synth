#include <math.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_delay.h"
#include "dsp.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 14
 */

FTYPE
stereo_delay(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  FTYPE ynL = ringbuf_read(state->audio_delay.bufs[0], state->audio_delay.delay_samps_l);
  FTYPE ynR = ringbuf_read(state->audio_delay.bufs[1], state->audio_delay.delay_samps_r);
  FTYPE dnL = *L + state->audio_delay.feedback * ynL;
  FTYPE dnR = *R + state->audio_delay.feedback * ynR;

  if (state->audio_delay.alg == AD_NORMAL) {
    ringbuf_write(state->audio_delay.bufs[0], dnL); 
    ringbuf_write(state->audio_delay.bufs[1], dnR); 
  } else if (state->audio_delay.alg == AD_PINGPONG) {
    ringbuf_write(state->audio_delay.bufs[0], dnR); 
    ringbuf_write(state->audio_delay.bufs[1], dnL); 
  }

  *L = state->audio_delay.dry_mix * *L + state->audio_delay.wet_mix * ynL;
  *R = state->audio_delay.dry_mix * *R + state->audio_delay.wet_mix * ynR;

  return control;
}

void
dsp_audio_delay_set_params(
    dsp_state *state,
    audio_delay_params params)
{
  Ringbuf rb;

  rb = state->audio_delay.bufs[0];
  if (!rb || state->audio_delay.delay_samps_l != params.delay_samps_l) {
    if (rb) {
      ringbuf_resize(rb, params.delay_samps_l * 2);
    } else {
      rb = ringbuf_init(params.delay_samps_l * 2); // 2 secs of buffer
      state->audio_delay.bufs[0] = rb;
    }
  }

  rb = state->audio_delay.bufs[1];
  if (!rb || state->audio_delay.delay_samps_r != params.delay_samps_r) {
    if (rb) {
      ringbuf_resize(rb, params.delay_samps_r * 2);
    } else {
      rb = ringbuf_init(params.delay_samps_r * 2); // 2 secs of buffer
      state->audio_delay.bufs[1] = rb;
    }
  }

  if (params.sample_rate < DEFAULT_SAMPLE_RATE) {
    params.sample_rate = DEFAULT_SAMPLE_RATE;
  }
  params.bufs[0] = state->audio_delay.bufs[0];
  params.bufs[1] = state->audio_delay.bufs[1];
  state->audio_delay = params;
}

DSP_callback
dsp_init_audio_delay(audio_delay_params params)
{
  DSP_callback cb = dsp_init();
  dsp_audio_delay_set_params(&cb->state, params);
  dsp_set_stereo(cb, stereo_delay);

  return cb;
}

DSP_callback
dsp_init_audio_delay_default()
{
  audio_delay_params params = {
    .sample_rate = DEFAULT_SAMPLE_RATE,
    .alg = AD_PINGPONG,
    .update_type = AD_LEFT_AND_RIGHT,
    .wet_mix = pow(10.0, -24.0 / 20.0),
    .dry_mix = pow(10.0, -3.0 / 20.0),
    .feedback = 0.2,
    .delay_samps_l = (1000.0 / 1000.0) * DEFAULT_SAMPLE_RATE,
    .delay_samps_r = (1000.0 / 1000.0) * DEFAULT_SAMPLE_RATE,
    .delay_ratio = 1.0
  };
  DSP_callback cb = dsp_init_audio_delay(params);

  return cb;
}

void
dsp_audio_delay_reset(DSP_callback cb)
{
  ringbuf_reset(cb->state.audio_delay.bufs[0]);
  ringbuf_reset(cb->state.audio_delay.bufs[1]);
}

void
dsp_audio_delay_cleanup(DSP_callback cb)
{
  ringbuf_cleanup(cb->state.audio_delay.bufs[0]);
  ringbuf_cleanup(cb->state.audio_delay.bufs[1]);
  dsp_cleanup(cb);
}
