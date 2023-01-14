#ifndef FX_BITCRUSHER_H
#define FX_BITCRUSHER_H

#include <stdlib.h>

#include "../lib/macros.h"

typedef struct fx_unit_bitcrusher_state_t {
  FTYPE quantized_bit_depth;
} fx_unit_bitcrusher_state;

typedef fx_unit_bitcrusher_state *FX_unit_bitcrusher_state;

typedef struct fx_unit_bitcrusher_params_t {
  FTYPE quantized_bit_depth;
} fx_unit_bitcrusher_params;

typedef struct fx_unit_bitcrusher_params_t *FX_unit_bitcrusher_params;

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_bitcrusher_init(FX_unit_params params);
fx_unit_params fx_unit_bitcrusher_default();

#endif
