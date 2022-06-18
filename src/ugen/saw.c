#include "../lib/macros.h"

#include "saw.h"
#include "ugen.h"

FTYPE
ugen_sample_saw(Ugen ugen, size_t phase_ind)
{
  return osc_saw[phase_ind];
}
