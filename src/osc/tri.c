#include <stdlib.h>

#include "osc.h"
#include "tri.h"
#include "../lib/macros.h"

Osc
tri_alloc(FTYPE tone_freq, FTYPE sample_freq)
{
  Osc rv = osc_alloc(OSC_TRI, tone_freq, sample_freq);
  // null checks
  return rv;
}

void
tri_free(Osc osc)
{
  free(osc);
}
