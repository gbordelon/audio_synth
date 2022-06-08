#include <stdlib.h>
#include <math.h>

#include "osc.h"
#include "imp.h"
#include "saw.h"
#include "sin.h"
#include "squ.h"
#include "tri.h"
#include "../lib/macros.h"


// TODO support realtime updates of tone_freq which requires updating p_inc_whole and p_inc_frac
// useful for pitch bends?
Osc
osc_alloc(enum osc_type type, FTYPE tone_freq, FTYPE sample_freq)
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
  return osc_sample_phase_mod(osc, 0);
}

// TODO this assumes we don't want to phase modulate the modulator oscillator
// How about callback logic or a function pointer in the osc struct to indicate the desired sampling method. This would allow for chaining of modulated signals
FTYPE
osc_sample_phase_osc(Osc osc, Osc mod)
{
  return osc_sample_phase_sample(osc, osc_sample(mod));
}

FTYPE
osc_sample_phase_sample(Osc osc, FTYPE sample_mod)
{
  // 1.0 maps to (OSC_TABLE_SIZE / 4), -1.0 maps to -(OSC_TABLE_SIZE / 4), 0.0 maps to 0
  long phase_mod = lround(sample_mod * OSC_TABLE_SIZE / 4.0);
  if (phase_mod < 0) {
    phase_mod += OSC_TABLE_SIZE;
  }
  return osc_sample_phase_mod(osc, (size_t)phase_mod);
}

FTYPE
osc_sample_phase_mod(Osc osc, size_t phase_mod)
{
  const FTYPE *table;
  FTYPE sample1, sample2;
  size_t phase_ind;

  switch(osc->type) {
  case OSC_SIN:
    table = osc_sin;
    break;
  case OSC_TRI:
    table = osc_tri;
    break;
  case OSC_SAW:
    table = osc_saw;
    break;
  case OSC_SQU:
    table = osc_squ;
    break;
  case OSC_IMP:
  default:
    table = NULL;
    break;
  }

  osc->p_ind += osc->p_inc_whole;
  if (osc->p_ind >= OSC_TABLE_SIZE) {
    osc->p_ind %= OSC_TABLE_SIZE;
  }
  phase_ind = osc->p_ind + phase_mod;
  if (phase_ind >= OSC_TABLE_SIZE) {
    phase_ind %= OSC_TABLE_SIZE;
  }

  if (osc->type == OSC_IMP) {
    sample1 = imp_sample(osc->u.imp.duty_cycle, phase_ind);
  } else {
    sample1 = table[phase_ind];
  }

  if (osc->type == OSC_IMP || osc->type == OSC_SQU) {
    sample2 = sample1;
  } else if ((phase_ind + 1) == OSC_TABLE_SIZE) {
    sample2 = table[0];
  } else {
    sample2 = table[phase_ind + 1];
  }

  return (1.0 - osc->p_inc_frac) * sample1 + osc->p_inc_frac * sample2;
}
