#ifndef FX_S2M_H
#define FX_S2M_H

#include <stdbool.h>
#include <stdlib.h>

#include "../lib/macros.h"

typedef struct fx_unit_s2m_state_t {
  bool left; // true if output should be left channel, false for right channel
} fx_unit_s2m_state;

typedef fx_unit_s2m_state *FX_unit_s2m_state;

typedef struct fx_unit_s2m_params_t {
  bool left;
} fx_unit_s2m_params;

typedef struct fx_unit_s2m_params_t *FX_unit_s2m_params;

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_s2m_init(FX_unit_params params);
fx_unit_params fx_unit_s2m_default();

#endif
