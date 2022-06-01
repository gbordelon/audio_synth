#include <stdlib.h>

#include "osc.h"
#include "squ.h"
#include "../lib/macros.h"

Osc
squ_alloc(FTYPE tone_freq, FTYPE sample_freq)
{
  Osc rv = osc_alloc(OSC_SQU, tone_freq, sample_freq);
  // null checks
  return rv;
}

void
squ_free(Osc osc)
{
  free(osc);
}
