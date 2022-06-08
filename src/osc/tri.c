#include "osc.h"
#include "tri.h"
#include "../lib/macros.h"

Osc
tri_init(FTYPE tone_freq)
{
  Osc rv = osc_init(OSC_TRI, tone_freq);
  // null checks
  return rv;
}

void
tri_cleanup(Osc osc)
{
  osc_cleanup(osc);
}
