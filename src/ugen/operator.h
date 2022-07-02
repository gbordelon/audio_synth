#ifndef OPERATOR_H
#define OPERATOR_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "operator.h"
#include "ugen.h"
#include "../env/envelope.h"

/*
 * The envelope generator is an LFO or an Envelope
 * */
typedef enum {
  OPERATOR_ENV,
  OPERATOR_UGEN
} operator_env_e;

typedef struct operator_t {
  Ugen ugen;

  operator_env_e e_type;
  union {
    Envelope env;
    Ugen lfo;
  } env_u;

  // sum these two to set the ugen frequency before sampling.
  FTYPE fc; // stored carrier freq
  FTYPE mod; // mod value for next ugen freq calc

  FTYPE mult; // carrier frequency multiplier
  FTYPE detune; // carrier frequency detune TODO what should the domain be? 1200 cents per octave...

  FTYPE velocity; // midi velocity scaled to [0,1]
  FTYPE vel_s; // scale the midi velocity

  // scale the output to be a frequency for another operator [-24khz, 24khz], or an audio sample [-1,1]
  FTYPE gain_c; // overall post-envelope gain
} *Operator;

Operator operator_init(ugen_type_e u_type, operator_env_e e_type, FTYPE gain);
Operator operator_init_default();

void operator_cleanup(Operator op);

void operator_reset(Operator op);

void operator_set_fc(Operator op, FTYPE fc);
void operator_set_mod(Operator op, FTYPE mod); // in radians
void operator_set_gain(Operator op, FTYPE gain);
void operator_set_velocity(Operator op, FTYPE vel);

FTYPE operator_sample(Operator op, bool sustain);

#endif
