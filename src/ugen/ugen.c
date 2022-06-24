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
  rv->u.impulse.duty_cycle_c = 0.5;
  rv->gain_c = 1.0;
  rv->conv.bias = 0.0;
  rv->conv.scale = 1.0;
  rv->cr = false;

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
  ugen->u.impulse.duty_cycle_c = duty_cycle;
  ugen_set_freq(ugen, freq);

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
  ugen->cr = true;
  return ugen;
}

Ugen
ugen_init_ease_out_circle(FTYPE freq)
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_ease_out_circle;
  ugen_set_freq(ugen, freq);
  ugen->cr = true;
  return ugen;
}

Ugen
ugen_init_ramp_linear(FTYPE freq)
{
  Ugen ugen = ugen_init();
  ugen->sample = ugen_sample_ramp_linear;
  ugen_set_freq(ugen, freq);
  ugen->cr = true;
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
  ugen_free(ugen);
}

void
ugen_set_freq(Ugen ugen, FTYPE freq)
{
  ugen->p_inc_whole = floor(UGEN_TABLE_SIZE * freq / (FTYPE)DEFAULT_SAMPLE_RATE);
  if (ugen->p_inc_whole < 1) {
    ugen->p_inc_whole = 1;
  }
  ugen->p_inc_frac = fmod(UGEN_TABLE_SIZE * freq / (FTYPE)DEFAULT_SAMPLE_RATE, 1);
}

/*
 * Caller is responsible for freeing the optional attached ugens
 */
void
ugen_set_gain(Ugen car, Ugen gain)
{
  if (!car->gain) {
    car->gain = gain;
  } else {
    // TODO complain
  }
}

void
ugen_set_mod(Ugen car, Ugen mod)
{
  if (!car->mod) {
    car->mod = mod;
  } else {
    // TODO complain
  }
}

void
ugen_set_duty_cycle(Ugen ugen, Ugen duty_cycle)
{
  if (!ugen->u.impulse.duty_cycle) {
    ugen->u.impulse.duty_cycle = duty_cycle;
  } else {
    // complain
  }
}

void
ugen_set_gain_c(Ugen ugen, FTYPE gain)
{
  if (gain > 1.0) {
    gain = 1.0;
  } else if (gain < 0.0) {
    gain = 0.0;
  }
  ugen->gain_c = gain;
}

void
ugen_set_duty_cycle_c(Ugen ugen, FTYPE duty_cycle)
{
  if (duty_cycle > 1.0) {
    duty_cycle = 1.0;
  } else if (duty_cycle < 0.0) {
    duty_cycle = 0.0;
  }
  ugen->u.impulse.duty_cycle_c = duty_cycle;
}

void
ugen_set_scale(Ugen ugen, FTYPE low, FTYPE high)
{
  if (high > 1) {
    high = 1;
  }
  ugen->conv.bias = low;

  if (ugen->cr) {
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

FTYPE
ugen_sample_mod(Ugen ugen, size_t phase_mod)
{
  FTYPE sample1, sample2;
  size_t phase_ind;

  ugen->p_ind += ugen->p_inc_whole;
  if (ugen->p_ind >= UGEN_TABLE_SIZE) {
    ugen->p_ind %= UGEN_TABLE_SIZE;
  }

  phase_ind = ugen->p_ind + phase_mod;
  if (phase_ind >= UGEN_TABLE_SIZE) {
    phase_ind %= UGEN_TABLE_SIZE;
  }

  sample1 = ugen->sample(ugen, phase_ind);
/*
  if (ugen->type == UGEN_OSC_IMP) {
    sample2 = sample1;
  } else
*/
  if ((phase_ind + 1) == UGEN_TABLE_SIZE) {
      sample2 = ugen->sample(ugen, 0);
  } else {
      sample2 = ugen->sample(ugen, phase_ind + 1);
  }

  sample1 = (1.0 - ugen->p_inc_frac) * sample1 + ugen->p_inc_frac * sample2;

  if (ugen->cr) {
    return ugen->conv.bias + ugen->conv.scale * sample1;
  }
  return ugen->conv.bias + ugen->conv.scale * (sample1 + 1.0);
}

FTYPE
ugen_sample(Ugen ugen)
{
  FTYPE sample;
  long phase_mod = 0;

  if (ugen->mod) {
    FTYPE phase_shift = ugen_sample(ugen->mod);

    // This is intended for periodic functions. 1/4 of the table size for a
    // table with one period's worth of samples means 90 deg phase shift
    // Only want to use positive values, so adding the table size to a neg
    // number is the same as using the neg number to shift phase.
    phase_mod = lround(phase_shift * UGEN_TABLE_SIZE / 4.0);
    if (phase_mod < 0) {
      phase_mod += UGEN_TABLE_SIZE;
    }
  }
  sample = ugen_sample_mod(ugen, (size_t)phase_mod);

  if (ugen->gain) {
    sample *= ugen_sample(ugen->gain);
  } else {
    sample *= ugen->gain_c;
  }

  return sample;
}

void
ugen_chunk_sample_mod(Ugen ugen, size_t phase_mod[CHUNK_SIZE], FTYPE gain[CHUNK_SIZE], FTYPE buf[CHUNK_SIZE])
{
  size_t *phase = phase_mod;
  FTYPE *rv = buf;
  FTYPE *g = gain;
  for (; rv - buf < CHUNK_SIZE; phase++, rv++) {
    if (phase_mod) {
      *rv = ugen_sample_mod(ugen, *phase);
    } else {
      *rv = ugen_sample_mod(ugen, 0.0);
    }
    if (gain) {
      *rv *= *g++;
    } else {
      *rv *= ugen->gain_c;
    }
  }
}

void
ugen_chunk_sample(Ugen ugen, FTYPE buf[CHUNK_SIZE])
{
  static size_t phase_chunk[CHUNK_SIZE] = {0};
  static FTYPE gain_chunk[CHUNK_SIZE] = {0};
  FTYPE *gain_ptr = NULL;

  if (ugen->gain) {
    // get a chunk of gains
    ugen_chunk_sample(ugen->gain, gain_chunk);
    gain_ptr = gain_chunk;
  }

  if (!ugen->mod) {
    ugen_chunk_sample_mod(ugen, NULL, gain_ptr, buf);
    return;
  }

  // get a chunk of phase shifts
  ugen_chunk_sample(ugen->mod, buf);

  size_t *phase = phase_chunk;
  FTYPE *mod_sample = buf;
  for(; phase - phase_chunk < CHUNK_SIZE; phase++, mod_sample++) {
    *phase = lround(*mod_sample * UGEN_TABLE_SIZE / 4.0);
    if (*phase < 0) {
      *phase += UGEN_TABLE_SIZE;
    }
  }

  ugen_chunk_sample_mod(ugen, phase_chunk, gain_ptr, buf);
}
