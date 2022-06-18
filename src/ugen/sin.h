#ifndef SIN_H
#define SIN_H

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_sin(Ugen ugen, size_t phase_ind);

void ugen_generate_table_sin();

/*
python3 >/tmp/test.c <<EOF
import math as m
N=96000
print('static const FTYPE osc_sin[OSC_TABLE_SIZE] = {')
for i in range(N): print('  ' + str(m.sin(m.pi * 2 * i / N)) + ',')
print('};')
EOF
*/
FTYPE osc_sin[UGEN_TABLE_SIZE];

#endif
