#ifndef DSP_PAN_H
#define DSP_PAN_H

#include "../lib/macros.h"

#include "dsp.h"

void stereo_pan(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control);

DSP_callback dsp_init_stereo_pan();

#endif
