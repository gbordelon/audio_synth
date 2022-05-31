#include <stdlib.h>

#include "osc.h"
#include "sin.h"
#include "../lib/macros.h"

Osc
sin_alloc(FTYPE tone_freq, FTYPE sample_freq)
{
  Osc rv = osc_alloc(OSC_SIN, tone_freq, sample_freq);
  // null checks
  return rv;
}

void
sin_free(Osc osc)
{
  free(osc);
}
