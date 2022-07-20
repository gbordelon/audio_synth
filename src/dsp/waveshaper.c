#include <math.h>
#include <stdio.h>

#include "../lib/macros.h"

#include "dsp.h"
#include "bitcrusher.h"
#include "../ugen/ugen.h"

/*
 * Using code from Designing Audio Effect Plugins in C++ by Pirkle, pgs. 550:551
 */

FTYPE
stereo_waveshaper(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  state->waveshaper.fn(L, &state->waveshaper); 
  state->waveshaper.fn(R, &state->waveshaper); 
  return control;
}

static const FTYPE C_1_3 = 1.0 / 3.0;
static const FTYPE C_E_1 = M_E + 1.0;
static const FTYPE C_1_E_1 = 1.0 / (M_E - 1.0);

void
arraya(FTYPE *x, waveshaper_params *p)
{
  *x = 1.5 * *x * (1.0 - *x * *x * C_1_3);
}

void
sigmoid(FTYPE *x, waveshaper_params *p)
{
  *x = 2.0 / (1 + exp(-p->saturation * *x)) - 1.0;
}

void
sigmoid2(FTYPE *x, waveshaper_params *p)
{
  FTYPE e = exp(*x);
  *x = (e - 1.0) * C_E_1 * C_1_E_1 / (e + 1.0);
}

void
hyperbolic_tangent(FTYPE *x, waveshaper_params *p)
{
  *x = tanh(p->saturation * *x) * p->tanh_k;
}

void
arctangent(FTYPE *x, waveshaper_params *p)
{
  *x = atan(p->saturation * *x) * p->atan_k;
}

inline FTYPE
ws_gain(FTYPE *x, FTYPE saturation, FTYPE asymmetry)
{
  return ((*x >= 0.0 && asymmetry > 0.0) || (*x < 0.0 && asymmetry < 0.0)) ? saturation * (1.0 + 4.0*fabs(asymmetry)) : saturation;
}

void
fuzz_exponential_1(FTYPE *x, waveshaper_params *p)
{
  FTYPE g = ws_gain(x, p->saturation, p->asymmetry);
  *x = sgn(*x) * (1.0 - exp(-fabs(g * *x))) / (1.0 - exp(-g));
}

void
fuzz_exponential_2(FTYPE *x, waveshaper_params *p)
{
  *x = sgn(-*x) * (1.0 - exp(fabs(*x))) * C_1_E_1;
}

void
exponential_2(FTYPE *x, waveshaper_params *p)
{
  *x = (M_E - exp(1.0 - *x)) * C_1_E_1;
}

void
atan_sqrt(FTYPE *x, waveshaper_params *p)
{
  FTYPE xs = *x * 0.9;
  *x = 2.5 * (atan(xs) + sqrt(1.0 - xs * xs) - 1.0);
}

void
square_sign(FTYPE *x, waveshaper_params *p)
{
  *x = sgn(*x) * *x * *x;
}

void
cube(FTYPE *x, waveshaper_params *p)
{
  *x = *x * *x * *x;
}

void
hard_clip(FTYPE *x, waveshaper_params *p)
{
  *x = (fabs(*x) > 0.5) ? 0.5 * sgn(*x) : *x;
}

void
halfwave_rect(FTYPE *x, waveshaper_params *p)
{
  *x = 0.5 * (*x + fabs(*x));
}

void
fullwave_rect(FTYPE *x, waveshaper_params *p)
{
  *x = fabs(*x);
}

void
square_law(FTYPE *x, waveshaper_params *p)
{
  *x = *x * *x;
}

void
abs_square_law(FTYPE *x, waveshaper_params *p)
{
  *x = sqrt(fabs(*x));
}

void
dsp_set_waveshaper_params(dsp_state *state, waveshaper_params params)
{
  if (params.saturation < 1.0) {
    params.saturation = 1.0;
  }

  state->waveshaper = params;
  state->waveshaper.tanh_k = 1.0;
  state->waveshaper.atan_k = 1.0;

  if (params.shape == WS_TANH) {
    state->waveshaper.tanh_k = 1.0 / tanh(params.saturation);
  } else if (params.shape == WS_ATAN) {
    state->waveshaper.atan_k = 1.0 / atan(params.saturation);
  }
}

DSP_callback
dsp_init_waveshaper(waveshaper_params params)
{
  DSP_callback cb = dsp_init();
  dsp_set_stereo(cb, stereo_waveshaper);
  switch (params.shape) {
  case WS_ARRY:
    params.fn = arraya;
    break;
  case WS_SIG:
    params.fn = sigmoid;
    break;
  case WS_SIG2:
    params.fn = sigmoid2;
    break;
  case WS_TANH:
    params.fn = hyperbolic_tangent;
    break;
  case WS_ATAN:
    params.fn = arctangent;
    break;
  case WS_FEXP1:
    params.fn = fuzz_exponential_1;
    break;
  case WS_FEXP2:
    params.fn = fuzz_exponential_2;
    break;
  case WS_EXP2:
    params.fn = exponential_2;
    break;
  case WS_ATSR:
    params.fn = atan_sqrt;
    break;
  case WS_SQS:
    params.fn = square_sign;
    break;
  case WS_CUBE:
    params.fn = cube;
    break;
  case WS_HCLIP:
    params.fn = hard_clip;
    break;
  case WS_HWR:
    params.fn = halfwave_rect;
    break;
  case WS_FWR:
    params.fn = fullwave_rect;
    break;
  case WS_SQR:
    params.fn = square_law;
    break;
  case WS_ASQRT:
    params.fn = abs_square_law;
    break;
  default:
    params.fn = square_law;
    break;
  }
  dsp_set_waveshaper_params(&cb->state, params);

  return cb;
}

DSP_callback
dsp_init_waveshaper_default()
{
  waveshaper_params p = {
    .shape = WS_ARRY,
    .saturation = 1.0,
    .asymmetry = -0.5
  };
  return dsp_init_waveshaper(p);
}

