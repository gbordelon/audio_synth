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

  // struct for ugen-specific vars
  union {
    // use constant if ugen is null
    struct {
      FTYPE duty_cycle_c;
      struct ugen_t *duty_cycle; // imagine LFO duty cycle shifting :)
    } impulse;
  } u;

  // sample fn determines the wave table
  FTYPE (*sample)(struct ugen_t *, size_t);

  // phase management for table indexing
  uint32_t p_inc_whole;
  FTYPE p_inc_frac;
  uint32_t p_ind;
} *Ugen;

FTYPE ugen_sample_constant(Ugen ugen, size_t phase_mod);

// called by other ugen inits
Ugen ugen_init();
void ugen_cleanup(Ugen ugen);

void ugen_set_mod(Ugen car, Ugen mod);
void ugen_set_duty_cycle(Ugen ugen, FTYPE duty_cycle);
void ugen_set_duty_cycle_ugen(Ugen ugen, Ugen duty_cycle);
void ugen_set_freq(Ugen ugen, FTYPE freq);
void ugen_chunk_sample(Ugen ugen, FTYPE buf[CHUNK_SIZE]);
FTYPE ugen_sample(Ugen ugen);

#endif
