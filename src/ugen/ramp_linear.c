#include <stddef.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "ugen.h"
#include "ramp_linear.h"

FTYPE
ugen_sample_ramp_linear(Ugen ugen, size_t phase_ind)
{
  return ramp_linear[phase_ind];
}

void
ugen_generate_table_ramp_linear()
{
  int i;
  FTYPE incr = 1.0 / UGEN_TABLE_SIZE;

  for (i = 0; i < UGEN_TABLE_SIZE; i++) {
    ramp_linear[i] = ((FTYPE)i) * incr;
  }
}
