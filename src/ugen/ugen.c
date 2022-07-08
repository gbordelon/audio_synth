#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "ugen.h"
#include "imp.h"
#include "saw.h"
#include "sin.h"
#include "tri.h"

#include "ease_circle.h"
#include "ramp_linear.h"

void
ugen_generate_tables()
{
  ugen_generate_table_saw();
  ugen_generate_table_sin();
  ugen_generate_table_tri();
  ugen_generate_table_ease_in_circle();
  ugen_generate_table_ease_out_circle();
  ugen_generate_table_ramp_linear();
}

Ugen
ugen_alloc()
{
  Ugen rv = calloc(1, sizeof(struct ugen_t));
  return rv;
}

FTYPE
ugen_sample_constant(Ugen ugen, size_t phase_ind)
{
  return 1.0;
}

FTYPE
ugen_sample_null(Ugen ugen, size_t phase_ind)
{
  return 0.0;
}

Ugen
ugen_init()
{
  Ugen rv = ugen_alloc();
  // null checks

  rv->sample = ugen_sample_null;
  ugen_set_scale(rv, -1.0, 1.0);
  rv->conv.cr = false;

  rv->u.impulse.duty_cycle = ugen_alloc();
  rv->u.impulse.duty_cycle->sample = ugen_sample_constant;
  rv->u.impulse.duty_cycle->conv.cr = true;
  ugen_set_scale(rv->u.impulse.duty_cycle, 0.5, 0.5);

  return rv;
}

Ugen
ugen_init_constant()
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_constant;
  return ugen;
}

Ugen
ugen_init_imp(FTYPE freq, FTYPE duty_cycle)
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_imp;
  ugen_set_freq(ugen, freq);
  ugen_set_scale(ugen->u.impulse.duty_cycle, duty_cycle, duty_cycle);

  return ugen;
}

Ugen
ugen_init_saw(FTYPE freq)
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_saw;
  ugen_set_freq(ugen, freq);

  return ugen;
}

Ugen
ugen_init_sin(FTYPE freq)
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_sin;
  ugen_set_freq(ugen, freq);

  return ugen;
}

Ugen
ugen_init_tri(FTYPE freq)
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_tri;
  ugen_set_freq(ugen, freq);

  return ugen;
}

Ugen
ugen_init_ease_in_circle(FTYPE freq)
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_ease_in_circle;
  ugen_set_freq(ugen, freq);
  ugen->conv.cr = true;
  return ugen;
}

Ugen
ugen_init_ease_out_circle(FTYPE freq)
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_ease_out_circle;
  ugen_set_freq(ugen, freq);
  ugen->conv.cr = true;
  return ugen;
}

Ugen
ugen_init_ramp_linear(FTYPE freq)
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_ramp_linear;
  ugen_set_freq(ugen, freq);
  ugen->conv.cr = true;
  return ugen;
}

void
ugen_free(Ugen ugen)
{
  free(ugen);
}

void
ugen_cleanup(Ugen ugen)
{
  if (ugen) {
    ugen_cleanup(ugen->gain);
    ugen_cleanup(ugen->u.impulse.duty_cycle);
    ugen_free(ugen);
  }
}

static const FTYPE DSR_INV = ((FTYPE)UGEN_TABLE_SIZE) / (FTYPE)DEFAULT_SAMPLE_RATE;

void
ugen_set_freq(Ugen ugen, FTYPE freq)
{
  ugen->p_inc_whole = floor(freq * DSR_INV);
  ugen->p_inc_frac = fmod(freq * DSR_INV, 1);
}

/*
 * Caller is responsible for freeing the optional attached ugens
 */
void
ugen_set_gain(Ugen car, Ugen gain)
{
  if (car->gain) {
    ugen_cleanup(car->gain);
  }
  car->gain = gain;
}

void
ugen_set_duty_cycle(Ugen ugen, Ugen duty_cycle)
{
  if (ugen->u.impulse.duty_cycle) {
    ugen_cleanup(ugen->u.impulse.duty_cycle);
  }
  ugen->u.impulse.duty_cycle = duty_cycle;
}

void
ugen_set_scale(Ugen ugen, FTYPE low, FTYPE high)
{
  if (high > 1) {
    high = 1;
  }
  ugen->conv.bias = low;

  if (ugen->conv.cr) {
    if (low < 0) {
      low = 0;
    }
    ugen->conv.scale = high - low;
  } else {
    if (low < -1) {
      low = -1;
    }
    ugen->conv.scale = (high - low) * 0.5;
  }
}

/*
 * Expect phase_mod as radians so convert to wave table phase units.
 */
FTYPE
ugen_sample_mod(Ugen ugen, FTYPE phase_mod)
{
  FTYPE sample1, sample2, phase_mod_frac;
  int32_t phase_ind, phase_mod_whole;

  phase_mod_whole = floor(phase_mod * 0.5 * M_1_PI * (FTYPE)UGEN_TABLE_SIZE);
  phase_mod_frac = fmod(phase_mod * 0.5 * M_1_PI * (FTYPE)UGEN_TABLE_SIZE, 1);
  if ((ugen->p_inc_frac + phase_mod_frac) < 1 && (ugen->p_inc_frac + phase_mod_frac) > -1) {
    phase_mod_frac += ugen->p_inc_frac;
  } else if ((ugen->p_inc_frac + phase_mod_frac) >= 1) {
    phase_mod_whole += ugen->p_inc_frac + phase_mod_frac;
    phase_mod_frac = fmod(ugen->p_inc_frac + phase_mod_frac, 1);
    phase_mod_whole -= phase_mod_frac;
  } else if ((ugen->p_inc_frac + phase_mod_frac) <= -1) {
    phase_mod_whole += ugen->p_inc_frac + phase_mod_frac;
    phase_mod_frac = fmod(ugen->p_inc_frac + phase_mod_frac, 1);
    phase_mod_whole -= phase_mod_frac;
  } else {
  }

  ugen->p_ind += ugen->p_inc_whole;
  ugen->p_ind &= (UGEN_TABLE_SIZE - 1);

  phase_ind = ugen->p_ind + phase_mod_whole;
  phase_ind &= (UGEN_TABLE_SIZE - 1);

  sample1 = ugen->sample(ugen, phase_ind);

  if ((ugen->p_inc_whole + phase_mod_whole) >= 0) {
    phase_ind++;
  } else {
    phase_ind--;
  }
  sample2 = ugen->sample(ugen, phase_ind & (UGEN_TABLE_SIZE - 1));

  if (phase_mod_frac >= 0) {
    sample1 = (1.0 - phase_mod_frac) * sample1 + phase_mod_frac * sample2;
  } else {
    sample1 = (1.0 + phase_mod_frac) * sample1 - phase_mod_frac * sample2;
  }

  if (ugen->conv.cr) {
    sample1 = ugen->conv.bias + ugen->conv.scale * sample1;
  } else {
    sample1 = ugen->conv.bias + ugen->conv.scale * (sample1 + 1.0);
  }

  if (ugen->gain) {
    sample1 *= ugen_sample_mod(ugen->gain, 0.0);
  }

  return sample1;
}
