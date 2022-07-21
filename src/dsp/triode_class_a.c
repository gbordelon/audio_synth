#include <stdbool.h>

#include "../lib/macros.h"

#include "audio_filter.h"
#include "dsp.h"
#include "triode_class_a.h"
#include "waveshaper.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 19
 */
FTYPE
stereo_triode_filter(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  stereo_fx_chain(state->triode_class_a.shaper, L, R);

  if (state->triode_class_a.invert_output) {
    *L = -*L;
    *R = -*R;
  }

  if (state->triode_class_a.enable_hpf) {
    stereo_fx_chain(state->triode_class_a.hpf, L, L);
    stereo_fx_chain(state->triode_class_a.hpf, R, R);
  }

  if (state->triode_class_a.enable_lsf) {
    stereo_fx_chain(state->triode_class_a.lsf, L, L);
    stereo_fx_chain(state->triode_class_a.lsf, R, R);
  }

  *L *= state->triode_class_a.output_gain; 
  *R *= state->triode_class_a.output_gain;

  return control;
}

void
triode_class_a_cleanup_helper(dsp_state *state)
{
  if (state->triode_class_a.shaper) {
    dsp_cleanup(state->triode_class_a.shaper);
  }
  if (state->triode_class_a.hpf) {
    dsp_cleanup(state->triode_class_a.hpf);
  }
  if (state->triode_class_a.lsf) {
    dsp_cleanup(state->triode_class_a.lsf);
  }
}

void
dsp_triode_class_a_cleanup(DSP_callback dsp)
{
  triode_class_a_cleanup_helper(&dsp->state);
  dsp_cleanup(dsp);
}

void
dsp_triode_class_a_set_params(
    dsp_state *state,
    triode_class_a_params params)
{
  triode_class_a_cleanup_helper(state);
  state->triode_class_a = params;

  audio_filter_params afp1 = {
    .fc = params.hpf_fc,
    .q = 0.707,
    .boost_cut_db = 0.0,
    .alg = AF_HPF1
  };

  audio_filter_params afp2 = {
    .fc = params.low_shelf_fc,
    .q = 0.707,
    .boost_cut_db = params.low_shelf_boost_cut_db,
    .alg = AF_LowShelf
  };

  state->triode_class_a.shaper = dsp_init_waveshaper(state->triode_class_a.shaper_params);
  state->triode_class_a.hpf = dsp_init_audio_filter(afp1);
  state->triode_class_a.lsf = dsp_init_audio_filter(afp2);
}

DSP_callback
dsp_init_triode_class_a(triode_class_a_params params)
{
  DSP_callback cb = dsp_init();
  dsp_triode_class_a_set_params(&cb->state, params);
  dsp_set_stereo(cb, stereo_triode_filter);

  return cb;
}

DSP_callback
dsp_init_triode_class_a_default()
{
  triode_class_a_params params = {
    .output_gain = pow(10.0, 0.0 / 20.0),
    .invert_output = true,
    .enable_hpf = true,
    .enable_lsf = true,
    .low_shelf_fc = 150.0,
    .low_shelf_boost_cut_db = 0.0,
    .hpf_fc = 4000.0,
    .shaper_params = {
      .shape = WS_SCLIP,
      .saturation = 2.0,
      .asymmetry = 0.0
    }
  };
  DSP_callback cb = dsp_init_triode_class_a(params);

  return cb;
}
