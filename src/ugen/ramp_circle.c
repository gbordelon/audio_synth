#include <math.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "ugen.h"
#include "ramp_circle.h"

FTYPE
ugen_sample_ramp_circle_down(Ugen ugen, size_t phase_ind)
{
  return ramp_circle_down[phase_ind];
}

FTYPE
ugen_sample_ramp_circle_up(Ugen ugen, size_t phase_ind)
{
  return 1.0 - ramp_circle_down[phase_ind];
}

void
ugen_generate_table_ramp_circle_down()
{
  int i;
  FTYPE incr = 1.0 / UGEN_TABLE_SIZE;
  for (i = 0; i < UGEN_TABLE_SIZE; i++) {
    ramp_circle_down[i] = sqrt(1.0 - ((FTYPE)i)*incr * ((FTYPE)i)*incr);
  }
}

