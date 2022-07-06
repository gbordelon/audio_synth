#include "../lib/macros.h"

#include "imp.h"
#include "ugen.h"

FTYPE
ugen_sample_imp(Ugen ugen, size_t phase_ind)
{
  FTYPE dc = 0.5;
  if (ugen->u.impulse.duty_cycle) {
    dc = ugen_sample_mod(ugen->u.impulse.duty_cycle, 0.0);
  }
  return ((FTYPE)phase_ind) / ((FTYPE)UGEN_TABLE_SIZE) < dc ? 1.0 : 0.0;
}
