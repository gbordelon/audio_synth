#include "osc.h"
#include "sin.h"
#include "../lib/macros.h"

Osc
sin_init(FTYPE tone_freq)
{
  Osc rv = osc_init(OSC_SIN, tone_freq);
  // null checks
  return rv;
}

void
sin_cleanup(Osc osc)
{
  osc_cleanup(osc);
}
