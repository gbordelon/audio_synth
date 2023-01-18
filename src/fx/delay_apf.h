#ifndef FX_DELAY_APF_H
#define FX_DELAY_APF_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#include "simple_delay.h"
#include "simple_lpf.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */

typedef struct delay_apf_t {
  FTYPE sample_rate;
  FTYPE a;
  FTYPE apf_g;
  FTYPE lpf_g;

  FTYPE delay_ms;
  uint32_t delay_samps;

  FTYPE lfo_freq;
  FTYPE lfo_depth;
  FTYPE lfo_max_mod_ms;

  bool enable_lfo;
  bool enable_lpf;

  Simple_lpf lpf;
  Ugen lfo;
  Simple_delay sd;

  struct delay_apf_t *nested_apf;
} *Delay_apf;

Delay_apf delay_apf_init(FTYPE a, FTYPE apf_g, FTYPE lpf_g, FTYPE delay_ms, FTYPE lfo_freq, FTYPE lfo_depth, FTYPE lfo_max_mod_ms, FTYPE sample_rate);

Delay_apf delay_apf_init_default();
Delay_apf delay_apf_init_nested_default();

void delay_apf_reset(Delay_apf apf);
void delay_apf_cleanup(Delay_apf apf);

FTYPE delay_apf_process(Delay_apf apf, FTYPE xn);

#endif
