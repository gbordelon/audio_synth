#ifndef SAW_H
#define SAW_H

#include <stddef.h>

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_saw(Ugen ugen, size_t phase_ind);
void ugen_generate_table_saw();

FTYPE osc_saw[UGEN_TABLE_SIZE];

#endif
