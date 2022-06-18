#include "../lib/macros.h"

#include "sin.h"
#include "ugen.h"

FTYPE
ugen_sample_sin(Ugen ugen, size_t phase_ind)
{
  return osc_sin[phase_ind];
}
