#include "../lib/macros.h"

#include "audio_filter.h"
#include "dsp.h"
#include "two_band_shelving_filter.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */
FTYPE
mono_shelving_filter(FTYPE *L, dsp_state *state, FTYPE control)
{
  FTYPE Lc = *L, Rc = *L;
  stereo_fx_chain(state->two_band_shelving_filter.filter_head, &Lc, &Rc);
  *L = Lc;

  return control;
}

void
two_band_shelving_filter_cleanup_helper(dsp_state *state)
{
  dsp_cleanup(state->two_band_shelving_filter.filter_head);
}

void
dsp_two_band_shelving_filter_cleanup(DSP_callback cb)
{
  two_band_shelving_filter_cleanup_helper(&cb->state);
  dsp_cleanup(cb);
}

void
dsp_two_band_shelving_filter_set_params(
    dsp_state *state,
    two_band_shelving_filter_params params)
{
  two_band_shelving_filter_cleanup_helper(state);

  if (params.sample_rate < DEFAULT_SAMPLE_RATE) {
    params.sample_rate = DEFAULT_SAMPLE_RATE;
  }
  state->two_band_shelving_filter = params;

  audio_filter_params afp1 = {
    .sample_rate = params.sample_rate,
    .fc = params.high_shelf_fc,
    .q = 0.707,
    .boost_cut_db = params.high_shelf_boost_cut_db,
    .alg = AF_HiShelf
  };

  audio_filter_params afp2 = {
    .sample_rate = params.sample_rate,
    .fc = params.low_shelf_fc,
    .q = 0.707,
    .boost_cut_db = params.low_shelf_boost_cut_db,
    .alg = AF_LowShelf
  };

  state->two_band_shelving_filter.filter_head = dsp_init_audio_filter(afp1);
  DSP_callback head = dsp_init_audio_filter(afp2);
  state->two_band_shelving_filter.filter_head = dsp_add_to_chain(state->two_band_shelving_filter.filter_head, head);
}

DSP_callback
dsp_init_two_band_shelving_filter(two_band_shelving_filter_params params)
{
  DSP_callback cb = dsp_init();
  dsp_two_band_shelving_filter_set_params(&cb->state, params);
  dsp_set_mono_left(cb, mono_shelving_filter);

  return cb;
}

DSP_callback
dsp_init_two_band_shelving_filter_default()
{
  two_band_shelving_filter_params params = {
    .sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE,
    .low_shelf_fc = 400.0,
    .low_shelf_boost_cut_db = 3.0,
    .high_shelf_fc = 4000.0,
    .high_shelf_boost_cut_db = -3.0,
  };
  DSP_callback cb = dsp_init_two_band_shelving_filter(params);

  return cb;
}
