#include <math.h>
#include <stdio.h>

#include "../lib/macros.h"

#include "waveshaper.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 * Based on equations from Designing Audio Effect Plugins in C++ by Pirkle, chapter 19
 */

void
fx_unit_waveshaper_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define state (fx_unit_head[idx].state.u.waveshaper)
  dst[FX_L] = state.fn(src[FX_L], &state);
  dst[FX_R] = state.fn(src[FX_R], &state);
  dst[FX_C] = src[FX_C];
#undef state
#undef src
#undef dst
}

void
fx_unit_waveshaper_cleanup(FX_unit_state state)
{
  // do nothing
}

static const FTYPE C_1_3 = 1.0 / 3.0;
static const FTYPE C_E_1 = M_E + 1.0;
static const FTYPE C_1_E_1 = 1.0 / (M_E - 1.0);

FTYPE
arraya(FTYPE x, FX_unit_waveshaper_state p)
{
  return 1.5 * x * (1.0 - x * x * C_1_3);
}

FTYPE
sigmoid(FTYPE x, FX_unit_waveshaper_state p)
{
  return 2.0 / (1 + exp(-p->saturation * x)) - 1.0;
}

FTYPE
sigmoid2(FTYPE x, FX_unit_waveshaper_state p)
{
  FTYPE e = exp(x);
  return (e - 1.0) * C_E_1 * C_1_E_1 / (e + 1.0);
}

FTYPE
hyperbolic_tangent(FTYPE x, FX_unit_waveshaper_state p)
{
  return tanh(p->saturation * x) * p->tanh_k;
}

FTYPE
arctangent(FTYPE x, FX_unit_waveshaper_state p)
{
  return atan(p->saturation * x) * p->atan_k;
}

FTYPE
soft_clip(FTYPE x, FX_unit_waveshaper_state p)
{
  return sgn(x) * (1.0 - exp(-fabs(p->saturation * x)));
}

inline FTYPE
ws_gain(FTYPE x, FTYPE saturation, FTYPE asymmetry)
{
  return ((x >= 0.0 && asymmetry > 0.0) || (x < 0.0 && asymmetry < 0.0)) ? saturation * (1.0 + 4.0*fabs(asymmetry)) : saturation;
}

FTYPE
fuzz_exponential_1(FTYPE x, FX_unit_waveshaper_state p)
{
  FTYPE g = ws_gain(x, p->saturation, p->asymmetry);
  return sgn(x) * (1.0 - exp(-fabs(g * x))) / (1.0 - exp(-g));
}

FTYPE
fuzz_exponential_2(FTYPE x, FX_unit_waveshaper_state p)
{
  return sgn(-x) * (1.0 - exp(fabs(x))) * C_1_E_1;
}

FTYPE
exponential_2(FTYPE x, FX_unit_waveshaper_state p)
{
  return (M_E - exp(1.0 - x)) * C_1_E_1;
}

FTYPE
atan_sqrt(FTYPE x, FX_unit_waveshaper_state p)
{
  FTYPE xs = x * 0.9;
  return 2.5 * (atan(xs) + sqrt(1.0 - xs * xs) - 1.0);
}

FTYPE
square_sign(FTYPE x, FX_unit_waveshaper_state p)
{
  return sgn(x) * x * x;
}

FTYPE
cube(FTYPE x, FX_unit_waveshaper_state p)
{
  return x * x * x;
}

FTYPE
hard_clip(FTYPE x, FX_unit_waveshaper_state p)
{
  return (fabs(x) > 0.5) ? 0.5 * sgn(x) : x;
}

FTYPE
halfwave_rect(FTYPE x, FX_unit_waveshaper_state p)
{
  return 0.5 * (x + fabs(x));
}

FTYPE
fullwave_rect(FTYPE x, FX_unit_waveshaper_state p)
{
  return fabs(x);
}

FTYPE
square_law(FTYPE x, FX_unit_waveshaper_state p)
{
  return x * x;
}

FTYPE
abs_square_law(FTYPE x, FX_unit_waveshaper_state p)
{
  return sqrt(fabs(x));
}

void
fx_unit_waveshaper_set_params(FX_unit_state state, FX_unit_params params)
{
  if (params->u.waveshaper.saturation < 1.0) {
    params->u.waveshaper.saturation = 1.0;
  }

  state->u.waveshaper.shape = params->u.waveshaper.shape;
  state->u.waveshaper.saturation = params->u.waveshaper.saturation;
  state->u.waveshaper.asymmetry = params->u.waveshaper.asymmetry;
  state->u.waveshaper.tanh_k = 1.0;
  state->u.waveshaper.atan_k = 1.0;

  if (state->u.waveshaper.shape == WS_TANH) {
    state->u.waveshaper.tanh_k = 1.0 / tanh(state->u.waveshaper.saturation);
  } else if (state->u.waveshaper.shape == WS_ATAN) {
    state->u.waveshaper.atan_k = 1.0 / atan(state->u.waveshaper.saturation);
  }

  switch (state->u.waveshaper.shape) {
  case WS_ARRY:
    state->u.waveshaper.fn = arraya;
    break;
  case WS_SIG:
    state->u.waveshaper.fn = sigmoid;
    break;
  case WS_SIG2:
    state->u.waveshaper.fn = sigmoid2;
    break;
  case WS_TANH:
    state->u.waveshaper.fn = hyperbolic_tangent;
    break;
  case WS_ATAN:
    state->u.waveshaper.fn = arctangent;
    break;
  case WS_SCLIP:
    state->u.waveshaper.fn = soft_clip;
    break;
  case WS_FEXP1:
    state->u.waveshaper.fn = fuzz_exponential_1;
    break;
  case WS_FEXP2:
    state->u.waveshaper.fn = fuzz_exponential_2;
    break;
  case WS_EXP2:
    state->u.waveshaper.fn = exponential_2;
    break;
  case WS_ATSR:
    state->u.waveshaper.fn = atan_sqrt;
    break;
  case WS_SQS:
    state->u.waveshaper.fn = square_sign;
    break;
  case WS_CUBE:
    state->u.waveshaper.fn = cube;
    break;
  case WS_HCLIP:
    state->u.waveshaper.fn = hard_clip;
    break;
  case WS_HWR:
    state->u.waveshaper.fn = halfwave_rect;
    break;
  case WS_FWR:
    state->u.waveshaper.fn = fullwave_rect;
    break;
  case WS_SQR:
    state->u.waveshaper.fn = square_law;
    break;
  case WS_ASQRT:
    state->u.waveshaper.fn = abs_square_law;
    break;
  default:
    state->u.waveshaper.fn = square_law;
    break;
  }
}

void
fx_unit_waveshaper_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_waveshaper_set_params(state, params);
}

fx_unit_idx
fx_unit_waveshaper_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_waveshaper_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_waveshaper_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_waveshaper_reset;

  fx_unit_waveshaper_reset(&fx_unit_head[idx].state, params);
  return idx;
}

FX_compound_unit
fx_compound_unit_waveshaper_init(FX_unit_params params)
{
  FX_compound_unit rv = fx_compound_unit_init(1, 1);
  fx_unit_idx waveshaper = fx_unit_waveshaper_init(params);

  rv->units[0] = waveshaper;
  rv->heads[0] = waveshaper;
  rv->tail = waveshaper;

  return rv;
}

fx_unit_params
fx_unit_waveshaper_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_WAVESHAPER;
  params.u.waveshaper.shape = WS_FEXP1;
  params.u.waveshaper.saturation = 0.707;
  params.u.waveshaper.asymmetry = -0.5;

  return params;
}
