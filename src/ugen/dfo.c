#include <math.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "dfo.h"

Dfo
dfo_alloc()
{
  return calloc(1, sizeof(struct dfo_t));
}

void
dfo_free(Dfo dfo)
{
  free(dfo);
}

Dfo
dfo_init()
{
  return dfo_alloc();
}

void
dfo_cleanup(Dfo dfo)
{
  dfo_free(dfo);
}

static const FTYPE HZ_TO_THETA = M_PI / (FTYPE)DEFAULT_SAMPLE_RATE;
void
dfo_set_freq(Dfo dfo, FTYPE freq)
{
  FTYPE wT = HZ_TO_THETA * freq;
  dfo->coeffs[DF_B1] = -2.0 * cos(wT);
  dfo->coeffs[DF_B2] = 1.0;

  FTYPE wnT1 = asin(dfo->states[DF_YZ1]);
  FTYPE n = wnT1 / wT;

  if (dfo->states[DF_YZ1] > dfo->states[DF_YZ2]) {
    n -= 1.0;
  } else {
    n += 1.0;
  }

  dfo->states[DF_YZ2] = sin(n * wT);
}

void
dfo_sample_biphase(Dfo dfo, FTYPE biphase[DF_NUM_PHASES])
{
  biphase[DF_NORM] = -dfo->coeffs[DF_B1] * dfo->states[DF_YZ1] - dfo->coeffs[DF_B2] * dfo->states[DF_YZ2];
  biphase[DF_INV] = -biphase[DF_NORM];

  dfo->states[DF_YZ2] = dfo->states[DF_YZ1];
  dfo->states[DF_YZ1] = biphase[DF_NORM];
}

FTYPE
dfo_sample(Dfo dfo)
{
  FTYPE biphase[DF_NUM_PHASES];
  dfo_sample_biphase(dfo, biphase);

  return biphase[DF_NORM];
}
