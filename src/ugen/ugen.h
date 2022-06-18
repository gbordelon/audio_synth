#ifndef UGEN_H
#define UGEN_H

#include <stddef.h>
#include <stdint.h>

#include "../lib/macros.h"

#define ugen_reset_phase(u) ((u)->p_ind = 0)

/*
 * Can be
 * audio rate oscillators,
 * control rate oscillators
 * amplitude envelopes
 * filters
 * anything else?
 */
enum ugen_type {
  UGEN_CONSTANT,
  UGEN_OSC_IMP,
  UGEN_OSC_SAW,
  UGEN_OSC_SIN,
  UGEN_OSC_TRI,
  UGEN_RAMP_CIRCLE_DOWN,
  UGEN_RAMP_CIRCLE_UP,
  UGEN_RAMP_LINEAR
};

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

  // phase management for table indexing
  uint32_t p_inc_whole;
  FTYPE p_inc_frac;
  uint32_t p_ind;
} *Ugen;

void ugen_generate_tables();

Ugen ugen_init_constant();
Ugen ugen_init_imp(FTYPE freq, FTYPE duty_cycle);
Ugen ugen_init_saw(FTYPE freq);
Ugen ugen_init_sin(FTYPE freq);
Ugen ugen_init_tri(FTYPE freq);

Ugen ugen_init_ramp_circle_down(FTYPE freq);
Ugen ugen_init_ramp_circle_up(FTYPE freq);
Ugen ugen_init_ramp_linear(FTYPE freq);

void ugen_cleanup(Ugen ugen);

void ugen_set_mod(Ugen car, Ugen mod);
void ugen_set_gain(Ugen car, Ugen gain);
void ugen_set_duty_cycle(Ugen ugen, Ugen duty_cycle);
void ugen_set_gain_c(Ugen ugen, FTYPE gain);
void ugen_set_duty_cycle_c(Ugen ugen, FTYPE duty_cycle);
void ugen_set_freq(Ugen ugen, FTYPE freq);
void ugen_chunk_sample(Ugen ugen, FTYPE buf[CHUNK_SIZE]);
FTYPE ugen_sample(Ugen ugen);

#endif
