#ifndef UGEN_H
#define UGEN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../lib/macros.h"

#include "../cli/cli.h"
#include "../tunable/tunable.h"

#define ugen_reset_phase(u) ((u)->p_ind = 0)
#define ugen_set_cr(u) ((u)->conv.cr = true)
#define ugen_unset_cr(u) ((u)->conv.cr = false)

/*
 * Can be
 * audio rate oscillators,
 * control rate oscillators
 * amplitude envelopes
 * filters ?
 * anything else?
 */
typedef enum {
  UGEN_CONSTANT, // ctrl
  UGEN_OSC_IMP, // audio
  UGEN_OSC_SAW, // audio
  UGEN_OSC_SIN, // audio
  UGEN_OSC_TRI, // audio
  UGEN_EASE_IN_CIRCLE, // ctrl
  UGEN_EASE_IN_CIRCLE_INV, // ctrl
  UGEN_EASE_OUT_CIRCLE, // ctrl
  UGEN_EASE_OUT_CIRCLE_INV, //ctrl
  UGEN_RAMP_LINEAR_DOWN, // ctrl
  UGEN_RAMP_LINEAR_UP // ctrl
} ugen_type_e;

typedef enum {
  UGEN_PHASE_NORM,     // in phase
  UGEN_PHASE_QUAD,     // +90
  UGEN_PHASE_QUAD_NEG, // -90 (or inverted quad)
  UGEN_PHASE_INV,      // inverted in phase
  UGEN_PHASE_NUM
} ugen_phase_e;

typedef FTYPE triphase[UGEN_PHASE_NUM];

typedef struct ar_cr_t {
  FTYPE bias;
  FTYPE scale;
  //true if caller wants [0,1], false if [-1,1]
  bool cr;
} rate_converter;

typedef struct ugen_params_t {
  FTYPE sample_rate;
  FTYPE freq;
  FTYPE scale_low;
  FTYPE scale_high;
  FTYPE duty_cycle;
  ugen_type_e shape;
} ugen_params;

typedef struct ugen_t {
  FTYPE sample_rate;

  // ugen to be used as a gain/velocity modulator
  struct ugen_t *gain;

  // struct for ugen-specific vars
  union {
    struct {
      FTYPE dc;
    } impulse;
  } u;

  // sample fn determines the wave table to use
  FTYPE (*sample)(struct ugen_t *, FTYPE);
  rate_converter conv;

  // phase management for table indexing
  int32_t p_inc_whole;
  FTYPE p_inc_frac;
  int32_t p_ind;

  // phase management for computation
  FTYPE p_inc;
  FTYPE p;

  Cli_menu menu;
  struct {
    ugen_params p;
    Tunable *ts;
  } tunables;
} *Ugen;

// sample fn determines the wave table to use
typedef FTYPE (*sample_fn)(Ugen, FTYPE);

void ugen_generate_tables();

Ugen ugen_init_imp(FTYPE freq, FTYPE duty_cycle, FTYPE sample_rate);
Ugen ugen_init_saw(FTYPE freq, FTYPE sample_rate);
Ugen ugen_init_sin(FTYPE freq, FTYPE sample_rate);
Ugen ugen_init_tri(FTYPE freq, FTYPE sample_rate);

Ugen ugen_init_ease_in_circle(FTYPE freq, FTYPE sample_rate);
Ugen ugen_init_ease_out_circle(FTYPE freq, FTYPE sample_rate);
Ugen ugen_init_ramp_linear(FTYPE freq, FTYPE sample_rate);

Ugen ugen_init_with_params(ugen_params *p);
void ugen_set_params(Ugen ugen, ugen_params *p);

void ugen_cleanup(Ugen ugen);

void ugen_set_sample_rate(Ugen ugen, FTYPE sample_rate);
void ugen_set_duty_cycle(Ugen ugen, FTYPE duty_cycle);
void ugen_set_freq(Ugen ugen, FTYPE freq);
void ugen_set_scale(Ugen ugen, FTYPE low, FTYPE high);

FTYPE ugen_sample_mod(Ugen ugen, FTYPE phase_mod);
void ugen_sample_mod_triphase(Ugen ugen, FTYPE phase_mod, triphase rv);

FTYPE ugen_sample_fast(Ugen ugen, FTYPE phase_mod);
void ugen_sample_fast_triphase(Ugen ugen, FTYPE phase_mod, triphase rv);

#endif
