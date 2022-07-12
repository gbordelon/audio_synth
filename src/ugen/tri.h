#ifndef TRI_H
#define TRI_H

#include <stddef.h>

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_tri(Ugen ugen, FTYPE phase_ind);
void ugen_generate_table_tri();

FTYPE osc_tri[UGEN_TABLE_SIZE];

#endif
