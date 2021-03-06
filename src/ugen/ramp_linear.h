#ifndef LINEAR_RAMP_H
#define LINEAR_RAMP_H

#include <stddef.h>

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_ramp_linear(Ugen ugen, FTYPE phase_ind);

void ugen_generate_table_ramp_linear();

FTYPE ramp_linear[UGEN_TABLE_SIZE];

#endif
