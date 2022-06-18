#ifndef TRI_H
#define TRI_H

#include "../lib/macros.h"

#include "ugen.h"

FTYPE ugen_sample_tri(Ugen ugen, size_t phase_ind);

void ugen_generate_table_tri();

/*
python3 >/tmp/test.c <<EOF
import math as m
N=96000
val=0.0
incr=4.0/96000.0
print('static const FTYPE osc_tri[OSC_TABLE_SIZE] = {')
for i in range(int(N/4)):
  print('  ' + str(val) + ',')
  val += incr
for i in range(int(N/4), int(3*N/4)):
  print('  ' + str(val) + ',')
  val -= incr
for i in range(int(3*N/4), int(N)):
  print('  ' + str(val) + ',')
  val += incr
print('};')
EOF
*/

FTYPE osc_tri[UGEN_TABLE_SIZE];

#endif
