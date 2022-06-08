#include "osc.h"
#include "squ.h"
#include "../lib/macros.h"

Osc
squ_init(FTYPE tone_freq)
{
  Osc rv = osc_init(OSC_SQU, tone_freq);
  // null checks
  return rv;
}

void
squ_cleanup(Osc osc)
{
  osc_cleanup(osc);
}
