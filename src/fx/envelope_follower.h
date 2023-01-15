#ifndef FX_ENVELOPE_FOLLOWER_H
#define FX_ENVELOPE_FOLLOWER_H

#include <stdlib.h>

#include "../lib/macros.h"

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

typedef struct fx_unit_envelope_follower_state_t {
  fx_unit_idx detector;
  fx_unit_idx filter;
  fx_unit_idx s2m;
  FTYPE fc;
  FTYPE fc_max;
  FTYPE threshold; // linear, not dB
  FTYPE sensitivity; // 0.25 to 10.0
  FX_unit_params filter_params;
} fx_unit_envelope_follower_state;

typedef fx_unit_envelope_follower_state *FX_unit_envelope_follower_state;

typedef struct fx_unit_envelope_follower_params_t {
  FTYPE fc;
  FTYPE fc_max;
  FTYPE threshold; // linear, not dB
  FTYPE sensitivity; // 0.25 to 10.0
  FX_unit_params detector_params;
  FX_unit_params filter_params;
  FX_unit_params s2m_params;
} fx_unit_envelope_follower_params;

typedef struct fx_unit_envelope_follower_params_t *FX_unit_envelope_follower_params;

fx_unit_idx fx_unit_envelope_follower_init(FX_unit_params params);
fx_unit_params fx_unit_envelope_follower_default();
void fx_unit_envelope_follower_params_free(FX_unit_params env);
fx_unit_idx fx_unit_envelope_follower_set_parent(fx_unit_idx env, fx_unit_idx parent);

#endif
