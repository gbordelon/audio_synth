#ifndef IMP_H
#define IMP_H

#include "../lib/macros.h"

#include "osc.h"
#include "ugen.h"

// callback for ugen sample code
FTYPE imp_sample(Ugen ugen, size_t phase_ind);

Osc imp_init(FTYPE velocity_freq, FTYPE tone_freq, FTYPE duty_cycle);
Osc imp_init_default(FTYPE tone_freq);
void imp_cleanup(Osc osc);
void imp_set_duty_cycle(Osc osc, FTYPE duty_cycle);
void imp_set_duty_cycle_ugen(Osc osc, Ugen duty_cycle);

#endif
