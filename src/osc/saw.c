#include "osc.h"
#include "saw.h"
#include "../lib/macros.h"

Osc
saw_init(FTYPE tone_freq)
{
  Osc rv = osc_init(OSC_SAW, tone_freq);
  // null checks
  return rv;
}

void
saw_cleanup(Osc osc)
{
  osc_cleanup(osc);
}
