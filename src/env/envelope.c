#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#include "envelope.h"

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
env_default_amp(Envelope env)
{
  env->amps[0] = 0.2;
  env->amps[1] = 0.6;
  env->amps[2] = 0.3;
  env->amps[3] = 0.2;
}

void
env_default_durations(Envelope env)
{
  env->durs[0] = 0.05;
  env->durs[1] = 0.05;
  env->durs[2] = 0.85;
  env->durs[3] = 0.05;

  env->max_samples[0] = floor(DEFAULT_SAMPLE_RATE * env->durs[0]);
  env->max_samples[1] = floor(DEFAULT_SAMPLE_RATE * env->durs[1]);
  env->max_samples[2] = floor(DEFAULT_SAMPLE_RATE * env->durs[2]);
  env->max_samples[3] = floor(DEFAULT_SAMPLE_RATE * env->durs[3]);
}

void
env_default_ugens(Envelope env)
{
  env->ugens[0] = ugen_init_ramp_linear(1.0 / env->durs[0]);
  env->ugens[1] = ugen_init_ramp_linear(1.0 / env->durs[1]);
  env->ugens[2] = ugen_init_ramp_linear(1.0 / env->durs[2]);
  env->ugens[3] = ugen_init_ramp_linear(1.0 / env->durs[3]);

  ugen_set_scale(env->ugens[0], env->amps[0], env->amps[1]);
  ugen_set_scale(env->ugens[1], env->amps[1], env->amps[2]);
  ugen_set_scale(env->ugens[2], env->amps[2], env->amps[3]);
  ugen_set_scale(env->ugens[3], env->amps[3], 0.0);
}

void
env_reset(Envelope env)
{
  env->p_ind = 0;
  env->state = ENV_ATTACK;
  ugen_reset_phase(env->ugens[0]);
  ugen_reset_phase(env->ugens[1]);
  ugen_reset_phase(env->ugens[2]);
  ugen_reset_phase(env->ugens[3]);
}

void
env_set_duration(Envelope env, FTYPE duration/*in seconds*/, env_state stage)
{
  env_reset(env);
  env->durs[stage] = duration;
  env->max_samples[stage] = floor(DEFAULT_SAMPLE_RATE * env->durs[stage]);
  ugen_set_freq(env->ugens[stage], 1.0 / duration);
}

Envelope
env_init()
{
  Envelope env = env_alloc();
  // null checks

  return env;
}

Envelope
env_init_default()
{
  Envelope env = env_init();
  env_default_amp(env);
  env_default_durations(env);
  env_default_ugens(env);

  // init the phase index as complete
  env->p_ind = env->max_samples[0]
             + env->max_samples[1]
             + env->max_samples[2]
             + env->max_samples[3];
  return env;
}

void
env_cleanup(Envelope env)
{
  ugen_cleanup(env->ugens[0]);
  ugen_cleanup(env->ugens[1]);
  ugen_cleanup(env->ugens[2]);
  ugen_cleanup(env->ugens[3]);
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
    sample = ugen_sample(env->ugens[env->state]);
    env->prev_sample = sample;
  }

  switch(env->state) {
  case ENV_ATTACK:
    if (env->p_ind == env->max_samples[0]) {
      env->state = ENV_DECAY;
    }
    break;
  case ENV_DECAY:
    if (env->p_ind == env->max_samples[0] + env->max_samples[1]) {
      env->state = ENV_SUSTAIN;
    }
    break;
  case ENV_SUSTAIN:
    if (env->p_ind == (env->max_samples[0] + env->max_samples[1] + env->max_samples[2])) {
      if (sustain) {
        env->p_ind--;
      } else {
        env->state = ENV_RELEASE;
      }
    }
    break;
  case ENV_RELEASE:
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
