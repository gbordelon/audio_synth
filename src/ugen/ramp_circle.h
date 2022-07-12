#ifndef RAMP_CIRCLE_H
#define RAMP_CIRCLE_H

#include <stddef.h>

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_ramp_circle_down(Ugen ugen, FTYPE phase_ind);
FTYPE ugen_sample_ramp_circle_up(Ugen ugen, FTYPE phase_ind);

void ugen_generate_table_ramp_circle_down();

FTYPE ramp_circle_down[UGEN_TABLE_SIZE];

#endif
