#ifndef SIMPLE_DELAY_H
#define SIMPLE_DELAY_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "ringbuf.h"

/*
 * Based on the description from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */

typedef struct simple_delay_t {
  FTYPE sample_rate;
  uint32_t delay_samps;
  FTYPE delay_ms;

  FTYPE buf_len_ms;
  uint32_t buf_len_samps;

  bool interpolate;
  Ringbuf buf;
} *Simple_delay;

Simple_delay simple_delay_init(uint32_t delay_samps, FTYPE sample_rate);
void simple_delay_cleanup(Simple_delay sd);

FTYPE simple_delay_read(Simple_delay sd);
FTYPE simple_delay_read_at_ms(Simple_delay sd, FTYPE ms);
FTYPE simple_delay_read_at_percentage(Simple_delay sd, FTYPE percent);

void simple_delay_write(Simple_delay sd, FTYPE sample);

#endif
