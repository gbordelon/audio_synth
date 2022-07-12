#include <math.h>

#include "../lib/macros.h"

#include "sin.h"
#include "ugen.h"

const FTYPE B = 4.0 / M_PI;
const FTYPE C = -4.0 / (M_PI * M_PI);
const FTYPE P = 0.225;

// domain is [-pi, pi]
FTYPE
parabolic_sine(FTYPE x)
{
  FTYPE y = B * x + C * x * fabs(x);
  y += P * (y * fabs(y) - y);
  return y;
}

const FTYPE UTSI = 2.0 * M_PI;// / (FTYPE)UGEN_TABLE_SIZE;

// expect phase between [0,1] then scale to [-pi,pi]
FTYPE
ugen_sample_sin_p(Ugen ugen, FTYPE phase_ind)
{
  FTYPE angle = phase_ind * UTSI - M_PI;
  return parabolic_sine(-angle);
}

FTYPE
ugen_sample_sin_table(Ugen ugen, FTYPE phase_ind)
{
  return osc_sin[(size_t)(phase_ind * UGEN_TABLE_SIZE)];
}

FTYPE
ugen_sample_sin(Ugen ugen, FTYPE phase_ind)
{
  return sin(phase_ind * UTSI);
}

void
ugen_generate_table_sin()
{
  int i;
  for (i = 0; i < UGEN_TABLE_SIZE; i++) {
    osc_sin[i] = sin(M_PI * 2 * ((FTYPE)i/(FTYPE)UGEN_TABLE_SIZE));
  }
}
