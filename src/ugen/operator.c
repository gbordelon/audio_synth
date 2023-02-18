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

  cli_menu_cleanup(op->menu);
  int i;
  for (i = 0; i < 5; i++) {
    tunable_cleanup(op->tunables.ts[i]);
  }
  free(op->tunables.ts);

  operator_free(op);
}

void
operator_set_params(Operator op, operator_params *p)
{
  bool detune = op->detune != p->detune;

  op->gain_c = p->gain_c;
  op->pan = p->pan;
  op->mult = p->mult;
  op->vel_s = p->vel_s;

  if (detune) {
    operator_set_detune(op, p->detune);
  }

  if (op->u_type != p->u_type) {
    ugen_cleanup(op->ugen);
    op->ugen = NULL;
    if (op->dfo != NULL) {
      dfo_cleanup(op->dfo);
      op->dfo = NULL;
    }
    switch (p->u_type) {
    case UGEN_OSC_IMP:
      op->ugen = ugen_init_imp(0.0, 0.5, p->sample_rate);
      break;
    case UGEN_OSC_SAW:
      op->ugen = ugen_init_saw(0.0, p->sample_rate);
      break;
    case UGEN_OSC_TRI:
      op->ugen = ugen_init_tri(0.0, p->sample_rate);
      break;
    default:
    // fall through
    case UGEN_OSC_SIN:
      op->ugen = ugen_init_sin(0.0, p->sample_rate);
      op->dfo = dfo_init(p->sample_rate);
      break;
    }
    op->u_type = p->u_type;
  }

  if (op->e_type != p->e_type) {
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
    switch (p->e_type) {
    case OPERATOR_UGEN:
      op->env_u.lfo = ugen_init_sin(1.0, p->sample_rate);
      ugen_set_scale(op->env_u.lfo, 0.0, 1.0);
    // TODO attach menu item for ugen
  //    cli_menu_add_menu(op->menu, op->env_u.lfo->menu);
      break;
    case OPERATOR_ENV:
      op->env_u.env = env_init_default();
      env_set_sample_rate(op->env_u.env, p->sample_rate);
      cli_menu_add_menu(op->menu, op->env_u.env->menu);
      break;
    default:
      // none
      break;
    }
    op->e_type = p->e_type;
  }
}

Operator
operator_init(operator_params *p)
{
  Operator op = operator_alloc();
  op->fc = 0.0;
  op->mod = 0.0;
  op->u_type = p->u_type;
  op->e_type = p->e_type;

  operator_set_params(op, p);

  // create operator menu
  op->menu = cli_menu_init(CLI_MENU, "Operator Menu", "Edit operator parameters");
  // attach a copy of params
  op->tunables.p = *p;
  // track tunables for freeing
  op->tunables.ts = calloc(5, sizeof(Tunable));

  tunable_arg args[2];
  tunable_fn fn;
  args[0].v = (void *)op;
  args[1].v = (void *)&op->tunables.p;
  fn.f2pp = operator_set_params;

  // create detune tunable
  Tunable t = tunable_init(
    TUNABLE_DOUBLE,
    TUNABLE_RANGE_NONE,
    &op->tunables.p.detune,
    args,
    ARITY_2_PP,
    &fn,
    "Operator Carrier Frequency Detune [cents, 1200 per octave]"
  );
  op->tunables.ts[0] = t;
  cli_menu_add_tunable(op->menu, t);

  // create gain tunable
  t = tunable_init(
    TUNABLE_DOUBLE,
    TUNABLE_RANGE_0_1,
    &op->tunables.p.gain_c,
    args,
    ARITY_2_PP,
    &fn,
    "Operator Gain [0-1]"
  );
  op->tunables.ts[1] = t;
  cli_menu_add_tunable(op->menu, t);

  // create pan tunable
  t = tunable_init(
    TUNABLE_DOUBLE,
    TUNABLE_RANGE_0_1,
    &op->tunables.p.pan,
    args,
    ARITY_2_PP,
    &fn,
    "Operator Pan [0-1]"
  );
  op->tunables.ts[2] = t;
  cli_menu_add_tunable(op->menu, t);

  // create mult tunable
  t = tunable_init(
    TUNABLE_DOUBLE,
    TUNABLE_RANGE_NONE,
    &op->tunables.p.mult,
    args,
    ARITY_2_PP,
    &fn,
    "Operator Carrier Frequency Multiplier [>0]"
  );
  op->tunables.ts[3] = t;
  cli_menu_add_tunable(op->menu, t);

  // create velocity sensitivity tunable
  t = tunable_init(
    TUNABLE_DOUBLE,
    TUNABLE_RANGE_0_1,
    &op->tunables.p.vel_s,
    args,
    ARITY_2_PP,
    &fn,
    "Operator Velocity Sensitivity [0-1]"
  );
  op->tunables.ts[4] = t;
  cli_menu_add_tunable(op->menu, t);

  ugen_params up = {
    .shape = p->u_type,
    .duty_cycle = 0.5,
    .freq = 0.0,
    .scale_low = -1.0,
    .scale_high = 1.0,
    .sample_rate = p->sample_rate,
  };
  op->ugen = ugen_init_with_params(&up);
  if (p->u_type == UGEN_OSC_SIN) {
    op->dfo = dfo_init(p->sample_rate);
  }
  cli_menu_add_menu(op->menu, op->ugen->menu);

  // TODO support configuration of EG at init time
  switch (p->e_type) {
  case OPERATOR_UGEN:
    up.shape = UGEN_OSC_SIN;
    up.freq = 1.0;
    up.scale_low = 0.0;
    op->env_u.lfo = ugen_init_with_params(&up);
    cli_menu_add_menu(op->menu, op->env_u.lfo->menu);
    break;
  case OPERATOR_ENV:
    op->env_u.env = env_init_default();
    env_set_sample_rate(op->env_u.env, p->sample_rate);
    cli_menu_add_menu(op->menu, op->env_u.env->menu);
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
  operator_params p = {
    .sample_rate = DEFAULT_SAMPLE_RATE,
    .u_type = UGEN_OSC_SIN,
    .e_type = OPERATOR_ENV,
    .mult = 1.0,
    .detune = 0.0,
    .vel_s = 1.0,
    .gain_c = 1.0,
    .pan = 0.5
  };
  return operator_init(&p);
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
cent = 1200 * log₂(f₂ / f₁)
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
  if (op->dfo != NULL) {
    dfo_set_freq(op->dfo, op->mult * op->fc);
  }
}

void
operator_set_detune(Operator op, FTYPE detune)
{
  op->detune = detune;
  operator_set_fc(op, op->fc);
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
