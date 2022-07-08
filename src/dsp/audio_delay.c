#include <math.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_delay.h"
#include "dsp.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Prikle, chapter 14
 */

FTYPE
stereo_delay(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  double ynL = ringbuf_read(state->audio_delay.bufs[0], state->audio_delay.delay_samps_l);
  double ynR = ringbuf_read(state->audio_delay.bufs[1], state->audio_delay.delay_samps_r);
  double dnL = *L + state->audio_delay.feedback * ynL;
  double dnR = *R + state->audio_delay.feedback * ynR;

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

// TODO resizable bufs
void
dsp_audio_delay_set_params(
    dsp_state *state,
    audio_delay_params params)
{
  Ringbuf rb0, rb1;
  if (state->audio_delay.bufs[0]) {
    rb0 = state->audio_delay.bufs[0];
  } else {
    rb0 = ringbuf_init_default(); // 2 secs of buffer
  }
  if (state->audio_delay.bufs[1]) {
    rb1 = state->audio_delay.bufs[1];
  } else {
    rb1 = ringbuf_init_default(); // 2 secs of buffer
  }
  state->audio_delay = params;
  state->audio_delay.bufs[0] = rb0;
  state->audio_delay.bufs[1] = rb1;
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
    .alg = AD_NORMAL,
    .update_type = AD_LEFT_AND_RIGHT,
    .wet_mix = pow(10.0, -3.0 / 20.0),
    .dry_mix = pow(10.0, -3.0 / 20.0),
    .feedback = 0.4,
    .delay_samps_l = (250.0 / 1000.0) * (FTYPE)DEFAULT_SAMPLE_RATE,
    .delay_samps_r = (250.0 / 1000.0) * (FTYPE)DEFAULT_SAMPLE_RATE,
    .delay_ratio = 1.0
  };
  DSP_callback cb = dsp_init_audio_delay(params);

  return cb;
}

void
dsp_audio_delay_cleanup(DSP_callback cb)
{
  ringbuf_cleanup(cb->state.audio_delay.bufs[0]);
  ringbuf_cleanup(cb->state.audio_delay.bufs[1]);
  dsp_cleanup(cb);
}
