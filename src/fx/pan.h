#ifndef FX_PAN_H
#define FX_PAN_H

#include <stdlib.h>

#include "../lib/macros.h"

typedef struct fx_unit_pan_state_t {
} fx_unit_pan_state;

typedef fx_unit_pan_state *FX_unit_pan_state;

typedef struct fx_unit_pan_params_t {
} fx_unit_pan_params;

typedef struct fx_unit_pan_params_t *FX_unit_pan_params;

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_pan_init(FX_unit_params params);
fx_unit_params fx_unit_pan_default();

#endif
