#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "../lib/macros.h"

#include "envelope.h"

#include "../cli/cli.h"
#include "../tunable/tunable.h"

Envelope
env_alloc()
{
  Envelope env = calloc(1, sizeof(struct envelope_t));
  return env;
}

void
env_free(Envelope env)
{
  free(env);
}

void
env_default_durations(Envelope env)
{
}

void
env_reset(Envelope env)
{
  env->p_ind = 0;
  env->state = ENV_ATTACK;
  env->p_inc = (env->amps[1] - env->amps[0]) / (FTYPE)env->max_samples[0];
  env->prev_sample = 0.0;
}

void
env_set_attack(Envelope env)
{
  if (env->state != ENV_ATTACK) {
    env_reset(env);
  }
}

void
env_set_decay(Envelope env)
{
  if (env->state != ENV_DECAY) {
    env->p_inc = (env->amps[2] - env->amps[1]) / (FTYPE)env->max_samples[1];
    env->state = ENV_DECAY;
  }
}

void
env_set_sustain(Envelope env)
{
  if (env->state != ENV_SUSTAIN) {
    env->p_inc = (env->amps[3] - env->amps[2]) / (FTYPE)env->max_samples[2];
    env->state = ENV_SUSTAIN;
  }
}

void
env_set_release(Envelope env)
{
  if (env->state != ENV_RELEASE) {
    env->state = ENV_RELEASE;
  }
  if (env->p_ind != env->max_samples[0] + env->max_samples[1] + env->max_samples[2]) {
    env->p_ind = env->max_samples[0] + env->max_samples[1] + env->max_samples[2];
    env->p_inc = (0.0 - env->prev_sample) / (FTYPE)env->max_samples[3];
  } else {
    env->p_inc = (0.0 - env->amps[3]) / (FTYPE)env->max_samples[3];
  }
}

void
env_set_sample_rate(Envelope env, FTYPE sample_rate)
{
  env->sample_rate = sample_rate;

  int i;
  for (i = ENV_ATTACK; i < ENV_NUM_STAGES; i++) {
    env_set_rate(env, env->rates[i], i);
  }
  env_reset(env);
}

void
env_update_max_sample(Envelope env, env_state stage)
{
  env->max_samples[stage] = floor(env->sample_rate / env->rates[stage]) - 1;
}

void
env_update_max_samples(Envelope env)
{
  int i;
  for (i = 0; i < ENV_NUM_STAGES; i++) {
    env_update_max_sample(env, i);
  }
}

void
env_set_rate(Envelope env, FTYPE rate/*in 1.0 / seconds*/, env_state stage)
{
  env->rates[stage] = rate;
  env_update_max_sample(env, stage);
}

void
env_set_duration(Envelope env, FTYPE duration/*in seconds*/, env_state stage)
{
  env_set_rate(env, 1.0 / duration, stage);
}

void
env_set_amplitude(Envelope env, FTYPE amp, env_state stage)
{
  env->amps[stage] = amp;
}

void
env_set_amplitudes(Envelope env, FTYPE amps[4])
{
  env->amps[0] = amps[0];
  env->amps[1] = amps[1];
  env->amps[2] = amps[2];
  env->amps[3] = amps[3];
}

void
env_set_decay_rate(Envelope env, FTYPE decay_rate/* [0,1] */)
{
  env->decay_rate = decay_rate;
}

void
env_set_params(Envelope env, envelope_params *p)
{
  env_set_decay_rate(env, p->decay_rate);
  env_set_amplitudes(env, p->amps);

  int i;
  for (i = ENV_ATTACK; i < ENV_NUM_STAGES; i++) {
    env_set_rate(env, p->rates[i], i);
  }
  env_reset(env);
}

Envelope
env_init(envelope_params p)
{
  Envelope env = env_alloc();
  // null checks

  env->sample_rate = p.sample_rate; // specifically don't call env_set_sample_rate here
  env_set_params(env, &p);

  // init the phase index as complete
  env->p_inc = 0.0;
  env->p_ind = env_max_duration(env);
  env->state = ENV_RELEASE;

  // create envelope menu
  env->menu = cli_menu_init(CLI_MENU, "Envelope Menu", "Edit envelope parameters");
  // attach a copy of params
  env->tunables.p = p;
  // track tunables for freeing
  env->tunables.ts = calloc(9, sizeof(Tunable));

  tunable_arg args[2];
  tunable_fn fn;
  args[0].v = (void *)env;
  args[1].v = (void *)&env->tunables.p;
  fn.f2pp = env_set_params;

  // create decay_rate tunable
  Tunable t = tunable_init(TUNABLE_DOUBLE,
      TUNABLE_RANGE_0_1,
      &env->tunables.p.decay_rate,
      args,
      ARITY_2_PP,
      &fn,
      "Envelope Decay Rate [0-1]"
  );
  env->tunables.ts[0] = t;
  cli_menu_add_tunable(env->menu, t);

  // attach attack amplitude and rate tunables
  t = tunable_init(
        TUNABLE_DOUBLE,
        TUNABLE_RANGE_0_1,
        &env->tunables.p.amps[ENV_ATTACK],
        args,
        ARITY_2_PP,
        &fn,
        "Envelope Attack Amplitude [0-1]"
  );
  env->tunables.ts[1] = t;
  cli_menu_add_tunable(env->menu, t);
  t = tunable_init(
        TUNABLE_DOUBLE,
        TUNABLE_RANGE_0_24000,
        &env->tunables.p.rates[ENV_ATTACK],
        args,
        ARITY_2_PP,
        &fn,
        "Envelope Attack Rate [0-24000]"
  );
  env->tunables.ts[2] = t;
  cli_menu_add_tunable(env->menu, t);
  // attach decay amplitude and rate tunables
  t = tunable_init(
        TUNABLE_DOUBLE,
        TUNABLE_RANGE_0_1,
        &env->tunables.p.amps[ENV_DECAY],
        args,
        ARITY_2_PP,
        &fn,
        "Envelope Decay Amplitude [0-1]"
  );
  env->tunables.ts[3] = t;
  cli_menu_add_tunable(env->menu, t);
  t = tunable_init(
        TUNABLE_DOUBLE,
        TUNABLE_RANGE_0_24000,
        &env->tunables.p.rates[ENV_DECAY],
        args,
        ARITY_2_PP,
        &fn,
        "Envelope Decay Rate [0-24000]"
  );
  env->tunables.ts[4] = t;
  cli_menu_add_tunable(env->menu, t);
  // attach sustain amplitude and rate tunables
  t = tunable_init(
        TUNABLE_DOUBLE,
        TUNABLE_RANGE_0_1,
        &env->tunables.p.amps[ENV_SUSTAIN],
        args,
        ARITY_2_PP,
        &fn,
        "Envelope Sustain Amplitude [0-1]"
  );
  env->tunables.ts[5] = t;
  cli_menu_add_tunable(env->menu, t);
  t = tunable_init(
        TUNABLE_DOUBLE,
        TUNABLE_RANGE_0_24000,
        &env->tunables.p.rates[ENV_SUSTAIN],
        args,
        ARITY_2_PP,
        &fn,
        "Envelope Sustain Rate [0-24000]"
  );
  env->tunables.ts[6] = t;
  cli_menu_add_tunable(env->menu, t);
  // attach release amplitude and rate tunables
  t = tunable_init(
        TUNABLE_DOUBLE,
        TUNABLE_RANGE_0_1,
        &env->tunables.p.amps[ENV_RELEASE],
        args,
        ARITY_2_PP,
        &fn,
        "Envelope Release Amplitude [0-1]"
  );
  env->tunables.ts[7] = t;
  cli_menu_add_tunable(env->menu, t);
  t = tunable_init(
        TUNABLE_DOUBLE,
        TUNABLE_RANGE_0_24000,
        &env->tunables.p.rates[ENV_RELEASE],
        args,
        ARITY_2_PP,
        &fn,
        "Envelope Release Rate [0-24000]"
  );
  env->tunables.ts[8] = t;
  cli_menu_add_tunable(env->menu, t);

  return env;
}

Envelope
env_init_default()
{
  envelope_params p = {
    .amps = { 0.0, 1.0, 0.7, 0.5 },
    .rates = { 8.0, 4.0, 1.0, 20.0 },
    .decay_rate = 1.0,
    .sample_rate = (FTYPE)DEFAULT_SAMPLE_RATE
  };
  Envelope env = env_init(p);
  return env;
}

void
env_cleanup(Envelope env)
{
  cli_menu_cleanup(env->menu);
  int i;
  for (i = 0; i < 9; i++) {
    tunable_cleanup(env->tunables.ts[i]);
  }
  free(env->tunables.ts);

  env_free(env);
}

FTYPE
env_sample(Envelope env, bool sustain)
{
  if (env_spent(env)) {
    return 0.0;
  }
  env->p_ind++;

  FTYPE sample;
  if (sustain && env->p_ind == (env->max_samples[0] + env->max_samples[1] + env->max_samples[2])) {
    sample = env->prev_sample;
  } else {
    // env->p_inc never changes so this is a linear decay.
    sample = env->p_inc + env->prev_sample;
    if (env->state == ENV_ATTACK) {
    } else if (env->state == ENV_DECAY) {
    } else if (env->state == ENV_SUSTAIN) {
    } else if (env->state == ENV_RELEASE) {
    }
    env->prev_sample = sample;
  }

  switch(env->state) {
  case ENV_ATTACK:
    if (env->p_ind == env->max_samples[0]) {
      env_set_decay(env);
    }
    break;
  case ENV_DECAY:
    if (env->p_ind == env->max_samples[0] + env->max_samples[1]) {
      env_set_sustain(env);
    }
    break;
  case ENV_SUSTAIN:
    if (env->p_ind == (env->max_samples[0] + env->max_samples[1] + env->max_samples[2])) {
      if (sustain) {
        env->p_ind--;
        if (fabs(env->prev_sample) > 0.000001) {
          env->prev_sample *= env->decay_rate;
        } else {
          env->prev_sample = 0.0;
        }
      } else {
        env_set_release(env);
      }
    }
    break;
  case ENV_RELEASE:
  // fall through
  default:
    break;
  }

  return sample; 
}

void
env_sample_chunk(Envelope env, bool sustain, FTYPE *buf)
{
  int i;
  for (i = 0; i < CHUNK_SIZE; i++, buf++) {
    *buf = env_sample(env, sustain);
  }
}
