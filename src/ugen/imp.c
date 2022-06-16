#include "../lib/macros.h"

#include "imp.h"
#include "osc.h"
#include "ugen.h"

FTYPE
imp_sample(Ugen ugen, size_t phase_ind)
{
  FTYPE dc;
  if (!ugen->u.impulse.duty_cycle) {
    dc = ugen->u.impulse.duty_cycle_c;
  } else {
    dc = ugen_sample(ugen->u.impulse.duty_cycle);
  }
  return ((FTYPE)phase_ind) / ((FTYPE)UGEN_TABLE_SIZE) < dc ? 1.0 : 0.0;
}

void
imp_set_duty_cycle(Osc osc, FTYPE duty_cycle)
{
  ugen_set_duty_cycle(osc->frequency, duty_cycle);
}

void
imp_set_duty_cycle_ugen(Osc osc, Ugen duty_cycle)
{
  ugen_set_duty_cycle_ugen(osc->frequency, duty_cycle);
}

Osc
imp_init(FTYPE velocity_freq, FTYPE tone_freq, FTYPE duty_cycle)
{
  Osc rv = osc_init(UGEN_CONSTANT, UGEN_OSC_IMP, velocity_freq, tone_freq);
  imp_set_duty_cycle(rv, duty_cycle);
  // null checks

  return rv;
}

Osc
imp_init_default(FTYPE tone_freq)
{
  return imp_init(1.0, tone_freq, 0.5);
}

void
imp_cleanup(Osc osc)
{
  if (osc->frequency->u.impulse.duty_cycle) {
    ugen_cleanup(osc->frequency->u.impulse.duty_cycle);
  }
  osc_cleanup(osc);
}

