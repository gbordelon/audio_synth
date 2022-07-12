#include "../lib/macros.h"

#include "imp.h"
#include "ugen.h"

FTYPE
ugen_sample_imp(Ugen ugen, FTYPE phase_ind)
{
  return phase_ind < ugen->u.impulse.dc ? 1.0 : 0.0;
}
