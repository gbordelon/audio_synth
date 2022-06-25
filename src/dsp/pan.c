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
  return dsp_init_default();
}
