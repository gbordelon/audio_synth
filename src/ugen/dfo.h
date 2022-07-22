#ifndef DFO_H
#define DFO_H

#include "../lib/macros.h"

typedef enum {
  DF_NORM,
  DF_INV,
  DF_NUM_PHASES
} dfo_phase;

typedef enum {
  DF_B1,
  DF_B2,
  DF_NUM_COEFFS
} dfo_coeffs;

typedef enum {
  DF_YZ1,
  DF_YZ2,
  DF_NUM_STATES
} dfo_states;

typedef struct dfo_t {
  FTYPE sample_rate;
  FTYPE sr_conv;
  FTYPE coeffs[DF_NUM_COEFFS];
  FTYPE states[DF_NUM_STATES];
} *Dfo;

Dfo dfo_init(FTYPE smaple_rate);
void dfo_cleanup(Dfo dfo);

void dfo_set_freq(Dfo dfo, FTYPE freq);
FTYPE dfo_sample(Dfo dfo);

#endif
