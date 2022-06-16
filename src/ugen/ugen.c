#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "ugen.h"

Ugen
ugen_alloc()
{
  Ugen rv = calloc(1, sizeof(struct ugen_t));
  return rv;
}

FTYPE
ugen_sample_constant(Ugen ugen, size_t phase_mod)
{
  return 1.0;
}

FTYPE
ugen_sample_null(Ugen ugen, size_t phase_mod)
{
  return 0.0;
}

Ugen
ugen_init()
{
  Ugen rv = ugen_alloc();
  // null checks

  rv->sample = ugen_sample_null;
  rv->u.impulse.duty_cycle = 0.5;

  return rv;
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

// min rate for an LFO is currently 0.5 hz
void
ugen_set_freq(Ugen ugen, FTYPE freq)
{
  ugen->p_inc_whole = floor(UGEN_TABLE_SIZE * tone_freq / (FTYPE)DEFAULT_SAMPLE_RATE);
  if (ugen->p_inc_whole < 1) {
    ugen->p_inc_whole = 1;
  }
  ugen->p_inc_frac = fmod(UGEN_TABLE_SIZE * tone_freq / (FTYPE)DEFAULT_SAMPLE_RATE, 1);
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
ugen_set_duty_cycle(Ugen ugen, FTYPE duty_cycle)
{
  if (duty_cycle > 1.0) {
    duty_cycle = 1.0;
  } else if (duty_cycle < 0.0) {
    duty_cycle = 0.0;
  }
  ugen->u.impulse.duty_cycle_c = duty_cycle;
}

/*
 * Caller is responsible for freeing the duty cycle ugen
 * Currently only impulse oscillators use duty cycle
 */
void
ugen_set_duty_cycle_ugen(Ugen ugen, Ugen duty_cycle)
{
  if (!ugen->u.impulse.duty_cycle) {
    ugen->u.impulse.duty_cycle = duty_cycle;
  } else {
    // complain
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
  if (ugen->type == UGEN_OSC_IMP) {
    sample2 = sample1;
  } else if ((phase_ind + 1) == UGEN_TABLE_SIZE) {
      sample2 = ugen->sample(ugen, 0);
  } else {
      sample2 = ugen->sample(ugen, phase_ind + 1);
  }

  return (1.0 - ugen->p_inc_frac) * sample1 + ugen->p_inc_frac * sample2;
}

FTYPE
ugen_sample(Ugen ugen)
{
  if (!ugen->mod) {
    return ugen_sample_mod(ugen, 0);
  }

  FTYPE phase_shift = ugen_sample(ugen->mod);

  // This is intended for periodic functions. 1/4 of the table size for a
  // table with one period's worth of samples means 90 deg phase shift
  // Only want to use positive values, so adding the table size to a neg
  // number is the same as using the neg number to shift phase.
  long phase_mod = lround(phase_shift * UGEN_TABLE_SIZE / 4.0);
  if (phase_mod < 0) {
    phase_mod += UGEN_TABLE_SIZE;
  }

  return ugen_sample_mod(ugen, (size_t)phase_mod);
}

void
ugen_chunk_sample_mod(Ugen ugen, size_t phase_mod[CHUNK_SIZE], FTYPE buf[CHUNK_SIZE])
{
  size_t *phase = phase_mod;
  FTYPE *rv = buf;
  for (; rv - buf < CHUNK_SIZE; phase++, rv++) {
    if (phase_mod) {
      *rv = ugen_sample_mod(ugen, *phase);
    } else {
      *rv = ugen_sample_mod(ugen, 0.0);
    }
  }
}

void
ugen_chunk_sample(Ugen ugen, FTYPE buf[CHUNK_SIZE])
{
  static size_t phase_chunk[CHUNK_SIZE] = {0};

  if (!ugen->mod) {
    ugen_chunk_sample_mod(ugen, NULL, buf);
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

  ugen_chunk_sample_mod(ugen, phase_chunk, buf);
}
