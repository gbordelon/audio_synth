#ifndef FX_SUM_H
#define FX_SUM_H

#include <stdlib.h>

#include "../lib/macros.h"

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef struct fx_compound_unit_t *FX_compound_unit;
typedef int16_t fx_unit_idx;

typedef struct fx_unit_sum_state_t {
  FTYPE *mix;
} fx_unit_sum_state;

typedef fx_unit_sum_state *FX_unit_sum_state;

typedef struct fx_unit_sum_params_t {
  FTYPE *mix;
} fx_unit_sum_params;

typedef struct fx_unit_sum_params_t *FX_unit_sum_params;

fx_unit_idx fx_unit_sum_init(FX_unit_params params);

FX_compound_unit fx_compound_unit_sum_init(FX_unit_params sum_p);

fx_unit_params fx_unit_sum_default();

void fx_unit_sum_mix_set(fx_unit_idx idx, FTYPE *mix);

#endif
