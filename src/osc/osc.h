#ifndef OSC_H
#define OSC_H

#include "../lib/macros.h"

#define osc_reset_phase(o) ((o)->p_ind = 0)

enum osc_type {
  OSC_SIN,
  OSC_TRI,
  OSC_SAW,
  OSC_IMP,
  OSC_SQU
};

typedef struct oscillator {
  enum osc_type type;
  FTYPE tone_freq;
  FTYPE sample_freq;
  uint32_t p_inc_whole;
  FTYPE p_inc_frac;
  uint32_t p_ind;
  union {
    struct {
      FTYPE duty_cycle;
    } imp;
  } u;
} *Osc;

Osc osc_alloc(enum osc_type type, FTYPE tone_freq, FTYPE sample_freq);
void osc_free(Osc osc);
FTYPE osc_sample(Osc osc);
FTYPE osc_sample_phase_osc(Osc osc, Osc mod);
FTYPE osc_sample_phase_sample(Osc osc, FTYPE sample_mod);
FTYPE osc_sample_phase_mod(Osc osc, size_t phase_mod);

#endif
