#ifndef DSP_PAN_H
#define DSP_PAN_H

void stereo_pan(FTYPE *L, FTYPE *R, FTYPE control);

DSP_callback dsp_init_stereo_pan();

#endif
