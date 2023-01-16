#ifndef FX_WAVESHAPER_H
#define FX_WAVESHAPER_H

#include <stdlib.h>

#include "../lib/macros.h"

/*
 * Based on equations from Designing Audio Effect Plugins in C++ by Pirkle, chapter 19
 */

#define sgn(x) (((x) > 0) - ((x) < 0))

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

// forward declaration
typedef struct fx_unit_waveshaper_state_t *FX_unit_waveshaper_state;

typedef FTYPE (* shaper_fn)(FTYPE, const FX_unit_waveshaper_state);

typedef struct fx_unit_waveshaper_state_t {
  waveshaper_e shape;
  shaper_fn fn;
  FTYPE saturation;
  FTYPE asymmetry;
  FTYPE tanh_k;
  FTYPE atan_k;
} fx_unit_waveshaper_state;

typedef struct fx_unit_waveshaper_params_t {
  waveshaper_e shape;
  FTYPE saturation;
  FTYPE asymmetry;
} fx_unit_waveshaper_params;

typedef struct fx_unit_waveshaper_params_t *FX_unit_waveshaper_params;

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef struct fx_compound_unit_t *FX_compound_unit;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_waveshaper_init(FX_unit_params params);
fx_unit_params fx_unit_waveshaper_default();
FX_compound_unit fx_compound_unit_waveshaper_init(FX_unit_params params);

#endif
