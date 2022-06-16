#include <stdlib.h>

#include "../lib/macros.h"
#include "ugen.h"
#include "ramp_circle_up.h"

FTYPE
ramp_circle_up_sample(Ugen ugen, size_t phase_ind)
{
  return ramp_circle_up[phase_ind];
}

Ugen
ramp_circle_up_init()
{
  Ugen ugen = ugen_init();
  ugen->sample = ramp_circle_up_sample;
  return ugen;
}

void
ramp_circle_up_cleanup(Ugen ugen)
{
  ugen_cleanup(ugen);
}
