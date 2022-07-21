#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "comb_filter.h"
#include "ringbuf.h"
#include "simple_lpf.h"

/*
 * Based on the description from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */
Comb_filter
comb_filter_alloc()
{
  return calloc(1, sizeof(struct comb_filter_t));
}

// TODO resizeable buffer
// TODO accept ms instead of samples so a useful fraction can be calculated for interpolation
Comb_filter
comb_filter_init(uint32_t delay_samps, uint32_t rt60_samps, FTYPE lpf_g, FTYPE sample_rate)
{
  Comb_filter cf = comb_filter_alloc();
  cf->delay_samps = delay_samps;
  cf->rt60_samps = rt60_samps;
  cf->delay_ms = 1000.0 * ((FTYPE)delay_samps) / sample_rate;
  cf->rt60_ms = 1000.0 * ((FTYPE)rt60_samps) / sample_rate;

  cf->g = pow(10.0, -3.0 * cf->delay_ms / cf->rt60_ms);

  cf->buf_len_samps = cf->delay_samps;
  cf->buf_len_ms = cf->delay_ms;
  cf->buf = ringbuf_init(cf->buf_len_samps);

  cf->lpf = simple_lpf_init(lpf_g);

  cf->interpolate = false;
  cf->enable_lpf = true;

  return cf;
}

void
comb_filter_free(Comb_filter cf)
{
  free(cf);
}

void
comb_filter_cleanup(Comb_filter cf)
{
  ringbuf_cleanup(cf->buf);
  simple_lpf_cleanup(cf->lpf);
  comb_filter_free(cf);
}

FTYPE
comb_filter_process(Comb_filter cf, FTYPE xn)
{
  FTYPE frac, yn2, yn1 = ringbuf_read(cf->buf, cf->delay_samps);
  frac = 0.5;

  if (cf->enable_lpf) {
    yn1 = simple_lpf_process(cf->lpf, yn1);
  }

  ringbuf_write(cf->buf, yn1 * cf->g + xn);

  if (!cf->interpolate) {
    return yn1;
  }

  yn2 = ringbuf_read(cf->buf, cf->delay_samps + 1);
  if (cf->enable_lpf) {
    yn2 = simple_lpf_process(cf->lpf, yn2);
  }

  return (1.0 - frac) * yn1 + (frac) * yn2;
}
