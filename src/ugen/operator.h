#ifndef OPERATOR_H
#define OPERATOR_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "../env/envelope.h"

#include "dfo.h"
#include "ugen.h"

#include "../cli/cli.h"
#include "../tunable/tunable.h"

/*
 * The envelope generator is an LFO or an Envelope
 */
typedef enum {
  OPERATOR_ENV,
  OPERATOR_UGEN,
  OPERATOR_NONE
} operator_env_e;

typedef struct operator_params_t {
  FTYPE sample_rate;
  ugen_type_e u_type;
  operator_env_e e_type;
  FTYPE mult;
  FTYPE detune;
  FTYPE vel_s;
  FTYPE gain_c;
  FTYPE pan;
} operator_params;

typedef struct operator_t {
  FTYPE sample_rate;

  ugen_type_e u_type;
  Ugen ugen;
  Dfo dfo;

  operator_env_e e_type;
  union {
    Envelope env;
    Ugen lfo;
  } env_u;

  // sum these two to set the ugen frequency before sampling.
  FTYPE fc; // stored carrier freq
  FTYPE velocity; // stored note velocity scaled to [0,1]

  FTYPE mod; // mod value for next ugen sample

  FTYPE mult; // carrier frequency multiplier
  FTYPE detune; // carrier frequency detune TODO what should the domain be? 1200 cents per octave...
  FTYPE vel_s; // velocity sensitivity. Lower means gain_c is heavier, higher means velocity is heavier
  FTYPE gain_c; // overall post-envelope gain
  FTYPE pan; // 0 pure left, 1 pure right

  Cli_menu menu;
  struct {
    operator_params p;
    Tunable *ts;
  } tunables;
} *Operator;

Operator operator_init(operator_params *p);
Operator operator_init_default();

void operator_cleanup(Operator op);

void operator_reset(Operator op);

void operator_set_detune(Operator op, FTYPE detune);
void operator_set_fc(Operator op, FTYPE fc);
void operator_set_gain(Operator op, FTYPE gain);
void operator_set_mod(Operator op, FTYPE mod); // in radians
void operator_set_mult(Operator op, FTYPE mult);
void operator_set_pan(Operator op, FTYPE pan);
void operator_set_velocity(Operator op, FTYPE vel);
void operator_set_vel_s(Operator op, FTYPE vel_s);

void operator_set_params(Operator op, operator_params *p);

void operator_release(Operator op);

FTYPE operator_sample(Operator op, bool sustain);

#endif
