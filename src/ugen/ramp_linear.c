#include <stddef.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "ugen.h"
#include "ramp_linear.h"

FTYPE
ugen_sample_ramp_linear_down(Ugen ugen, size_t phase_ind)
{
  return 1.0 - ramp_linear_up[phase_ind];
}

FTYPE
ugen_sample_ramp_linear_up(Ugen ugen, size_t phase_ind)
{
  return ramp_linear_up[phase_ind];
}

void
ugen_generate_table_ramp_linear_up()
{
  int i;
  FTYPE incr = 1.0 / UGEN_TABLE_SIZE;

  ramp_linear_up[0] = 0.0;
  for (i = 1; i < UGEN_TABLE_SIZE; i++) {
    ramp_linear_up[i] = ramp_linear_up[i - 1] + incr;
  }
}
