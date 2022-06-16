#ifndef OSC_H
#define OSC_H

#include <stdlib.h>

#include "../lib/macros.h"

#include "ugen.h"

typedef struct osc_t {
  FTYPE gain;
  Ugen velocity;
  Ugen frequency;
} *Osc;


Osc osc_init(enum ugen_type type, FTYPE tone_freq);
void osc_cleanup(Osc osc);
void osc_set(Osc osc, enum ugen_type velocity_type, enum ugen_type frequency_type, FTYPE velocity_freq, FTYPE tone_freq, FTYPE velocity);
void osc_set_velocity_freq(Osc osc, FTYPE freq);
void osc_set_tone_freq(Osc osc, FTYPE freq);
void osc_set_velocity_gain(Osc osc, FTYPE gain);

void osc_reset_phase(Osc osc);
FTYPE osc_sample(Osc osc);
void osc_chunk_sample(Osc osc, Osc mod, FTYPE *buf);

#endif
