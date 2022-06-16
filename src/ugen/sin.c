#include "../lib/macros.h"

#include "osc.h"
#include "sin.h"
#include "ugen.h"

Osc
sin_init(FTYPE velocity_freq, FTYPE tone_freq)
{
  Osc rv = osc_init(UGEN_CONSTANT, UGE_OSC_SIN, velocity_freq, tone_freq);
  // null checks
  return rv;
}

Osc
sin_init_default(FTYPE tone_freq)
{
  return sin_init(1.0, tone_freq);
}

void
sin_cleanup(Osc osc)
{
  osc_cleanup(osc);
}
