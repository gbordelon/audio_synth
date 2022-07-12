#ifndef SIN_H
#define SIN_H

#include <stddef.h>

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_sin_p(Ugen ugen, FTYPE phase_ind);
FTYPE ugen_sample_sin(Ugen ugen, FTYPE phase_ind);
FTYPE ugen_sample_sin_table(Ugen ugen, FTYPE phase_ind);
void ugen_generate_table_sin();

FTYPE osc_sin[UGEN_TABLE_SIZE];

#endif
