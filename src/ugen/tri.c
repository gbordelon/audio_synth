#include "../lib/macros.h"

#include "tri.h"
#include "ugen.h"

FTYPE
ugen_sample_tri(Ugen ugen, FTYPE phase_ind)
{
  return osc_tri[(size_t)(phase_ind * UGEN_TABLE_SIZE)];
}

void
ugen_generate_table_tri()
{
  FTYPE incr = 4.0 / (FTYPE)UGEN_TABLE_SIZE;
  FTYPE val = 0.0;
  int i;
  for (i = 0; i < UGEN_TABLE_SIZE>>2; i++, val += incr) {
    osc_tri[i] = val;
  }
  for (; i < ((UGEN_TABLE_SIZE>>1) + (UGEN_TABLE_SIZE>>2)); i++, val -= incr) {
    osc_tri[i] = val;
  }
  for (; i < UGEN_TABLE_SIZE; i++, val += incr) {
    osc_tri[i] = val;
  }
}
