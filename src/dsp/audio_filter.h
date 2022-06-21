#ifndef AUDIO_FILTER_H
#define AUDIO_FILTER_H

#include "../lib/macros.h"

#include "biquad.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Prikle, chapter 11
 */

typedef enum {
  AF_LPF1P,
  AF_LPF1,
  AF_HPF1,
  AF_LPF2,
  AF_HPF2,
  AF_BPF2,
  AF_BSF2,
  AF_ButterLPF2,
  AF_ButterHPF2,
  AF_ButterBPF2,
  AF_ButterBSF2,
  AF_MMALPF2,
  AF_MMALPF2B,
  AF_LowShelf,
  AF_HiShelf,
  AF_NCQParaEQ,
  AF_CQParaEQBoost,
  AF_CQParaEQCut,
  AF_LWRLPF2,
  AF_LWRHPF2,
  AF_APF1,
  AF_APF2,
  AF_ResonA,
  AF_MatchLP2A,
  AF_MatchLP2B,
  AF_MatchBP2A,
  AF_MatchBP2B,
  AF_ImpInvLPF1,
  AF_ImpInvLPF2
} audio_filter_algorithm;

typedef struct audio_filter_params_t {
  double fc;
  double q;
  double boost_cut_db;
  audio_filter_algorithm alg;
  biquad_params biquad;
} audio_filter_params;

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;

void dsp_audio_filter_set_params(dsp_state *state, audio_filter_algorithm alg, double fc, double q, double boost_cut_db);

#endif
