#ifndef SAW_H
#define SAW_H

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_saw(Ugen ugen, size_t phase_ind);
void ugen_generate_table_saw();

/*
python3 >/tmp/test.c <<EOF
import math as m
N=96000
val=0.0
incr=2.0/96000.0
print('static const FTYPE osc_saw[OSC_TABLE_SIZE] = {')
for i in range(int(N/2)):
  print('  ' + str(val) + ',')
  val += incr
val = -1.0
for i in range(int(N/2), int(N)):
  print('  ' + str(val) + ',')
  val += incr
print('};')
EOF
*/
FTYPE osc_saw[UGEN_TABLE_SIZE];

#endif
