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

Ugen
ugen_init(FTYPE sample_rate)
{
  Ugen rv = ugen_alloc();
  // null checks

  rv->sample_rate =
      sample_rate < DEFAULT_SAMPLE_RATE
      ? DEFAULT_SAMPLE_RATE
      : sample_rate;
  ugen_set_scale(rv, -1.0, 1.0);
  rv->conv.cr = false;

  return rv;
}

Ugen
ugen_init_helper(FTYPE freq, sample_fn fn, FTYPE sample_rate)
{
  Ugen ugen = ugen_init(sample_rate);
  ugen->sample = fn;
  ugen_set_freq(ugen, freq);

  return ugen;
}

Ugen
ugen_init_imp(FTYPE freq, FTYPE duty_cycle, FTYPE sample_rate)
{
  Ugen ugen = ugen_init_helper(freq, ugen_sample_imp, sample_rate);
  ugen->u.impulse.dc = duty_cycle;

  return ugen;
}

Ugen
ugen_init_saw(FTYPE freq, FTYPE sample_rate)
{
  return ugen_init_helper(freq, ugen_sample_saw, sample_rate);
}

Ugen
ugen_init_sin(FTYPE freq, FTYPE sample_rate)
{
  return ugen_init_helper(freq, ugen_sample_sin_p, sample_rate);
}

Ugen
ugen_init_tri(FTYPE freq, FTYPE sample_rate)
{
  return ugen_init_helper(freq, ugen_sample_tri, sample_rate);
}

Ugen
ugen_init_ease_in_circle(FTYPE freq, FTYPE sample_rate)
{
  Ugen ugen = ugen_init_helper(freq, ugen_sample_ease_in_circle, sample_rate);
  ugen->conv.cr = true;
  return ugen;
}

Ugen
ugen_init_ease_out_circle(FTYPE freq, FTYPE sample_rate)
{
  Ugen ugen = ugen_init_helper(freq, ugen_sample_ease_out_circle, sample_rate);
  ugen->conv.cr = true;
  return ugen;
}

Ugen
ugen_init_ramp_linear(FTYPE freq, FTYPE sample_rate)
{
  Ugen ugen = ugen_init_helper(freq, ugen_sample_ramp_linear, sample_rate);
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
    ugen_free(ugen);
  }
}

void
ugen_set_freq(Ugen ugen, FTYPE freq)
{
  ugen->p_inc_whole = floor(freq * ((FTYPE)UGEN_TABLE_SIZE) / ugen->sample_rate);
  ugen->p_inc_frac = fmod(freq * ((FTYPE)UGEN_TABLE_SIZE) / ugen->sample_rate, 1);

  ugen->p_inc = freq / ugen->sample_rate;
}

void
ugen_set_sample_rate(Ugen ugen, FTYPE sample_rate)
{
  ugen->p_inc_whole = floor(ugen->p_inc_whole * ugen->sample_rate / sample_rate);
  ugen->p_inc_frac = fmod(ugen->p_inc_frac * ugen->sample_rate / sample_rate, 1);
  ugen->p_inc *= ugen->sample_rate / sample_rate;

  ugen->sample_rate =
      sample_rate < DEFAULT_SAMPLE_RATE
      ? DEFAULT_SAMPLE_RATE
      : sample_rate;
}

void
ugen_set_duty_cycle(Ugen ugen, FTYPE duty_cycle)
{
  ugen->u.impulse.dc = duty_cycle;
}

void
ugen_set_scale(Ugen ugen, FTYPE low, FTYPE high)
{
  if (high > 1.0) {
    high = 1.0;
  }
  ugen->conv.bias = low;

  if (ugen->conv.cr) {
    if (low < 0.0) {
      low = 0.0;
    }
    ugen->conv.scale = high - low;
  } else {
    if (low < -1.0) {
      low = -1.0;
    }
    ugen->conv.scale = (high - low) * 0.5;
  }
}

static const FTYPE UGEN_TABLE_SIZE_INV = 1.0 / (FTYPE)UGEN_TABLE_SIZE;
/*
 * Expect phase_mod as radians so convert to wave table phase units.
 */
void
ugen_sample_mod_triphase(Ugen ugen, FTYPE phase_mod, triphase rv)
{
  FTYPE sample1, sample2, phase_mod_frac;
  int32_t phase_ind, phase_mod_whole;

  if (phase_mod < 0.000001 && phase_mod > -0.000001) {
    phase_mod_whole = 0;
    phase_mod_frac = 0.0;
  } else {
    phase_mod *= 0.5 * M_1_PI * (FTYPE)UGEN_TABLE_SIZE;
    phase_mod_whole = (int32_t)phase_mod;
    phase_mod_frac = phase_mod - phase_mod_whole;
    if ((ugen->p_inc_frac + phase_mod_frac) < 1 && (ugen->p_inc_frac + phase_mod_frac) > -1) {
      phase_mod_frac += ugen->p_inc_frac;
    } else if ((ugen->p_inc_frac + phase_mod_frac) >= 1) {
      phase_mod_whole += ugen->p_inc_frac + phase_mod_frac;
      phase_mod_frac = ugen->p_inc_frac + phase_mod_frac - (int32_t)(ugen->p_inc_frac + phase_mod_frac);
      phase_mod_whole -= phase_mod_frac;
    } else if ((ugen->p_inc_frac + phase_mod_frac) <= -1) {
      phase_mod_whole += ugen->p_inc_frac + phase_mod_frac;
      phase_mod_frac = ugen->p_inc_frac + phase_mod_frac - (int32_t)(ugen->p_inc_frac + phase_mod_frac);
      phase_mod_whole -= phase_mod_frac;
    } else {
    }
  }

  // increment phase index
  ugen->p_ind &= (UGEN_TABLE_SIZE - 1); // wrap

  // iterate over both desired phases
  int i;
  for (i = UGEN_PHASE_NORM; i < UGEN_PHASE_QUAD_NEG; i++) {  
    // modulate phase
    phase_ind = ugen->p_ind + phase_mod_whole;

    if (i == UGEN_PHASE_NORM) {
      // do nothing
    } else if (i == UGEN_PHASE_QUAD) {
      phase_ind += (UGEN_TABLE_SIZE>>2);
    }

    // sample the wave table
    sample1 = ugen->sample(ugen, ((FTYPE)(phase_ind & (UGEN_TABLE_SIZE - 1))) * UGEN_TABLE_SIZE_INV);

    // prepare to sample the wave table at an adjacent index
    if ((ugen->p_inc_whole + phase_mod_whole) >= 0) {
      phase_ind++;
    } else {
      phase_ind--;
    }

    // sample the wave table
    sample2 = ugen->sample(ugen, ((FTYPE)(phase_ind & (UGEN_TABLE_SIZE - 1))) * UGEN_TABLE_SIZE_INV);

    // linear interpolate between the two samples
    if (phase_mod_frac >= 0) {
      sample1 = (1.0 - phase_mod_frac) * sample1 + phase_mod_frac * sample2;
    } else {
      sample1 = (1.0 + phase_mod_frac) * sample1 - phase_mod_frac * sample2;
    }

    // convert sample range from [-1,1] or [0,1] to the desired range
    if (ugen->conv.cr) {
      sample1 = ugen->conv.bias + ugen->conv.scale * sample1;
    } else {
      sample1 = ugen->conv.bias + ugen->conv.scale * (sample1 + 1.0);
    }

    // store the result
    rv[i] = sample1;
  }

  ugen->p_ind += ugen->p_inc_whole;

  rv[UGEN_PHASE_QUAD_NEG] = -rv[UGEN_PHASE_QUAD];
  rv[UGEN_PHASE_INV] = -rv[UGEN_PHASE_NORM];
}

void
ugen_sample_fast_triphase(Ugen ugen, FTYPE phase_mod, triphase rv)
{
  FTYPE sample;
  FTYPE phase_ind;

  ugen->p = ugen->p - (int32_t)ugen->p; // wrap to [0,1]

  phase_mod *= 0.5 * M_1_PI; // convert radian to unit
/*
 * -1.00 -> 1.00 f(x) = 1.0 + 1.0 + x = -x - 0.00
 * -0.90 -> 0.10 f(x) = 1.0 + x       = -x - 0.80
 * -0.80 -> 0.20 f(x) = 1.0 + x       = -x - 0.60
 * -0.75 -> 0.25 f(x) = 1.0 + x       = -x - 0.50
 * -0.70 -> 0.30 f(x) = 1.0 + x       = -x - 0.40
 * -0.60 -> 0.40 f(x) = 1.0 + x       = -x - 0.20
 * -0.50 -> 0.50 f(x) = 1.0 + x       = -x + 0.00
 * -0.40 -> 0.60 f(x) = 1.0 + x       = -x + 0.20
 * -0.30 -> 0.70 f(x) = 1.0 + x       = -x + 0.40
 * -0.25 -> 0.75 f(x) = 1.0 + x       = -x + 0.50
 * -0.20 -> 0.80 f(x) = 1.0 + x       = -x + 0.60
 * -0.10 -> 0.90 f(x) = 1.0 + x       = -x + 0.80
 *  0.00 -> 0.00 f(x) = x
 *  0.10 -> 0.10 f(x) = x             = -x + 0.20
 *  0.20 -> 0.20 f(x) = x             = -x + 0.40
 *  0.25 -> 0.25 f(x) = x
 *  0.40 -> 0.40 f(x) = x             = -x + 0.80
 *  0.50 -> 0.50 f(x) = x             = -x + 1.00
 *  0.60 -> 0.60 f(x) = x             = -x + 1.20
 *  0.75 -> 0.75 f(x) = x
 *  0.90 -> 0.90 f(x) = x             = -x + 1.80
 *  1.00 -> 1.00 f(x) = x             = -x + 2.00
 */
  phase_mod += phase_mod <= -1.0 ? 2.0 : phase_mod < 0 ? 1.0 : 0.0; // convert from [-1,1] to [0,1]
  phase_ind = ugen->p + phase_mod - (int32_t)(ugen->p + phase_mod);

  sample = ugen->sample(ugen, phase_ind);
  if (ugen->conv.cr) {
    sample = ugen->conv.bias + ugen->conv.scale * sample;
  } else {
    sample = ugen->conv.bias + ugen->conv.scale * (sample + 1.0);
  }

  // store the result
  rv[UGEN_PHASE_NORM] = sample;
  rv[UGEN_PHASE_INV] = -sample;

  phase_ind += 0.25;
  phase_ind = phase_ind - (int32_t)phase_ind;

  sample = ugen->sample(ugen, phase_ind);
  if (ugen->conv.cr) {
    sample = ugen->conv.bias + ugen->conv.scale * sample;
  } else {
    sample = ugen->conv.bias + ugen->conv.scale * (sample + 1.0);
  }

  rv[UGEN_PHASE_QUAD] = sample;
  rv[UGEN_PHASE_QUAD_NEG] = -sample;

  ugen->p += ugen->p_inc;
}

FTYPE
ugen_sample_mod(Ugen ugen, FTYPE phase_mod)
{
  triphase rv;

  ugen_sample_fast_triphase(ugen, phase_mod, rv);
  return rv[UGEN_PHASE_NORM];
}

FTYPE
ugen_sample_fast(Ugen ugen, FTYPE phase_mod)
{
  triphase rv;

  ugen_sample_fast_triphase(ugen, phase_mod, rv);
  return rv[UGEN_PHASE_NORM];
}
