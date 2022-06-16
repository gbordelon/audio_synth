#include "../lib/macros.h"

#include "osc.h"
#include "saw.h"
#include "ugen.h"

FTYPE
saw_sample(Ugen ugen, size_t phase_ind)
{
  return osc_saw[phase_ind];
}

Osc
saw_init(FTYPE velocity_freq, FTYPE tone_freq)
{
  Osc rv = osc_init(UGEN_CONSTANT, UGEN_OSC_SAW, velocity_freq, tone_freq);
  // null checks
  return rv;
}

Osc
saw_init_default(FTYPE tone_freq)
{
  return saw_init(1.0, tone_freq);
}

void
saw_cleanup(Osc osc)
{
  osc_cleanup(osc);
}
