#include <math.h>
 
#include "../lib/macros.h"

#include "tri.h"
#include "ugen.h"

FTYPE
ugen_sample_tri_table(Ugen ugen, FTYPE phase_ind)
{
  return osc_tri[(size_t)(phase_ind * UGEN_TABLE_SIZE)];
}

FTYPE
ugen_sample_tri(Ugen ugen, FTYPE phase_ind)
{
  // convert [0,1] to [-1,1]
  phase_ind = phase_ind * 2.0 - 1.0;
  return 2.0 * fabs(phase_ind) - 1.0;
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
