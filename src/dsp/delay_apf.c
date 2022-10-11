#include <stdbool.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#include "delay_apf.h"
#include "simple_delay.h"

/*
 * Based on the description from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */
Delay_apf
delay_apf_alloc()
{
  return calloc(1, sizeof(struct delay_apf_t));
}

// TODO resizeable buffers
Delay_apf
delay_apf_init(
    FTYPE a,
    FTYPE apf_g,
    FTYPE lpf_g,
    FTYPE delay_ms,
    FTYPE lfo_freq,
    FTYPE lfo_depth,
    FTYPE lfo_max_mod_ms,
    FTYPE sample_rate)
{
  Delay_apf apf = delay_apf_alloc();
  apf->sample_rate = sample_rate;
  apf->delay_ms = delay_ms;
  apf->delay_samps = delay_ms * 0.001 * sample_rate;
  apf->a = a;
  apf->apf_g = apf_g;
  apf->lfo_depth = lfo_depth;
  apf->lfo_max_mod_ms = lfo_max_mod_ms;

  apf->sd = simple_delay_init(apf->delay_samps, sample_rate);
  apf->lfo = ugen_init_tri(lfo_freq, sample_rate);
  apf->lpf = simple_lpf_init(lpf_g);

  apf->sd->interpolate = false;
  apf->enable_lfo = true;
  apf->enable_lpf = true;

  return apf;
}

Delay_apf
delay_apf_init_default()
{
  return delay_apf_init(1.1, 0.707, 0.707, 120.0, 0.1, 0.4, 10.0, DEFAULT_SAMPLE_RATE);
}

Delay_apf
delay_apf_init_nested_default()
{
  Delay_apf apf = delay_apf_init(0.9, 0.5, 0.3, 33.0 * 0.317, 0.15, 1.0, 0.3, DEFAULT_SAMPLE_RATE);
  apf->nested_apf = delay_apf_init(0.0, -0.5, 0.0, 33.0 * 0.873, 0.15, 1.0, 0.3, DEFAULT_SAMPLE_RATE);
  apf->nested_apf->enable_lpf = false;
  apf->nested_apf->enable_lfo = false;
  return apf;
}

void
delay_apf_free(Delay_apf apf)
{
  free(apf);
}

void
delay_apf_reset(Delay_apf apf)
{
  simple_delay_reset(apf->sd);
  ugen_reset_phase(apf->lfo);
}

void
delay_apf_cleanup(Delay_apf apf)
{
  simple_delay_cleanup(apf->sd);
  ugen_cleanup(apf->lfo);
  simple_lpf_cleanup(apf->lpf);
  if (apf->nested_apf) {
    delay_apf_cleanup(apf->nested_apf);
  }
  delay_apf_free(apf);
}

FTYPE
delay_apf_process(Delay_apf apf, FTYPE xn)
{
  FTYPE delay_ms;
  FTYPE yn, zn, xn1, intermediate;

  if (apf->enable_lfo) {
    delay_ms = ugen_sample_mod(apf->lfo, 0.0) * apf->lfo_depth;
    delay_ms = bipolar_modulation(delay_ms, apf->delay_ms - apf->lfo_max_mod_ms, apf->delay_ms);
  } else {
    delay_ms = apf->delay_ms;
  }

  zn = simple_delay_read_at_ms(apf->sd, delay_ms);
  xn1 = xn + zn * apf->apf_g;
  yn = zn - xn1 * apf->apf_g;

  if (apf->enable_lpf) {
    intermediate = simple_lpf_process(apf->lpf, xn1) * apf->a;
  } else {
    intermediate = xn1;
  }

  if (apf->nested_apf) {
    intermediate = delay_apf_process(apf->nested_apf, intermediate);
  }
  simple_delay_write(apf->sd, intermediate);

  return yn;
}
