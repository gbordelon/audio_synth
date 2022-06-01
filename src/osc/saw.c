#include <stdlib.h>

#include "osc.h"
#include "saw.h"
#include "../lib/macros.h"

Osc
saw_alloc(FTYPE tone_freq, FTYPE sample_freq)
{
  Osc rv = osc_alloc(OSC_SAW, tone_freq, sample_freq);
  // null checks
  return rv;
}

void
saw_free(Osc osc)
{
  free(osc);
}
