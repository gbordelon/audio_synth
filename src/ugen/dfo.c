#include <math.h>
#include <stdbool.h>
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
dfo_init(FTYPE sample_rate)
{
  Dfo dfo = dfo_alloc();
  dfo->sample_rate = sample_rate;
  dfo->sr_conv = M_PI / sample_rate;
  dfo_set_scale(dfo, -1.0, 1.0);
  return dfo;
}

void
dfo_cleanup(Dfo dfo)
{
  dfo_free(dfo);
}

void
dfo_set_freq(Dfo dfo, FTYPE freq)
{
  dfo->freq = freq;

  FTYPE wT = dfo->sr_conv * freq;
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
dfo_set_sample_rate(Dfo dfo, FTYPE sample_rate)
{
  dfo->sample_rate = sample_rate;
  dfo->sr_conv = M_PI / sample_rate;
  dfo_set_freq(dfo, dfo->freq);
}

void
dfo_sample_biphase(Dfo dfo, FTYPE biphase[DF_NUM_PHASES])
{
  biphase[DF_NORM] = -dfo->coeffs[DF_B1] * dfo->states[DF_YZ1] - dfo->coeffs[DF_B2] * dfo->states[DF_YZ2];
  biphase[DF_INV] = -biphase[DF_NORM];

  dfo->states[DF_YZ2] = dfo->states[DF_YZ1];
  dfo->states[DF_YZ1] = biphase[DF_NORM];
}

void
dfo_set_scale(Dfo dfo, FTYPE low, FTYPE high)
{
  if (high > 1.0) {
    high = 1.0;
  }
  dfo->conv.bias = low;

  if (low < -1.0) {
    low = -1.0;
  }
  dfo->conv.scale = (high - low) * 0.5;
}

FTYPE
dfo_sample(Dfo dfo)
{
  FTYPE biphase[DF_NUM_PHASES];
  dfo_sample_biphase(dfo, biphase);

  biphase[DF_NORM] = dfo->conv.bias + dfo->conv.scale * (biphase[DF_NORM] + 1.0);

  return biphase[DF_NORM];
}
