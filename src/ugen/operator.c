#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "operator.h"
#include "ugen.h"
#include "../env/envelope.h"

Operator
operator_alloc()
{
  return calloc(1, sizeof(struct operator_t));
}

void
operator_free(Operator op)
{
  free(op);
}

void
operator_cleanup(Operator op)
{
  ugen_cleanup(op->ugen);
  switch (op->e_type) {
  case OPERATOR_ENV:
    env_cleanup(op->env_u.env);
    break;
  case OPERATOR_UGEN:
    ugen_cleanup(op->env_u.lfo);
    break;
  // no default
  }
  operator_free(op);
}

Operator
operator_init(ugen_type_e u_type, operator_env_e e_type, FTYPE gain)
{
  Operator op = operator_alloc();
  op->e_type = e_type;
  op->gain_c = gain;
  op->fc = 0.0;
  op->fm = 0.0;
  op->mult = 1.0;

  switch (u_type) {
  case UGEN_OSC_IMP:
    op->ugen = ugen_init_imp(0.0, 0.5);
    break;
  case UGEN_OSC_SAW:
    op->ugen = ugen_init_saw(0.0);
    break;
  case UGEN_OSC_TRI:
    op->ugen = ugen_init_tri(0.0);
    break;
  default:
  // fall through
  case UGEN_OSC_SIN:
    op->ugen = ugen_init_sin(0.0);
    break;
  }

  // TODO support configuration of EG at init time
  switch (e_type) {
  case OPERATOR_UGEN:
    op->env_u.lfo = ugen_init_sin(1.0);
    ugen_set_scale(op->env_u.lfo, 0.0, 1.0);
    break;
  default:
  // fall through
  case OPERATOR_ENV:
    op->env_u.env = env_init_default();
    break;
  }

  return op;
}

Operator
operator_init_default()
{
  return operator_init(UGEN_OSC_SIN, OPERATOR_ENV, 1.0);
}

void
operator_reset(Operator op)
{
  ugen_reset_phase(op->ugen);
  op->fm = 0.0; // ?

  switch (op->e_type) {
  case OPERATOR_UGEN:
    ugen_reset_phase(op->env_u.lfo);
    break;
  case OPERATOR_ENV:
    env_reset(op->env_u.env);
    break;
  // no default
  }
}

/*
cent = 1200 * log₂(f₂ / f₁)
cent = 1200 * (log2(f2) - log2(f1))
cent / 1200 = log2(f2) - log2(f1)
log2(f2) = cent / 1200 + log2(f1)
f2 = pow(2.0, cent/1200 + log2(f1))
 */
void
operator_set_fc(Operator op, FTYPE fc, FTYPE detune_in_cents)
{
  op->fc = pow(2.0, detune_in_cents / 1200.0 + log2(fc));
}

void
operator_set_fm(Operator op, FTYPE fm)
{
  op->fm = fm;
}

void
operator_set_gain(Operator op, FTYPE gain)
{
  op->gain_c = gain;
}

void
operator_set_mult(Operator op, FTYPE mult)
{
  op->mult = mult;
}

FTYPE
operator_sample(Operator op, bool sustain)
{
  FTYPE s1, s2;

  ugen_set_freq(op->ugen, op->fc * op->mult + op->fm);
  s1 = ugen_sample(op->ugen);
  switch (op->e_type) {
  case OPERATOR_UGEN:
    s2 = ugen_sample(op->env_u.lfo);
    break;
  case OPERATOR_ENV:
    s2 = env_sample(op->env_u.env, sustain);
    break;
  default:
    s2 = 1.0;
    break;
  }

  return op->gain_c * s1 * s2;
}