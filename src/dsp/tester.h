#ifndef TESTER_H
#define TESTER_H

#include "../lib/macros.h"

#include "comb_filter.h"
#include "dsp.h"
#include "simple_delay.h"
#include "simple_lpf.h"

typedef struct tester_params_t {
  Simple_delay delay;
  Simple_lpf lpf;
  Comb_filter comb;
} tester_params;

void dsp_set_tester_params(dsp_state *state, tester_params params);

#endif
