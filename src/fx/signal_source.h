#ifndef FX_SIGNAL_SOURCE_H
#define FX_SIGNAL_SOURCE_H

#include <stdlib.h>

#include "../lib/macros.h"

#include "../ugen/dfo.h"
#include "../ugen/ugen.h"

typedef enum fx_unit_signal_source_type_t {
  FX_SIGNAL_CONSTANT,
  FX_SIGNAL_DFO,
  FX_SIGNAL_UGEN
} fx_unit_signal_source_type;

typedef enum fx_unit_signal_source_dst_fields_e {
  FX_SIGNAL_L = 1,
  FX_SIGNAL_R = 2,
  FX_SIGNAL_C = 4,
} fx_unit_signal_source_dst_fields;

typedef struct fx_unit_signal_source_state_t {
  fx_unit_signal_source_type t;
  fx_unit_signal_source_dst_fields d;
  union {
    FTYPE constant;
    Dfo dfo;
    Ugen ugen;
  } u;
} fx_unit_signal_source_state;

typedef fx_unit_signal_source_state *FX_unit_signal_source_state;

typedef struct fx_unit_signal_source_params_t {
  fx_unit_signal_source_type t;
  fx_unit_signal_source_dst_fields d;
  union {
    FTYPE constant;
    Dfo dfo;
    Ugen ugen;
  } u;
} fx_unit_signal_source_params;

typedef struct fx_unit_signal_source_params_t *FX_unit_signal_source_params;

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_signal_source_init(FX_unit_params params);
fx_unit_params fx_unit_signal_source_constant_default();
fx_unit_params fx_unit_signal_source_dfo_default();
fx_unit_params fx_unit_signal_source_ugen_default();

#endif
