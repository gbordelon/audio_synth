#include <stdbool.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "ringbuf.h"
#include "simple_delay.h"

/*
 * Based on the description from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */
Simple_delay
simple_delay_alloc()
{
  return calloc(1, sizeof(struct simple_delay_t));
}

// TODO resizeable buffers
Simple_delay
simple_delay_init(uint32_t delay_samps, FTYPE sample_rate)
{
  Simple_delay sd = simple_delay_alloc();
  sd->sample_rate = sample_rate;
  sd->buf = ringbuf_init(delay_samps);
  sd->delay_samps = delay_samps;
  sd->delay_ms = 1000.0 * ((FTYPE)delay_samps) / sample_rate;

  sd->buf_len_samps = sd->delay_samps;
  sd->buf_len_ms = sd->delay_ms;

  sd->interpolate = false;

  return sd;
}

void
simple_delay_free(Simple_delay sd)
{
  free(sd);
}

void
simple_delay_reset(Simple_delay sd)
{
  ringbuf_reset(sd->buf);
}

void
simple_delay_cleanup(Simple_delay sd)
{
  ringbuf_cleanup(sd->buf);
  simple_delay_free(sd);
}

FTYPE
simple_delay_read(Simple_delay sd)
{
  return ringbuf_read(sd->buf, sd->delay_samps);
}

FTYPE
simple_delay_read_at_ms(Simple_delay sd, FTYPE ms)
{
  FTYPE s1, s2;
  FTYPE frac = 0.001 * ms * sd->sample_rate;
  uint32_t index = (uint32_t)frac;

  s1 = ringbuf_read(sd->buf, index);
  if (!sd->interpolate) {
    return s1;
  }

  s2 = ringbuf_read(sd->buf, index + 1);
  frac -= index;
  return (1.0 - frac) * s1 + (frac) * s2;
}

FTYPE
simple_delay_read_at_percentage(Simple_delay sd, FTYPE percent)
{
  return simple_delay_read_at_ms(sd, sd->delay_ms * percent * 0.01);
}

void
simple_delay_write(Simple_delay sd, FTYPE sample)
{
  ringbuf_write(sd->buf, sample); 
}
