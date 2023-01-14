#include "../lib/macros.h"

#include "biquad.h"

void
biquad_set_params(biquad_params *params, biquad_algorithm alg)
{
  params->alg = alg;
}

#define a0  (params->coeffs[BIQUAD_a0])
#define a1  (params->coeffs[BIQUAD_a1])
#define a2  (params->coeffs[BIQUAD_a2])
#define b1  (params->coeffs[BIQUAD_b1])
#define b2  (params->coeffs[BIQUAD_b2])
#define xn  sample
#define xz1 (params->states[BIQUAD_x_z1])
#define xz2 (params->states[BIQUAD_x_z2])
#define yz1 (params->states[BIQUAD_y_z1])
#define yz2 (params->states[BIQUAD_y_z2])
FTYPE
biquad_process_sample(biquad_params *params, FTYPE sample)
{
  FTYPE yn = xn;
  FTYPE wn;

  switch (params->alg) {
  case BIQUAD_CANONICAL:
    wn = xn - b1 * xz1 - b2 * xz2;
    yn = a0 * wn + a1 * xz1 + a2 * xz2;

    xz2 = xz1;
    xz1 = wn;

    break;
  case BIQUAD_TRANSPOSE_DIRECT:
    wn = xn + yz1;
    yn = a0 * wn + xz1;

    yz1 = yz2 - b1 * wn;
    yz2 = -b2 * wn;
    xz1 = xz2 + a1 * wn;
    xz2 = a2 * wn;

    break;
  case BIQUAD_TRANSPOSE_CANONICAL:
    yn = a0 * xn + xz1;

    xz1 = a1 * xn - b1 * yn + xz2;
    xz2 = a2 * xn - b2 * yn;

    break;
  default:
    // fall through
  case BIQUAD_DIRECT:
    yn = a0 * xn +
         a1 * xz1 +
         a2 * xz2 -
         b1 * yz1 -
         b2 * yz2;

    xz2 = xz1;
    xz1 = xn;
    yz2 = yz1;
    yz1 = yn;
    break;
  }

  return yn;
}

FTYPE
biquad_get_G(biquad_params *params)
{
  return a0;
}

FTYPE
biquad_get_S(biquad_params *params)
{
  FTYPE S = 0;
  switch (params->alg) {
  case BIQUAD_CANONICAL:
    // undef
    break;
  case BIQUAD_TRANSPOSE_DIRECT:
    // undef
    break;
  case BIQUAD_TRANSPOSE_CANONICAL:
    S = a1 * xz1 +
        a2 * xz2 -
        b1 * yz1 -
        b2 * yz2;
    break;
  default:
    // fall through
  case BIQUAD_DIRECT:
    S = a1 * xz1 +
        a2 * xz2 -
        b1 * yz1 -
        b2 * yz2;
    break;
  }
  return S;
}

#undef a0
#undef a1
#undef a2
#undef b1
#undef b2
#undef xn
#undef xz1
#undef xz2
#undef yz1
#undef yz2
