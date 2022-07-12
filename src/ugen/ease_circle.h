#ifndef EASE_CIRCLE_H
#define EASE_CIRCLE_H

#include <stddef.h>

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_ease_in_circle(Ugen ugen, FTYPE phase_ind);
FTYPE ugen_sample_ease_out_circle(Ugen ugen, FTYPE phase_ind);

void ugen_generate_table_ease_in_circle();
void ugen_generate_table_ease_out_circle();

FTYPE ease_in_circle[UGEN_TABLE_SIZE];
FTYPE ease_out_circle[UGEN_TABLE_SIZE];

#endif
