#include "../lib/macros.h"

#include "osc.h"
#include "tri.h"
#include "ugen.h"

FTYPE
tri_sample(Ugen ugen, size_t phase_ind)
{
  return osc_tri[phase_ind];
}

Osc
tri_init(FTYPE velocity_freq, FTYPE tone_freq)
{
  Osc rv = osc_init(UGEN_CONSTANT, UGEN_OSC_TRI, velocity_freq, tone_freq);
  // null checks
  return rv;
}

Osc
tri_init_default(FTYPE tone_freq)
{
  return tri_init(1.0, tone_freq);
}

void
tri_cleanup(Osc osc)
{
  osc_cleanup(osc);
}
