#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"

#include "ringbuf.h"

Ringbuf
ringbuf_alloc()
{
  return calloc(1, sizeof(struct ringbuf_t));
}

void
ringbuf_free(Ringbuf rb)
{
  free(rb);
}

void
ringbuf_cleanup(Ringbuf rb)
{
  free(rb->buf);
  ringbuf_free(rb);
}

Ringbuf
ringbuf_init(uint32_t buf_len)
{
  Ringbuf rb = ringbuf_alloc();
  rb->buf_len = (size_t)pow(2.0, ceil(log2(buf_len)));
  rb->wrap_mask = rb->buf_len - 1;
  rb->buf = calloc(rb->buf_len, sizeof(FTYPE));

  return rb;
}

Ringbuf
ringbuf_init_default()
{
  return ringbuf_init(DEFAULT_SAMPLE_RATE * 2); // 2 seconds
}

void
ringbuf_flush(Ringbuf rb)
{
  rb->write_idx = 0;
  memset(rb->buf, 0, rb->buf_len * sizeof(FTYPE));
}

FTYPE
_ringbuf_read(Ringbuf rb, int32_t delay_in_samples)
{
  int32_t read_idx = rb->write_idx - delay_in_samples;
  read_idx &= rb->wrap_mask;
  return rb->buf[read_idx];
}

FTYPE
ringbuf_read(Ringbuf rb, FTYPE delay_in_samples)
{
  FTYPE s1, s2, frac;
  s1 = _ringbuf_read(rb, (int32_t)delay_in_samples);
  s2 = _ringbuf_read(rb, 1 + (int32_t)delay_in_samples);
  frac = delay_in_samples - (int32_t)delay_in_samples;

  return (1.0 - frac) * s1 + (frac) * s2;
}

void
ringbuf_write(Ringbuf rb, FTYPE sample)
{
  rb->buf[rb->write_idx++] = sample;
  rb->write_idx &= rb->wrap_mask;
}
