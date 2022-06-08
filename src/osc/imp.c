#include "osc.h"
#include "imp.h"
#include "../lib/macros.h"

Osc
imp_init(FTYPE tone_freq, FTYPE duty_cycle)
{
  Osc rv = osc_init(OSC_IMP, tone_freq);
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
imp_cleanup(Osc osc)
{
  osc_cleanup(osc);
}
