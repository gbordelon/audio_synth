#ifndef OSC_H
#define OSC_H

#include <stdlib.h>

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
  FTYPE velocity;
  uint32_t p_inc_whole;
  FTYPE p_inc_frac;
  uint32_t p_ind;
  union {
    struct {
      FTYPE duty_cycle;
    } imp;
  } u;
} *Osc;


Osc osc_alloc_many(size_t num);
Osc osc_init(enum osc_type type, FTYPE tone_freq);
void osc_cleanup(Osc osc);
void osc_set(Osc osc, enum osc_type type, FTYPE tone_freq, FTYPE velocity);
void osc_set_freq(Osc osc, FTYPE tone_freq, FTYPE velocity);

FTYPE osc_sample(Osc osc);
FTYPE osc_sample_phase_osc(Osc osc, Osc mod);
FTYPE osc_sample_phase_sample(Osc osc, FTYPE sample_mod);
FTYPE osc_sample_phase_mod(Osc osc, size_t phase_mod);
void osc_sample_chunk(Osc osc, Osc mod, FTYPE *buf);

#endif
