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
  FTYPE fc;
  FTYPE fm;
  FTYPE mult;

  // scale the output to be a frequency for another operator [-24khz, 24khz], or an audio sample [-1,1]
  FTYPE gain_c;
} *Operator;

Operator operator_init(ugen_type_e u_type, operator_env_e e_type, FTYPE gain);
Operator operator_init_default();

void operator_cleanup(Operator op);

void operator_reset(Operator op);

void operator_set_fc(Operator op, FTYPE fc, FTYPE detune_in_cents);
void operator_set_fm(Operator op, FTYPE fm);
void operator_set_gain(Operator op, FTYPE gain);

FTYPE operator_sample(Operator op, bool sustain);

#endif
