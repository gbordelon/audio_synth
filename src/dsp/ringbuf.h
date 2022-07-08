#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>
#include "../lib/macros.h"

typedef struct ringbuf_t {
  size_t write_idx;
  size_t wrap_mask;
  size_t buf_len;
  FTYPE *buf;
} *Ringbuf;

Ringbuf ringbuf_init(uint32_t buf_len);
Ringbuf ringbuf_init_default();

void ringbuf_cleanup(Ringbuf rb);

FTYPE ringbuf_read(Ringbuf rb, FTYPE delay_in_samples);
void ringbuf_write(Ringbuf rb, FTYPE sample);

#endif
