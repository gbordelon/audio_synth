#include <stdlib.h>
#include <math.h>

#include "osc.h"
#include "imp.h"
#include "saw.h"
#include "sin.h"
#include "squ.h"
#include "tri.h"
#include "../lib/macros.h"

Osc
osc_alloc()
{
  return calloc(1, sizeof(struct osc_t));
}

void
osc_free(Osc osc)
{
  free(osc);
}

// TODO these two functions will only work for impulse waves so move the functions
void
osc_set_velocity_duty_cycle_constant(Osc osc, FTYPE duty_cycle)
{
  ugen_set_duty_cycle(osc->velocity, duty_cycle);
}

void
osc_set_frequency_duty_cycle_constant(Osc osc, FTYPE duty_cycle)
{
  ugen_set_duty_cycle(osc->frequency, duty_cycle);
}

void
osc_set_velocity_gain(Osc osc, FTYPE gain)
{
  osc->gain = gain;
}

void
osc_set_velocity_freq(Osc osc, FTYPE freq)
{
  ugen_set_freq(osc->velocity, freq);
}

void
osc_set_tone_freq(Osc osc, FTYPE freq)
{
  ugen_set_freq(&sc->frequency, freq);
}

void
osc_set(Osc osc, enum ugen_type velocity_type, enum ugen_type frequncy_type, FTYPE velocity_freq, FTYPE tone_freq, FTYPE gain)
{
  osc_set_tone_freq(osc, tone_freq);
  osc_set_velocity_freq(osc, velocity_freq);
  osc_set_velocity_gain(osc, gain);

  switch (frequency_type) {
  case UGEN_OSC_IMP:
    osc->frequency.sample = imp_sample;
    break;
  case UGEN_OSC_SAW:
    osc->frequency.sample = saw_sample;
    break;
  case UGEN_OSC_SIN:
    osc->frequency.sample = sin_sample;
    break;
  case UGEN_OSC_TRI:
    osc->frequency.sample = tri_sample;
    break;
  case UGEN_CONSTANT:
    // fall through
  default:
    osc->frequency.sample = ugen_sample_constant;
    break;
  }

  switch (velocity_type) {
  case UGEN_OSC_IMP:
    osc->velocity.sample = imp_sample;
    break;
  case UGEN_OSC_SAW:
    osc->velocity.sample = saw_sample;
    break;
  case UGEN_OSC_SIN:
    osc->velocity.sample = sin_sample;
    break;
  case UGEN_OSC_TRI:
    osc->velocity.sample = tri_sample;
    break;
  case UGEN_CONSTANT:
    // fall through
  default:
    osc->velocity.sample = ugen_sample_constant;
    break;
  }

  ugen_reset_phase(&osc->velocity);
  ugen_reset_phase(&osc->frequency);
}

Osc
osc_init(enum ugen_type velocity_type, enum ugen_type frequency_type, FTYPE velocity_freq, FTYPE tone_freq)
{
  Osc rv = osc_alloc();
  // null checks

  rv->velocity = ugen_init();
  rv->frequency = ugen_init();

  ugen_set_freq(rv->velocity, velocity_freq);
  ugen_set_freq(rv->frequency, tone_freq);

  osc_set(rv, velocity_type, tone_type, velocity_freq, tone_freq, 0.5);

  return rv;
}

Osc
osc_init_default()
{
  return osc_init(UGEN_CONSTANT, UGEN_OSC_SIN, 0.0, 440.0);
}

void
osc_cleanup(Osc osc)
{
  // TODO make sure to keep track of the correct cleanup function for ugens
  ugen_cleanup(osc->velocity);
  ugen_cleanup(osc->frequency);

  osc_free(osc);
}

void
osc_reset_phase(Osc osc)
{
  ugen_reset_phase(osc->frequency);
  ugen_reset_phase(osc->velocity);
}

FTYPE
osc_sample(Osc osc)
{
  FTYPE sample = ugen_sample(osc->frequency);
  FTYPE gain = osc->gain * ugen_sample(osc->velocity);
  return sample * gain;
}

void
osc_chunk_sample(Osc osc, FTYPE *buf)
{
  static FTYPE samples[CHUNK_SIZE] = {0};
  static FTYPE gains[CHUNK_SIZE] = {0};

  ugen_chunk_sample(osc->frequency, samples);
  ugen_chunk_sample(osc->velocity, gains);

  FTYPE *b, *g, *s;
  for (b = buf, g = gains, s = samples; b - buf < CHUNK_SIZE; b++, g++, s++) {
    *b = osc->gain * *g * *s;
  }
}
