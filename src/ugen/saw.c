#include "../lib/macros.h"

#include "saw.h"
#include "ugen.h"

FTYPE
ugen_sample_saw(Ugen ugen, FTYPE phase_ind)
{
  return osc_saw[(size_t)(phase_ind * UGEN_TABLE_SIZE)];
}

void
ugen_generate_table_saw()
{
  FTYPE incr = 2.0 / (FTYPE)UGEN_TABLE_SIZE;
  FTYPE val = 0.0;
  int i;
  for (i = 0; i < UGEN_TABLE_SIZE>>2; i++, val += incr) {
    osc_saw[i] = val;
  }
  val = -1.0;
  for (; i < UGEN_TABLE_SIZE; i++, val += incr) {
    osc_saw[i] = val;
  }
}
