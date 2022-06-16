#include <stdlib.h>

#include "../lib/macros.h"
#include "ugen.h"
#include "ramp_linear.h"

FTYPE
ramp_linear_sample(Ugen ugen, size_t phase_ind)
{
  return ramp_linear[phase_ind];
}

Ugen
ramp_linear_init()
{
  Ugen ugen = ugen_init();
  ugen->sample = ramp_linear_sample;
  return ugen;
}

void
ramp_linear_cleanup(Ugen ugen)
{
  ugen_cleanup(ugen);
}
