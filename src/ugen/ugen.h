#ifndef UGEN_H
#define UGEN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../lib/macros.h"

#define ugen_reset_phase(u) ((u)->p_ind = 0)
#define ugen_set_cr(u) ((u)->cr = true)
#define ugen_unset_cr(u) ((u)->cr = false)

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

typedef struct ar_cr_t {
  FTYPE bias;
  FTYPE scale;
} rate_converter;

typedef struct ugen_t {
  // ugen to be used as a phase modulator
  struct ugen_t *mod;
  // ugen to be used as a gain/velocity modulator
  struct ugen_t *gain;
  // constant gain to be used when no gain modulator is assoc'd
  FTYPE gain_c;

  // struct for ugen-specific vars
  union {
    struct {
    // use constant if ugen is null
      FTYPE duty_cycle_c;
      struct ugen_t *duty_cycle; // imagine LFO duty cycle shifting :)
    } impulse;
  } u;

  // sample fn determines the wave table to use
  FTYPE (*sample)(struct ugen_t *, size_t);
  //true if caller wants [0,1], false if [-1,1]
  rate_converter conv;
  bool cr;

  // phase management for table indexing
  int32_t p_inc_whole;
  FTYPE p_inc_frac;
  uint32_t p_ind;
} *Ugen;

// sample fn determines the wave table to use
typedef FTYPE (*sample_fn)(Ugen, size_t);

void ugen_generate_tables();

Ugen ugen_init_constant();
Ugen ugen_init_imp(FTYPE freq, FTYPE duty_cycle);
Ugen ugen_init_saw(FTYPE freq);
Ugen ugen_init_sin(FTYPE freq);
Ugen ugen_init_tri(FTYPE freq);

Ugen ugen_init_ease_in_circle(FTYPE freq);
Ugen ugen_init_ease_out_circle(FTYPE freq);
Ugen ugen_init_ramp_linear(FTYPE freq);

void ugen_cleanup(Ugen ugen);

void ugen_set_mod(Ugen car, Ugen mod);
void ugen_set_gain(Ugen car, Ugen gain);
void ugen_set_duty_cycle(Ugen ugen, Ugen duty_cycle);
void ugen_set_gain_c(Ugen ugen, FTYPE gain);
void ugen_set_duty_cycle_c(Ugen ugen, FTYPE duty_cycle);
void ugen_set_freq(Ugen ugen, FTYPE freq);
void ugen_set_scale(Ugen ugen, FTYPE low, FTYPE high);

void ugen_chunk_sample(Ugen ugen, FTYPE buf[CHUNK_SIZE]);
FTYPE ugen_sample(Ugen ugen);
FTYPE ugen_sample_mod(Ugen ugen, size_t phase_mod);

#endif
