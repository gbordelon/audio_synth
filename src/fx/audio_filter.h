#ifndef FX_AUDIO_FILTER_H
#define FX_AUDIO_FILTER_H

#include <stdlib.h>

#include "../lib/macros.h"

#include "biquad.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 11
 */

typedef enum fx_unit_audio_filter_algorithm_e {
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
} fx_unit_audio_filter_algorithm;

typedef struct fx_unit_audio_filter_state_t {
  FTYPE sample_rate_i; // multiplicative inverse
  FTYPE fc;
  FTYPE q;
  FTYPE boost_cut_db;
  fx_unit_audio_filter_algorithm alg;
  biquad_params biquad;
} fx_unit_audio_filter_state;

typedef fx_unit_audio_filter_state *FX_unit_audio_filter_state;

typedef struct fx_unit_audio_filter_params_t {
  FTYPE fc;
  FTYPE q;
  FTYPE boost_cut_db;
  fx_unit_audio_filter_algorithm alg;
} fx_unit_audio_filter_params;

typedef struct fx_unit_audio_filter_params_t *FX_unit_audio_filter_params;

// forward decl
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_audio_filter_init(FX_unit_params params);

#endif
