#ifndef FX_CONTROL_JOINER_H
#define FX_CONTROL_JOINER_H

#include <stdlib.h>

#include "../lib/macros.h"

typedef struct fx_unit_control_joiner_state_t {
} fx_unit_control_joiner_state;

typedef fx_unit_control_joiner_state *FX_unit_control_joiner_state;

typedef struct fx_unit_control_joiner_params_t {
} fx_unit_control_joiner_params;

typedef struct fx_unit_control_joiner_params_t *FX_unit_control_joiner_params;

// forward decl
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_control_joiner_init(FX_unit_params params);

#endif
