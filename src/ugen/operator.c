#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "../env/envelope.h"

#include "dfo.h"
#include "operator.h"
#include "ugen.h"

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
  dfo_cleanup(op->dfo);
  switch (op->e_type) {
  case OPERATOR_ENV:
    env_cleanup(op->env_u.env);
    break;
  case OPERATOR_UGEN:
    ugen_cleanup(op->env_u.lfo);
    break;
  default:
    break;
  }
  operator_free(op);
}

Operator
operator_init(ugen_type_e u_type, operator_env_e e_type, FTYPE gain, FTYPE sample_rate)
{
  Operator op = operator_alloc();
  op->e_type = e_type;
  op->detune = 0.0;
  op->fc = 0.0;
  op->mod = 0.0;
  op->gain_c = gain;
  op->pan = 0.5;
  op->mult = 1.0;
  op->vel_s = 1.0;

  switch (u_type) {
  case UGEN_OSC_IMP:
    op->ugen = ugen_init_imp(0.0, 0.5, sample_rate);
    break;
  case UGEN_OSC_SAW:
    op->ugen = ugen_init_saw(0.0, sample_rate);
    break;
  case UGEN_OSC_TRI:
    op->ugen = ugen_init_tri(0.0, sample_rate);
    break;
  default:
  // fall through
  case UGEN_OSC_SIN:
    op->ugen = ugen_init_sin(0.0, sample_rate);
    op->dfo = dfo_init(sample_rate);
    break;
  }

  // TODO support configuration of EG at init time
  switch (e_type) {
  case OPERATOR_UGEN:
    op->env_u.lfo = ugen_init_sin(1.0, sample_rate);
    ugen_set_scale(op->env_u.lfo, 0.0, 1.0);
    break;
  case OPERATOR_ENV:
    op->env_u.env = env_init_default();
    env_set_sample_rate(op->env_u.env, sample_rate);
    break;
  default:
    // none
    break;
  }

  return op;
}

Operator
operator_init_default()
{
  return operator_init(UGEN_OSC_SIN, OPERATOR_ENV, 1.0, DEFAULT_SAMPLE_RATE);
}

void
operator_reset(Operator op)
{
  ugen_reset_phase(op->ugen);
  op->mod = 0.0;

  switch (op->e_type) {
  case OPERATOR_UGEN:
    ugen_reset_phase(op->env_u.lfo);
    break;
  case OPERATOR_ENV:
    env_reset(op->env_u.env);
    break;
  default:
    break;
  }
}

/*
cent = 1200 * log???(f??? / f???)
cent = 1200 * (log2(f2) - log2(f1))
cent / 1200 = log2(f2) - log2(f1)
log2(f2) = cent / 1200 + log2(f1)
f2 = pow(2.0, cent/1200 + log2(f1))
 */
void
operator_set_fc(Operator op, FTYPE fc)
{
  op->fc = pow(2.0, op->detune / 1200.0 + log2(fc));
  ugen_set_freq(op->ugen, op->mult * op->fc);
  if (op->dfo) {
    dfo_set_freq(op->dfo, op->mult * op->fc);
  }
}

// convert from radians to wave table phase units inside the ugen sample function
void
operator_set_mod(Operator op, FTYPE mod)
{
  op->mod = mod;
}

void
operator_set_gain(Operator op, FTYPE gain)
{
  op->gain_c = gain;
}

void
operator_set_pan(Operator op, FTYPE pan)
{
  op->pan = pan;
}

void
operator_set_velocity(Operator op, FTYPE vel)
{
  op->velocity = vel;
}

void
operator_set_vel_s(Operator op, FTYPE vel_s)
{
  op->vel_s = vel_s;
}

void
operator_set_mult(Operator op, FTYPE mult)
{
  op->mult = mult;
}

void
operator_release(Operator op)
{
  switch (op->e_type) {
  case OPERATOR_UGEN:
    break;
  case OPERATOR_ENV:
    env_set_release(op->env_u.env);
    break;
  default:
    break;
  }
}

FTYPE
operator_sample_apply_gain(Operator op, FTYPE sample)
{
  // as vel_s -> 0, more gain
  // as vel_s -> 1, more scaling by velocity
  //
  // vel_s = 0 and vel = 0 => gain
  // vel_s = 0 and vel = 1 => gain
  // vel_s = 1 and vel = 0 => 0
  // vel_s = 1 and vel = 1 => gain
  // vel_s = 0.8,
  //   vel = [0,1] => 0.2gain + 0.8gain*vel
  // vel_s = 0.1,
  //   vel = [0,1] => 0.9gain + 0.1gain*vel

  return op->gain_c * ((1.0 - op->vel_s) + op->vel_s * op->velocity) * sample;
}

FTYPE
operator_sample(Operator op, bool sustain)
{
  FTYPE s1, s2;

  s1 = ugen_sample_mod(op->ugen, op->mod);

  switch (op->e_type) {
  case OPERATOR_UGEN:
    s2 = ugen_sample_mod(op->env_u.lfo, 0.0);
    break;
  case OPERATOR_ENV:
    s2 = env_sample(op->env_u.env, sustain);
    break;
  default:
    s2 = 1.0;
    break;
  }

  return operator_sample_apply_gain(op, s1 * s2);
}
