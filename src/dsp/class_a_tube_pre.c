#include <stdbool.h>

#include "../lib/macros.h"

#include "audio_filter.h"
#include "class_a_tube_pre.h"
#include "dsp.h"
#include "triode_class_a.h"
#include "waveshaper.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 19
 */
FTYPE
stereo_tube_pre(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  FTYPE input_L = *L * state->class_a_tube_pre.input_level;
  FTYPE input_R = *R * state->class_a_tube_pre.input_level;
  stereo_fx_chain(state->class_a_tube_pre.triodes[0], &input_L, &input_R);
  stereo_fx_chain(state->class_a_tube_pre.triodes[1], &input_L, &input_R);
  stereo_fx_chain(state->class_a_tube_pre.triodes[2], &input_L, &input_R);

  stereo_fx_chain(state->class_a_tube_pre.shelving_filter, &input_L, &input_L);
  stereo_fx_chain(state->class_a_tube_pre.shelving_filter, &input_R, &input_R);

  stereo_fx_chain(state->class_a_tube_pre.triodes[3], &input_L, &input_R);
  *L = input_L * state->class_a_tube_pre.output_level;
  *R = input_R * state->class_a_tube_pre.output_level;
  return control;
}

void
class_a_tube_pre_cleanup_helper(dsp_state *state)
{
  if (state->class_a_tube_pre.shelving_filter) {
    dsp_two_band_shelving_filter_cleanup(state->class_a_tube_pre.shelving_filter);
  }
  int i;
  for (i = 0; i < NUM_TUBES; i++) {
    if (state->class_a_tube_pre.triodes[i]) {
      dsp_triode_class_a_cleanup(state->class_a_tube_pre.triodes[i]);
    }
  }
}

void
dsp_class_a_tube_pre_cleanup(DSP_callback dsp)
{
  class_a_tube_pre_cleanup_helper(&dsp->state);
  dsp_cleanup(dsp);
}

void
dsp_class_a_tube_pre_set_params(
    dsp_state *state,
    class_a_tube_pre_params params)
{
  class_a_tube_pre_cleanup_helper(state);
  state->class_a_tube_pre = params;

  state->class_a_tube_pre.shelving_filter = dsp_init_two_band_shelving_filter(state->class_a_tube_pre.filter_params);
  int i;
  for (i = 0; i < NUM_TUBES; i++) {
    state->class_a_tube_pre.triodes[i] = dsp_init_triode_class_a(state->class_a_tube_pre.tube_params);
  }
}

DSP_callback
dsp_init_class_a_tube_pre(class_a_tube_pre_params params)
{
  DSP_callback cb = dsp_init();
  dsp_class_a_tube_pre_set_params(&cb->state, params);
  dsp_set_stereo(cb, stereo_tube_pre);

  return cb;
}

DSP_callback
dsp_init_class_a_tube_pre_default()
{
  class_a_tube_pre_params params = {
    .input_level = pow(10.0, -6.0 / 20.0),
    .output_level = pow(10.0, -6.0 / 20.0),
    .tube_params = {
      .output_gain = 1.0,
      .invert_output = true,
      .enable_hpf = true,
      .enable_lsf = true,
      .low_shelf_fc = 88.0,
      .low_shelf_boost_cut_db = -12.0,
      .hpf_fc = 1.0, // removes a DC bias
      .shaper_params = {
        .shape = WS_FEXP1, //WS_TANH, //WS_ATAN, //WS_SCLIP,
        .saturation = 2.0,
        .asymmetry = -0.6
      }
    },
    .filter_params = {
      .low_shelf_fc = 150.0,
      .low_shelf_boost_cut_db = 0.0,
      .high_shelf_fc = 4000.0,
      .high_shelf_boost_cut_db = 0.0,
    }
  };
  DSP_callback cb = dsp_init_class_a_tube_pre(params);

  return cb;
}
