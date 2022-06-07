#include <stdlib.h>

#include "osc.h"
#include "imp.h"
#include "../lib/macros.h"

Osc
imp_alloc(FTYPE tone_freq, FTYPE duty_cycle, FTYPE sample_freq)
{
  Osc rv = osc_alloc(OSC_IMP, tone_freq, sample_freq);
  // null checks
  if (duty_cycle > 1.0) {
    duty_cycle = 1.0;
  } else if (duty_cycle < 0.0) {
    duty_cycle = 0.0;
  }
  rv->u.imp.duty_cycle = duty_cycle;
  return rv;
}

void
imp_free(Osc osc)
{
  free(osc);
}
