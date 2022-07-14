#include <math.h>

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#include "comb_filter.h"
#include "dsp.h"
#include "simple_delay.h"
#include "simple_lpf.h"

FTYPE
stereo_tester(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  FTYPE yn;
/*
  yn = simple_delay_read_at_percentage(state->tester.delay, 50.0);
  simple_delay_write(state->tester.delay, *L);
*/
/*
  yn = simple_lpf_process(state->tester.lpf, *L);
  *L = yn;

  yn = simple_lpf_process(state->tester.lpf, *R);
  *R = yn;
*/
/*
  yn = *L * 0.5 + *R * 0.5;
  yn = comb_filter_process(state->tester.comb, yn);
  *L = *L * 0.707 + yn * 0.707;
  *R = *R * 0.707 + yn * 0.707;
*/
  yn = *L * 0.5 + *R * 0.5;
  yn = delay_apf_process(state->tester.apf, yn);
  *L = *L + yn * pow(10.0, -9.0 / 20.0);
  *R = *R + yn * pow(10.0, -9.0 / 20.0);
  return control;
}

// TODO cleanup before assigning
void
dsp_set_tester_params(dsp_state *state, tester_params params)
{
  state->tester = params;
}

DSP_callback
dsp_init_tester(tester_params params)
{
  DSP_callback cb = dsp_init();
  dsp_set_stereo(cb, stereo_tester);
  dsp_set_tester_params(&cb->state, params);

  return cb;
}

DSP_callback
dsp_init_tester_default()
{
  tester_params params = {
    .delay = simple_delay_init(500.0 / 1000.0 * (FTYPE)DEFAULT_SAMPLE_RATE),
    .lpf = simple_lpf_init(0.99),
    .comb = comb_filter_init(100.0 / 1000.0 * (FTYPE)DEFAULT_SAMPLE_RATE, 500.0 / 1000.0 * (FTYPE)DEFAULT_SAMPLE_RATE, 0.8),
    .apf = delay_apf_init_default(),
  };
  DSP_callback cb = dsp_init_tester(params);
  return cb;
}
