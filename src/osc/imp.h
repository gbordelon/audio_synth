#ifndef IMP_H
#define IMP_H

#include "osc.h"
#include "../lib/macros.h"

#define imp_sample(d,p) ((((FTYPE)(p)) / ((FTYPE)OSC_TABLE_SIZE)) < (d) ? 1.0 : 0.0)

Osc imp_alloc(FTYPE tone_freq, FTYPE duty_cycle, FTYPE sample_freq);
void imp_free(Osc osc);

#endif
