#ifndef COMB_FILTER_H
#define COMB_FILTER_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "../fx/ringbuf.h"
#include "simple_lpf.h"

/*
 * Based on the description from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */

typedef struct comb_filter_t {
  FTYPE sample_rate;
  FTYPE delay_ms;
  uint32_t delay_samps;
  FTYPE rt60_ms;
  uint32_t rt60_samps;

  FTYPE g;

  bool enable_lpf;
  bool interpolate;

  FTYPE buf_len_ms;
  uint32_t buf_len_samps;
  Ringbuf buf;
  Simple_lpf lpf;
} *Comb_filter;

Comb_filter comb_filter_init(uint32_t delay_samps, uint32_t rt60_samps, FTYPE lpf_g, FTYPE sample_rate);
void comb_filter_cleanup(Comb_filter cf);

void comb_filter_reset(Comb_filter cf);

FTYPE comb_filter_process(Comb_filter cf, FTYPE xn);

#endif
