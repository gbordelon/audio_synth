#ifndef DSP_WAVESHAPER_H
#define DSP_WAVESHAPER_H

#include "../lib/macros.h"

#include "../fx/waveshaper.h"

/*
 * Based on equations from Designing Audio Effect Plugins in C++ by Pirkle, chapter 19
 */
/*
//#define sgn(x) (((x) > 0) - ((x) < 0))

typedef enum {
  WS_ARRY,
  WS_SIG,
  WS_SIG2,
  WS_TANH,
  WS_ATAN,
  WS_SCLIP,
  WS_FEXP1,
  WS_FEXP2,
  WS_EXP2,
  WS_ATSR,
  WS_SQS,
  WS_CUBE,
  WS_HCLIP,
  WS_HWR,
  WS_FWR,
  WS_SQR,
  WS_ASQRT
} waveshaper_e;
*/
// forward declaration
typedef struct waveshaper_params_t waveshaper_params;
typedef union dsp_state_u dsp_state;

//typedef void (* shaper_fn)(FTYPE *, waveshaper_params *);

typedef struct waveshaper_params_t {
  waveshaper_e shape;
  shaper_fn fn;
  FTYPE saturation;
  FTYPE asymmetry;
  FTYPE tanh_k;
  FTYPE atan_k;
} waveshaper_params;

void dsp_set_waveshaper_params(dsp_state *state, waveshaper_params params);

#endif
