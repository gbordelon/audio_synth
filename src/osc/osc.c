#include <stdlib.h>
#include <math.h>

#include "osc.h"
#include "sin.h"
#include "../lib/macros.h"

Osc
osc_alloc(enum Osc_type type, FTYPE tone_freq, FTYPE sample_freq)
{
  Osc rv = calloc(1, sizeof(struct oscillator));
  // null checks
  rv->type = type;
  rv->tone_freq = tone_freq;
  rv->sample_freq = sample_freq;
  rv->p_inc_whole = floor(OSC_TABLE_SIZE * tone_freq / sample_freq);
  rv->p_inc_frac = fmod(OSC_TABLE_SIZE * tone_freq / sample_freq, 1);
  rv->p_ind = 0;
  return rv;
}

void
osc_free(Osc osc)
{
  free(osc);
}

FTYPE
osc_sample(Osc osc)
{
  const FTYPE *table;
  FTYPE sample1, sample2;

  switch(osc->type) {
  case OSC_SIN:
    table = osc_sin;
    break;
  case OSC_TRI:
    //break;
  case OSC_SAW:
    //break;
  case OSC_IMP:
    //break;
  case OSC_SQU:
    //break;
  default:
    table = NULL;
    break;
  }

  osc->p_ind += osc->p_inc_whole;
  if (osc->p_ind >= OSC_TABLE_SIZE) {
    osc->p_ind %= OSC_TABLE_SIZE;
  }
  sample1 = table[osc->p_ind];

  if ((osc->p_ind + 1) == OSC_TABLE_SIZE) {
    sample2 = table[0];
  } else {
    sample2 = table[osc->p_ind + 1];
  }

  return (1.0 - osc->p_inc_frac) * sample1 + osc->p_inc_frac * sample2;
}
