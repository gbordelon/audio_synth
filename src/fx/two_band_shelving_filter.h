#ifndef FX_TWO_BAND_SHELVING_FILTER_H
#define FX_TWO_BAND_SHELVING_FILTER_H

#include <stdlib.h>

#include "../lib/macros.h"

/*
 * Based on code from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef struct fx_compound_unit_t *FX_compound_unit;
typedef int16_t fx_unit_idx;

typedef struct fx_unit_two_band_shelving_filter_state_t {
  fx_unit_idx low_apf;
  fx_unit_idx high_apf;
  FTYPE low_shelf_fc;
  FTYPE low_shelf_boost_cut_db;
  FTYPE high_shelf_fc;
  FTYPE high_shelf_boost_cut_db;
} fx_unit_two_band_shelving_filter_state;

typedef fx_unit_two_band_shelving_filter_state *FX_unit_two_band_shelving_filter_state;

typedef struct fx_unit_two_band_shelving_filter_params_t {
  FTYPE low_shelf_fc;
  FTYPE low_shelf_boost_cut_db;
  FTYPE high_shelf_fc;
  FTYPE high_shelf_boost_cut_db;
} fx_unit_two_band_shelving_filter_params;

typedef struct fx_unit_two_band_shelving_filter_params_t *FX_unit_two_band_shelving_filter_params;

fx_unit_idx fx_unit_two_band_shelving_filter_init(FX_unit_params params);

FX_compound_unit fx_compound_unit_two_band_shelving_filter_init(FX_unit_params two_band_shelving_filter_p);

fx_unit_params fx_unit_two_band_shelving_filter_default();

#endif
