#ifndef OSC_H
#define OSC_H

#include "../lib/macros.h"

enum Osc_type {
  OSC_SIN,
  OSC_TRI,
  OSC_SAW,
  OSC_IMP,
  OSC_SQU
};

typedef struct oscillator {
  enum Osc_type type;
  FTYPE tone_freq;
  FTYPE sample_freq;
  uint32_t p_inc_whole;
  FTYPE p_inc_frac;
  uint32_t p_ind;
} *Osc;

Osc osc_alloc(enum Osc_type type, FTYPE tone_freq, FTYPE sample_freq);
void osc_free(Osc osc);
FTYPE osc_sample(Osc osc);
FTYPE osc_sample_phase_osc(Osc osc, Osc mod);
FTYPE osc_sample_phase_sample(Osc osc, FTYPE sample_mod);
FTYPE osc_sample_phase_mod(Osc osc, size_t phase_mod);

#endif
