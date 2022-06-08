#ifndef IMP_H
#define IMP_H

#include "osc.h"
#include "../lib/macros.h"

#define imp_sample(d,p) ((((FTYPE)(p)) / ((FTYPE)OSC_TABLE_SIZE)) < (d) ? 1.0 : 0.0)
#define imp_set_duty_cycle(o,d) ((o)->u.imp.duty_cycle = (d))

Osc imp_init(FTYPE tone_freq, FTYPE duty_cycle);
void imp_cleanup(Osc osc);

#endif
