#ifndef PHASE_SHIFTER_H
#define PHASE_SHIFTER_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#include "audio_filter.h"

#define PHASER_STAGES 6

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;
typedef struct dsp_callback_t *DSP_callback;

typedef struct phase_shifter_t {
  audio_filter_params filters[PHASER_STAGES];
  DSP_callback apfs[PHASER_STAGES];
  Ugen lfo;
  double lfo_rate;
  double lfo_scale; // [0,1] "depth"
  double intensity; // 0.75 to 0.95 optimal "feedback"
  bool quad_phase_lfo;
} phase_shifter_params;

void dsp_phase_shifter_set_params(dsp_state *state, phase_shifter_params params);
void dsp_phase_shifter_cleanup(DSP_callback ps);

#endif
