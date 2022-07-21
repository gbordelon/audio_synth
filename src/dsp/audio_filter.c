#include <math.h>
#include <string.h>

#include "../lib/macros.h"

#include "audio_filter.h"
#include "biquad.h"
#include "dsp.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 11
 */

#define coeffs (params->biquad.coeffs)
void
calculate_filter_coefficients(audio_filter_params *params)
{
  FTYPE A_0,
    A_1,
    A_2,
    alpha,
    B_0,
    B_1,
    B_2,
    beta,
    beta_denominator,
    beta_numerator,
    c,
    c_im,
    d,
    delta,
    d_0,
    e_0,
    eta,
    f_0,
    g,
    gamma,
    kappa,
    mu,
    omega_c,
    p_im,
    p_real,
    phi_0,
    phi_1,
    phi_2,
    q,
    r,
    r_0,
    r_1,
    r_2,
    resonance,
    sigma,
    t,
    theta_c,
    v_0;

  memset(coeffs, 0, sizeof(FTYPE) * BIQUAD_NUM_COEFF);

  // default passthrough
  coeffs[BIQUAD_a0] = 1.0;
  coeffs[BIQUAD_c0] = 1.0;
  coeffs[BIQUAD_d0] = 0.0;

  switch (params->alg) {
  case AF_LPF1P: // pg 282 first order all pole
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    gamma = 2 - cos(theta_c);
    coeffs[BIQUAD_b1] = sqrt(gamma * gamma - 1.0) - gamma;
    coeffs[BIQUAD_a0] = 1.0 - coeffs[BIQUAD_b1];

    break;
  case AF_HPF1: // pg 271
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    gamma = cos(theta_c) / (1.0 + sin(theta_c));

    coeffs[BIQUAD_a0] = (1.0 + gamma) * 0.5;
    coeffs[BIQUAD_a1] = -(1.0 + gamma) * 0.5;
    coeffs[BIQUAD_a2] = 0.0;
    coeffs[BIQUAD_b1] = -gamma;
    coeffs[BIQUAD_b2] = 0.0;
  
    break;
  case AF_LPF2: // pg 272 second order low pass
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    d = 1.0 / params->q;
    beta_numerator = 1.0 - ((d * 0.5) * sin(theta_c));
    beta_denominator = 1.0 + ((d * 0.5) * sin(theta_c));
    beta = 0.5 * beta_numerator / beta_denominator;
    gamma = (0.5 + beta) * cos(theta_c);
    alpha = (0.5 + beta - gamma) * 0.5;

    coeffs[BIQUAD_a0] = alpha;
    coeffs[BIQUAD_a1] = 2.0 * alpha;
    coeffs[BIQUAD_a2] = alpha;
    coeffs[BIQUAD_b1] = -2.0 * gamma;
    coeffs[BIQUAD_b2] = 2.0 * beta;

    break;
  case AF_HPF2: // pg 272 second order low pass
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    d = 1.0 / params->q;
    beta_numerator = 1.0 - ((d * 0.5) * sin(theta_c));
    beta_denominator = 1.0 + ((d * 0.5) * sin(theta_c));
    beta = 0.5 * beta_numerator / beta_denominator;
    gamma = (0.5 + beta) * cos(theta_c);
    alpha = (0.5 + beta + gamma) * 0.5;

    coeffs[BIQUAD_a0] = alpha;
    coeffs[BIQUAD_a1] = -2.0 * alpha;
    coeffs[BIQUAD_a2] = alpha;
    coeffs[BIQUAD_b1] = -2.0 * gamma;
    coeffs[BIQUAD_b2] = 2.0 * beta;

    break;
  case AF_BPF2: // pg 273
    kappa = tan(M_PI * params->fc * params->sample_rate_i);
    delta = kappa * kappa * params->q + kappa + params->q;

    coeffs[BIQUAD_a0] = kappa / delta;
    coeffs[BIQUAD_a1] = 0.0;
    coeffs[BIQUAD_a2] = -kappa / delta;
    coeffs[BIQUAD_b1] = 2.0 * params->q * (kappa * kappa - 1.0) / delta;
    coeffs[BIQUAD_b2] = (kappa * kappa * params->q - kappa + params->q) / delta;

    break;
  case AF_BSF2: // pg 273
    kappa = tan(M_PI * params->fc * params->sample_rate_i);
    delta = kappa * kappa * params->q + kappa + params->q;

    coeffs[BIQUAD_a0] = params->q * (kappa * kappa + 1) / delta;
    coeffs[BIQUAD_a1] = 2.0 * params->q * (kappa * kappa - 1) / delta;
    coeffs[BIQUAD_a2] = params->q * (kappa * kappa + 1) / delta;
    coeffs[BIQUAD_b1] = 2.0 * params->q * (kappa * kappa - 1.0) / delta;
    coeffs[BIQUAD_b2] = (kappa * kappa * params->q - kappa + params->q) / delta;

    break;
  case AF_ButterLPF2: // pg 273
    c = 1.0 / tan(M_PI * params->fc * params->sample_rate_i);

    coeffs[BIQUAD_a0] = 1.0 / (1.0 + M_SQRT2 * c + c * c);
    coeffs[BIQUAD_a1] = 2.0 * coeffs[BIQUAD_a0];
    coeffs[BIQUAD_a2] = coeffs[BIQUAD_a0];
    coeffs[BIQUAD_b1] = 2.0 * coeffs[BIQUAD_a0] * (1.0 - c * c);
    coeffs[BIQUAD_b2] = coeffs[BIQUAD_a0] * (1.0 - M_SQRT2 * c + c * c);

    break;
  case AF_ButterHPF2: // pg 273
    c = tan(M_PI * params->fc * params->sample_rate_i);

    coeffs[BIQUAD_a0] = 1.0 / (1.0 + M_SQRT2 * c + c * c);
    coeffs[BIQUAD_a1] = -2.0 * coeffs[BIQUAD_a0];
    coeffs[BIQUAD_a2] = coeffs[BIQUAD_a0];
    coeffs[BIQUAD_b1] = 2.0 * coeffs[BIQUAD_a0] * (c * c - 1.0);
    coeffs[BIQUAD_b2] = coeffs[BIQUAD_a0] * (1.0 - M_SQRT2 * c + c * c);

    break;
  case AF_ButterBPF2: // pg 274
    c = 1.0 / tan(M_PI * params->fc * params->fc * params->sample_rate_i / params->q);
    d = 2.0 * cos(2.0 * M_PI * params->fc * params->sample_rate_i);

    coeffs[BIQUAD_a0] = 1.0 / (1.0 + c);
    coeffs[BIQUAD_a1] = 0.0;
    coeffs[BIQUAD_a2] = -coeffs[BIQUAD_a0];
    coeffs[BIQUAD_b1] = -coeffs[BIQUAD_a0] * c * d;
    coeffs[BIQUAD_b2] = coeffs[BIQUAD_a0] * (c - 1.0);

    break;
  case AF_ButterBSF2: // pg 274
    c = tan(M_PI * params->fc * params->fc * params->sample_rate_i / params->q);
    d = 2.0 * cos(2.0 * M_PI * params->fc * params->sample_rate_i);

    coeffs[BIQUAD_a0] = 1.0 / (1.0 + c);
    coeffs[BIQUAD_a1] = -coeffs[BIQUAD_a0] * d;
    coeffs[BIQUAD_a2] = coeffs[BIQUAD_a0];
    coeffs[BIQUAD_b1] = -coeffs[BIQUAD_a0] * d;
    coeffs[BIQUAD_b2] = coeffs[BIQUAD_a0] * (1.0 - c);

    break;
  case AF_MMALPF2: // pg 284 MIDI
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    resonance = params->q <= 0.707
              ? 0.0
              : 20.0 * log10(params->q * params->q / sqrt(params->q * params->q - 0.25));
    r = (cos(theta_c) + sin(theta_c) * sqrt(pow(10.0, resonance / 10.0) - 1.0)) /
        (pow(10.0, resonance / 20.0) * sin(theta_c) + 1.0);
    g = pow(10.0, -params->boost_cut_db / 40.0);

    coeffs[BIQUAD_b1] = -2.0 * r * cos(theta_c);
    coeffs[BIQUAD_b2] = r * r;
    coeffs[BIQUAD_a0] = g * (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]);
    coeffs[BIQUAD_a1] = 0.0;
    coeffs[BIQUAD_a2] = 0.0;

    break;
  case AF_MMALPF2B: // pg
    break;
  case AF_LowShelf: // pg 278
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    mu = pow(10.0, params->boost_cut_db / 20.0);
    beta = 4.0 / (1.0 + mu);
    delta = beta * tan(theta_c * 0.5);
    gamma = (1 - delta) / (1 + delta);

    coeffs[BIQUAD_a0] = 0.5 * (1.0 - gamma);
    coeffs[BIQUAD_a1] = 0.5 * (1.0 - gamma);
    coeffs[BIQUAD_a2] = 0.0;
    coeffs[BIQUAD_b1] = -gamma;
    coeffs[BIQUAD_b2] = 0.0;

    coeffs[BIQUAD_c0] = mu - 1.0;
    coeffs[BIQUAD_d0] = 1.0;

    break;
  case AF_HiShelf: // pg 278
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    mu = pow(10.0, params->boost_cut_db / 20.0);
    beta = 0.25 * (1.0 + mu);
    delta = beta * tan(theta_c * 0.5);
    gamma = (1 - delta) / (1 + delta);

    coeffs[BIQUAD_a0] = 0.5 * (1.0 + gamma);
    coeffs[BIQUAD_a1] = -0.5 * (1.0 + gamma);
    coeffs[BIQUAD_a2] = 0.0;
    coeffs[BIQUAD_b1] = -gamma;
    coeffs[BIQUAD_b2] = 0.0;

    coeffs[BIQUAD_c0] = mu - 1.0;
    coeffs[BIQUAD_d0] = 1.0;

    break;
  case AF_NCQParaEQ: // pg 279
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    mu = pow(10, params->boost_cut_db / 20.0);
    sigma = 4.0 / (1.0 + mu);
    beta = sigma * tan(theta_c / (2.0 * params->q));
    beta = 0.5 * (1.0 - beta) / (1.0 + beta);
    gamma = (0.5 + beta) * cos(theta_c);

    coeffs[BIQUAD_a0] = 0.5 - beta;
    coeffs[BIQUAD_a1] = 0.0;
    coeffs[BIQUAD_a2] = -(0.5 - beta);
    coeffs[BIQUAD_b1] = -2.0 * gamma;
    coeffs[BIQUAD_b2] = 2.0 * beta;

    coeffs[BIQUAD_c0] = mu - 1.0;
    coeffs[BIQUAD_d0] = 1.0;

    break;
  case AF_CQParaEQBoost: // pg 280
    kappa = tan(M_PI * params->fc * params->sample_rate_i);
    v_0 = pow(10.0, params->boost_cut_db / 20.0);
    d_0 = 1.0 + kappa / params->q + kappa * kappa;
    alpha = 1.0 + kappa * v_0 / params->q + kappa * kappa;
    beta = 2.0 * (kappa * kappa - 1);
    gamma = 1.0 - kappa * v_0 / params->q + kappa * kappa;
    delta = 1.0 - kappa / params->q + kappa * kappa;

    coeffs[BIQUAD_a0] = alpha / d_0;
    coeffs[BIQUAD_a1] = beta / d_0;
    coeffs[BIQUAD_a2] = gamma / d_0;
    coeffs[BIQUAD_b1] = beta / d_0;
    coeffs[BIQUAD_b2] = delta / d_0;

    break;
  case AF_CQParaEQCut: // pg 280
    kappa = tan(M_PI * params->fc * params->sample_rate_i);
    v_0 = pow(10.0, params->boost_cut_db / 20.0);
    d_0 = 1.0 + kappa / params->q + kappa * kappa;
    e_0 = 1.0 + kappa / (params->q * v_0) + kappa * kappa;
    alpha = 1.0 + kappa * v_0 / params->q + kappa * kappa;
    beta = 2.0 * (kappa * kappa - 1);
    gamma = 1.0 - kappa * v_0 / params->q + kappa * kappa;
    delta = 1.0 - kappa / params->q + kappa * kappa;
    eta = 1.0 - kappa / (params->q * v_0) + kappa * kappa;

    coeffs[BIQUAD_a0] = d_0 / e_0;
    coeffs[BIQUAD_a1] = beta / e_0;
    coeffs[BIQUAD_a2] = delta / e_0;
    coeffs[BIQUAD_b1] = beta / e_0;
    coeffs[BIQUAD_b2] = eta / e_0;

    break;
  case AF_LWRLPF2: // pg 275
    theta_c = M_PI * params->fc * params->sample_rate_i;
    omega_c = M_PI * params->fc;
    kappa = omega_c / tan(theta_c);
    delta = kappa * kappa + omega_c * omega_c + 2.0 * kappa * omega_c;

    coeffs[BIQUAD_a0] = omega_c * omega_c / delta;
    coeffs[BIQUAD_a1] = 2.0 * omega_c * omega_c / delta;
    coeffs[BIQUAD_a2] = omega_c * omega_c / delta;
    coeffs[BIQUAD_b1] = (-2.0 * kappa * kappa + 2 * omega_c * omega_c) / delta;
    coeffs[BIQUAD_b2] = (-2.0 * kappa * omega_c + kappa * kappa + omega_c * omega_c) / delta;

    break;
  case AF_LWRHPF2: // pg 275
    theta_c = M_PI * params->fc * params->sample_rate_i;
    omega_c = M_PI * params->fc;
    kappa = omega_c / tan(theta_c);
    delta = kappa * kappa + omega_c * omega_c + 2.0 * kappa * omega_c;

    coeffs[BIQUAD_a0] = kappa * kappa / delta;
    coeffs[BIQUAD_a1] = -2.0 * kappa * kappa / delta;
    coeffs[BIQUAD_a2] = kappa * kappa / delta;
    coeffs[BIQUAD_b1] = (-2.0 * kappa * kappa + 2 * omega_c * omega_c) / delta;
    coeffs[BIQUAD_b2] = (-2.0 * kappa * omega_c + kappa * kappa + omega_c * omega_c) / delta;

    break;
  case AF_APF1: // pg 277
    alpha = tan(M_PI * params->fc * params->sample_rate_i);
    alpha = (alpha - 1.0) / (alpha + 1.0);
    
    coeffs[BIQUAD_a0] = alpha;
    coeffs[BIQUAD_a1] = 1.0;
    coeffs[BIQUAD_a2] = 0.0;
    coeffs[BIQUAD_b1] = alpha;
    coeffs[BIQUAD_b2] = 0.0;

    break;
  case AF_APF2: // pg 277
    beta = M_PI * params->fc * params->sample_rate_i / params->q;
    alpha = tan(beta);
    alpha = (alpha - 1.0) / (alpha + 1.0);
    beta = -cos(2.0 * params->q * beta);

    coeffs[BIQUAD_a0] = -alpha;
    coeffs[BIQUAD_a1] = beta * (1.0 - alpha);
    coeffs[BIQUAD_a2] = 1.0;
    coeffs[BIQUAD_b1] = beta * (1.0 - alpha);
    coeffs[BIQUAD_b2] = -alpha;

    break;
  case AF_ResonA: // pg 260 smith-angell reso
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    
    coeffs[BIQUAD_b2] = exp(-2.0 * M_PI * params->fc * params->sample_rate_i / params->q);
    coeffs[BIQUAD_b1] = -4.0 * coeffs[BIQUAD_b2] * cos(theta_c) / (1.0 + coeffs[BIQUAD_b2]);
    coeffs[BIQUAD_a0] = 1.0 - sqrt(coeffs[BIQUAD_b2]);
    //coeffs[BIQUAD_a0] = (1.0 - coeffs[BIQUAD_b2]) * sqrt(1.0 - 0.25 * coeffs[BIQUAD_b1] * coeffs[BIQUAD_b1] / coeffs[BIQUAD_b2]);
    coeffs[BIQUAD_a1] = 0.0;
    coeffs[BIQUAD_a2] = -coeffs[BIQUAD_a0];

    break;
  case AF_MatchLP2A: // pg 286 tight fit
    omega_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    q = 0.5 / params->q;
    phi_1 = sin(0.5 * omega_c) * sin(0.5 * omega_c);
    phi_0 = 1.0 - phi_1;
    phi_2 = 4.0 * phi_0 * phi_1;

    coeffs[BIQUAD_b2] = exp(-2.0 * q * omega_c);
    coeffs[BIQUAD_b1] = q <= 1
                      ? -2.0 * exp(-q * omega_c) * cos(sqrt(1.0 - q * q) * omega_c)
                      : -2.0 * exp(-q * omega_c) * cosh(sqrt(q * q - 1.0) * omega_c);

    B_0 = (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]);
    B_1 = (1.0 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * (1.0 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]);
    B_2 = -4.0 * coeffs[BIQUAD_b2];

    A_0 = B_0;
    r_1 = (B_0 * phi_0 + B_1 * phi_1 + B_2 * phi_2) * params->q * params->q;
    A_1 = (r_1 - A_0 * phi_0) / phi_1;

    coeffs[BIQUAD_a0] = 0.5 * (sqrt(A_0) + sqrt(A_1));
    coeffs[BIQUAD_a1] = sqrt(A_0) - coeffs[BIQUAD_a0];
    coeffs[BIQUAD_a2] = 0.0;

    break;
  case AF_MatchLP2B: // pg 286 loose fit
    omega_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    q = 0.5 / params->q;
    phi_1 = sin(0.5 * omega_c) * sin(0.5 * omega_c);
    phi_0 = 1.0 - phi_1;
    phi_2 = 4.0 * phi_0 * phi_1;

    coeffs[BIQUAD_b2] = exp(-2.0 * q * omega_c);
    coeffs[BIQUAD_b1] = q <= 1
                      ? -2.0 * exp(-q * omega_c) * cos(sqrt(1.0 - q * q) * omega_c)
                      : -2.0 * exp(-q * omega_c) * cosh(sqrt(q * q - 1.0) * omega_c);

    B_0 = (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]);
    B_1 = (1.0 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * (1.0 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]);
    B_2 = -4.0 * coeffs[BIQUAD_b2];

    f_0 = omega_c / M_PI;
    r_1 = (1 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * f_0 * f_0 /
          sqrt((1.0 - f_0 * f_0) * (1.0 - f_0 * f_0) + f_0 * f_0 / (params->q * params->q));
    r_0 = 1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2];

    coeffs[BIQUAD_a0] = 0.5 * (r_0 + r_1);
    coeffs[BIQUAD_a1] = r_0 - coeffs[BIQUAD_a0];
    coeffs[BIQUAD_a2] = 0.0;

    break;
  case AF_MatchBP2A: // pg 287 tight fit
    omega_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    q = 0.5 / params->q;
    phi_1 = sin(0.5 * omega_c) * sin(0.5 * omega_c);
    phi_0 = 1.0 - phi_1;
    phi_2 = 4.0 * phi_0 * phi_1;

    coeffs[BIQUAD_b2] = exp(-2.0 * q * omega_c);
    coeffs[BIQUAD_b1] = q <= 1
                      ? -2.0 * exp(-q * omega_c) * cos(sqrt(1.0 - q * q) * omega_c)
                      : -2.0 * exp(-q * omega_c) * cosh(sqrt(q * q - 1.0) * omega_c);

    B_0 = (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]);
    B_1 = (1.0 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * (1.0 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]);
    B_2 = -4.0 * coeffs[BIQUAD_b2];

    r_1 = B_0 * phi_0 + B_1 * phi_1 + B_2 * phi_2;
    r_2 = -B_0 + B_1 + 4.0 * (phi_0 - phi_1) * B_2;
    A_2 = 0.25 * (r_1 - r_2 * phi_1) / (phi_1 * phi_1);
    A_1 = r_2 + 4.0 * (phi_1 - phi_0) * A_2;

    coeffs[BIQUAD_a1] = -0.5 * sqrt(A_1);
    coeffs[BIQUAD_a0] = 0.5 * (sqrt(A_2 + coeffs[BIQUAD_a1] * coeffs[BIQUAD_a1]) - coeffs[BIQUAD_a1]);
    coeffs[BIQUAD_a2] = -coeffs[BIQUAD_a0] - coeffs[BIQUAD_a1];

    break;
  case AF_MatchBP2B: // pg 287 loose fit
    omega_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    q = 0.5 / params->q;
    phi_1 = sin(0.5 * omega_c) * sin(0.5 * omega_c);
    phi_0 = 1.0 - phi_1;
    phi_2 = 4.0 * phi_0 * phi_1;

    coeffs[BIQUAD_b2] = exp(-2.0 * q * omega_c);
    coeffs[BIQUAD_b1] = q <= 1
                      ? -2.0 * exp(-q * omega_c) * cos(sqrt(1.0 - q * q) * omega_c)
                      : -2.0 * exp(-q * omega_c) * cosh(sqrt(q * q - 1.0) * omega_c);

    B_0 = (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]);
    B_1 = (1.0 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * (1.0 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]);
    B_2 = -4.0 * coeffs[BIQUAD_b2];

    f_0 = omega_c / M_PI;
    r_0 = (1.0 + coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) / (M_PI * f_0 * params->q);
    r_1 = (1.0 - coeffs[BIQUAD_b1] + coeffs[BIQUAD_b2]) * f_0 /
          (params->q * sqrt((1.0 - f_0 * f_0) * (1.0 - f_0 * f_0) + f_0 * f_0 / (params->q * params->q)));
    coeffs[BIQUAD_a1] = -0.5 * r_1;
    coeffs[BIQUAD_a0] = 0.5 * (r_1 - coeffs[BIQUAD_a1]);
    coeffs[BIQUAD_a2] = -coeffs[BIQUAD_a0] - coeffs[BIQUAD_a1];

    break;
  case AF_ImpInvLPF1: // pg 290
    t = 1.0 * params->sample_rate_i;
    omega_c = 2.0 * M_PI * params->fc;

    coeffs[BIQUAD_a0] = 1 - exp(-omega_c * t);
    coeffs[BIQUAD_b1] = -exp(-omega_c * t);

    break;
  case AF_ImpInvLPF2: // pg 291
    omega_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    p_real = -0.5 * omega_c / params->q;
    p_im = omega_c * sqrt(1.0 - (0.5 / params->q) * (0.5 / params->q));
    c_im = 0.5 * omega_c / sqrt(1.0 - (0.5 / params->q) * (0.5 / params->q));

    coeffs[BIQUAD_a0] = 0.0;
    coeffs[BIQUAD_a1] = -2.0 * c_im * sin(p_im) * exp(p_real);
    coeffs[BIQUAD_a2] = 0.0;
    coeffs[BIQUAD_b1] = -2.0 * cos(p_im) * exp(p_real);
    coeffs[BIQUAD_b2] = exp(p_real) * exp(p_real);

    break;
  default:
    // fall through
  case AF_LPF1: // pg 271 first order low pass
    theta_c = 2.0 * M_PI * params->fc * params->sample_rate_i;
    gamma = cos(theta_c) / (1.0 + sin(theta_c));

    coeffs[BIQUAD_a0] = (1.0 - gamma) * 0.5;
    coeffs[BIQUAD_a1] = (1.0 - gamma) * 0.5;
    coeffs[BIQUAD_a2] = 0.0;
    coeffs[BIQUAD_b1] = -gamma;
    coeffs[BIQUAD_b2] = 0.0;
  
    break;
  }
}
#undef coeffs

#define coeffs (state->audio_filter.biquad.coeffs)
FTYPE
mono_filter(FTYPE *L, dsp_state *state, FTYPE control)
{
  *L = coeffs[BIQUAD_d0] * *L +
       coeffs[BIQUAD_c0] * biquad_process_sample(&state->audio_filter.biquad, *L);
  return control;
}
#undef coeffs

void
dsp_audio_filter_set_params(
    dsp_state *state,
    audio_filter_params params)
{
  state->audio_filter.sample_rate =
      params.sample_rate < DEFAULT_SAMPLE_RATE
      ? DEFAULT_SAMPLE_RATE
      : params.sample_rate;
  state->audio_filter.sample_rate_i = 1.0 / state->audio_filter.sample_rate,
  state->audio_filter.alg = params.alg;
  state->audio_filter.fc = params.fc;
  state->audio_filter.boost_cut_db = params.boost_cut_db;
  state->audio_filter.q =
      params.q <= 0
      ? 0.707
      : params.q;

  calculate_filter_coefficients(&state->audio_filter);
}

DSP_callback
dsp_init_audio_filter(audio_filter_params params)
{
  DSP_callback cb = dsp_init();
  dsp_audio_filter_set_params(&cb->state, params);
  dsp_set_mono_left(cb, mono_filter);

  return cb;
}

DSP_callback
dsp_init_audio_filter_default()
{
  audio_filter_params params = {
    .sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE,
    .fc = 100.0,
    .q = 0.707,
    .boost_cut_db = 0,
    .alg = AF_LPF1
  };
  DSP_callback cb = dsp_init_audio_filter(params);

  return cb;
}
