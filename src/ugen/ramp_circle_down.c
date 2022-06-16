#include <stdlib.h>

#include "../lib/macros.h"
#include "ugen.h"
#include "ramp_circle_down.h"

FTYPE
ramp_circle_down_sample(Ugen ugen, size_t phase_ind)
{
  return ramp_circle_down[phase_ind];
}

Ugen
ramp_circle_down_init()
{
  Ugen ugen = ugen_init();
  ugen->sample = ramp_circle_down_sample;
  return ugen;
}

void
ramp_circle_down_cleanup(Ugen ugen)
{
  ugen_cleanup(ugen);
}
