#include "../lib/macros.h"

#include "dsp.h"
#include "pan.h"

FTYPE
stereo_pan(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  *L *= (1.0 - control);
  *R *= control;
  return control;
}

DSP_callback
dsp_init_stereo_pan()
{
  DSP_callback cb = dsp_init();
  dsp_set_control_constant(cb, 0.5);
  dsp_set_stereo(cb, stereo_pan);

  return cb;
}
