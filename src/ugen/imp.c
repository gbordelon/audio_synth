#include "../lib/macros.h"

#include "imp.h"
#include "ugen.h"

FTYPE
ugen_sample_imp(Ugen ugen, size_t phase_ind)
{
  return ((FTYPE)phase_ind) / ((FTYPE)UGEN_TABLE_SIZE) < ugen->u.impulse.dc ? 1.0 : 0.0;
}
