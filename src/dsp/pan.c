#include "../lib/macros.h"

#include "dsp.h"
#include "pan.h"
#include "../ugen/ugen.h"

void
stereo_pan(FTYPE *L, FTYPE *R, FTYPE control)
{
  *L *= (1.0 - control);
  *R *= control;
}

DSP_callback
dsp_init_stereo_pan()
{
  return dsp_init_default();
}
