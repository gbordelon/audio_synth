#include <math.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_filter.h"
#include "biquad.h"
#include "dsp.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Prikle, chapter 11
 */

#define coeffs (params->biquad.coeffs)
void
calculate_filter_coefficients(audio_filter_params *params)
{
  double alpha,
    beta,
    beta_denominator,
    beta_numerator,
    d,
    gamma,
    theta_c;

  memset(coeffs, 0, sizeof(double) * BIQUAD_NUM_COEFF);

  // default passthrough
  coeffs[BIQUAD_a0] = 1.0;
  coeffs[BIQUAD_c0] = 1.0;
  coeffs[BIQUAD_d0] = 0.0;

  switch (params->alg) {
  case AF_LPF2:
    theta_c = 2.0 * M_PI * params->fc / (double)DEFAULT_SAMPLE_RATE;
    d = 1.0 / params->q;
    beta_numerator = 1.0 - ((d / 2.0) * sin(theta_c));
    beta_denominator = 1.0 + ((d / 2.0) * sin(theta_c));
    beta = 0.5 * beta_numerator / beta_denominator;
    gamma = (0.5 + beta) * cos(theta_c);
    alpha = (0.5 + beta - gamma) / 2.0;

    coeffs[BIQUAD_a0] = alpha;
    coeffs[BIQUAD_a1] = 2.0 * alpha;
    coeffs[BIQUAD_a2] = alpha;
    coeffs[BIQUAD_b1] = -2.0 * gamma;
    coeffs[BIQUAD_b2] = 2.0 * beta;

    break;
  default:
    // fall through
  case AF_LPF1:
    theta_c = 2.0 * M_PI * params->fc / (double)DEFAULT_SAMPLE_RATE;
    gamma = cos(theta_c) / (1.0 + sin(theta_c));

    coeffs[BIQUAD_a0] = (1.0 - gamma) / 2.0;
    coeffs[BIQUAD_a1] = (1.0 - gamma) / 2.0;
    coeffs[BIQUAD_a2] = 0.0;
    coeffs[BIQUAD_b1] = -gamma;
    coeffs[BIQUAD_b2] = 0.0;
  
    break;
  }
}
#undef coeffs

#define coeffs (state->audio_filter.biquad.coeffs)
void
mono_filter(FTYPE *L, dsp_state *state, FTYPE control)
{
  *L = coeffs[BIQUAD_d0] * *L +
       coeffs[BIQUAD_c0] * biquad_process_sample(&state->audio_filter.biquad, *L);
}
#undef coeffs

void
dsp_audio_filter_set_params(
    dsp_state *state,
    audio_filter_algorithm alg,
    double fc,
    double q,
    double boost_cut_db)
{
  state->audio_filter.alg = alg;
  state->audio_filter.fc = fc;
  state->audio_filter.q = q;
  state->audio_filter.boost_cut_db = boost_cut_db;

  if (q <= 0) {
    state->audio_filter.q = 0.707;
  }

  calculate_filter_coefficients(&state->audio_filter);
}

DSP_callback
dsp_init_audio_filter()
{
  DSP_callback cb = dsp_init();
  cb->fn_type = DSP_MONO_L;
  cb->fn_u.mono = mono_filter;
  dsp_audio_filter_set_params(&cb->state, AF_LPF1, 100.0, 0.707, 0.0);

  return cb;
}
