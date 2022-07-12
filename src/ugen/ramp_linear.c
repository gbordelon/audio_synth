#include <stddef.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "ugen.h"
#include "ramp_linear.h"

FTYPE
ugen_sample_ramp_linear(Ugen ugen, FTYPE phase_ind)
{
  return ramp_linear[(size_t)(phase_ind * UGEN_TABLE_SIZE)];
}

void
ugen_generate_table_ramp_linear()
{
  int i;
  FTYPE incr = 1.0 / (FTYPE)UGEN_TABLE_SIZE;

  for (i = 0; i < UGEN_TABLE_SIZE; i++) {
    ramp_linear[i] = ((FTYPE)i) * incr;
  }
}
