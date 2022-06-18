#ifndef LINEAR_RAMP_H
#define LINEAR_RAMP_H

#include <stddef.h>

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_ramp_linear_down(Ugen ugen, size_t phase_ind);
FTYPE ugen_sample_ramp_linear_up(Ugen ugen, size_t phase_ind);

void ugen_generate_table_ramp_linear_up();

FTYPE ramp_linear_up[UGEN_TABLE_SIZE];

#endif
