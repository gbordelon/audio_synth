#include <math.h>
#include "../lib/macros.h"

#include "sin.h"
#include "ugen.h"

FTYPE
ugen_sample_sin(Ugen ugen, size_t phase_ind)
{
  return osc_sin[phase_ind];
}

void
ugen_generate_table_sin()
{
  int i;
  for (i = 0; i < UGEN_TABLE_SIZE; i++) {
    osc_sin[i] = sin(M_PI * 2 * ((FTYPE)i/(FTYPE)UGEN_TABLE_SIZE));
  }
}
