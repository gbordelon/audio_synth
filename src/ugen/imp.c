#include "../lib/macros.h"

#include "imp.h"
#include "ugen.h"

FTYPE
ugen_sample_imp(Ugen ugen, size_t phase_ind)
{
  FTYPE dc;
  if (ugen->u.impulse.duty_cycle) {
    dc = ugen_sample(ugen->u.impulse.duty_cycle);
  } else {
    dc = ugen->u.impulse.duty_cycle_c;
  }
  return ((FTYPE)phase_ind) / ((FTYPE)UGEN_TABLE_SIZE) < dc ? 1.0 : 0.0;
}
