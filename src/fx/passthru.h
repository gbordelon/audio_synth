#ifndef FX_PASSTHRU_H
#define FX_PASSTHRU_H

#include <stdlib.h>

#include "../lib/macros.h"

typedef struct fx_unit_passthru_state_t {
} fx_unit_passthru_state;

typedef fx_unit_passthru_state *FX_unit_passthru_state;

typedef struct fx_unit_passthru_params_t {
} fx_unit_passthru_params;

typedef struct fx_unit_passthru_params_t *FX_unit_passthru_params;

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef struct fx_compound_unit_t *FX_compound_unit;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_passthru_init(FX_unit_params params);

FX_compound_unit fx_compound_unit_passthru_init(FX_unit_params params);

fx_unit_params fx_unit_passthru_default();

#endif
