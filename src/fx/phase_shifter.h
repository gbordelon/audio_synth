#ifndef FX_PHASE_SHIFTER_H
#define FX_PHASE_SHIFTER_H

#include <stdlib.h>

#include "../lib/macros.h"

#define PHASER_STAGES 6

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef struct fx_compound_unit_t *FX_compound_unit;
typedef int16_t fx_unit_idx;

typedef struct fx_unit_phase_shifter_state_t {
  FX_unit_params apf_params;
  fx_unit_idx apf[PHASER_STAGES];
  fx_unit_idx lfo;
  fx_unit_idx passthru;
  fx_unit_idx control_joiner;
  fx_unit_idx sum;
  // adjust in the lfo by calling ugen_set_scale()
  //double lfo_scale; // [0,1] "depth"
  double intensity; // 0.75 to 0.95 optimal "feedback"
} fx_unit_phase_shifter_state;

typedef fx_unit_phase_shifter_state *FX_unit_phase_shifter_state;

typedef struct fx_unit_phase_shifter_params_t {
  double intensity; // 0.75 to 0.95 optimal "feedback"
} fx_unit_phase_shifter_params;

typedef fx_unit_phase_shifter_params *FX_unit_phase_shifter_params;

FX_compound_unit fx_compound_unit_phase_shifter_init(FX_unit_params params, FX_unit_params signal_source_params);

fx_unit_params fx_unit_phase_shifter_default();
fx_unit_params fx_unit_phase_shifter_signal_source_default();

#endif
