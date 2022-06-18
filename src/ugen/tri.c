#include "../lib/macros.h"

#include "tri.h"
#include "ugen.h"

FTYPE
ugen_sample_tri(Ugen ugen, size_t phase_ind)
{
  return osc_tri[phase_ind];
}
