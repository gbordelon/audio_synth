#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

#include "../lib/macros.h"

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
  env->amps[0] = 0.0;
  env->amps[1] = 1.0;
  env->amps[2] = 0.7;
  env->amps[3] = 0.6;
}

void
env_default_durations(Envelope env)
{
  env->durs[0] = 0.05;
  env->durs[1] = 0.05;
  env->durs[2] = 0.80;
  env->durs[3] = 0.10;

  env->max_samples[0] = floor(DEFAULT_SAMPLE_RATE * env->durs[0]) - 1;
  env->max_samples[1] = floor(DEFAULT_SAMPLE_RATE * env->durs[1]) - 1;
  env->max_samples[2] = floor(DEFAULT_SAMPLE_RATE * env->durs[2]) - 1;
  env->max_samples[3] = floor(DEFAULT_SAMPLE_RATE * env->durs[3]) - 1;
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
env_set_release(Envelope env)
{
  if (env->state != ENV_RELEASE) {
    env->p_ind = env->max_samples[0] + env->max_samples[1] + env->max_samples[2];
    env->p_inc = (0.0 - env->prev_sample) / (FTYPE)env->max_samples[3];
    env->state = ENV_RELEASE;
  }
}

void
env_set_duration(Envelope env, FTYPE duration/*in seconds*/, env_state stage)
{
  env->durs[stage] = duration;
  env->max_samples[stage] = floor(DEFAULT_SAMPLE_RATE * env->durs[stage]) - 1;
}

void
env_set_amplitudes(Envelope env, FTYPE amps[4])
{
  env->amps[0] = amps[0];
  env->amps[1] = amps[1];
  env->amps[2] = amps[2];
  env->amps[3] = amps[3];
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
  env->decay_rate = 0.0;
  env->p_inc = 0.0;

  // init the phase index as complete
  env->p_ind = env_max_duration(env);
  env->state = ENV_RELEASE;
  return env;
}

void
env_cleanup(Envelope env)
{
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
    sample = env->p_inc + env->prev_sample;
    if (env->state == ENV_ATTACK && sample > env->amps[ENV_DECAY]) {
      sample = env->amps[ENV_DECAY];
    }
    env->prev_sample = sample;
  }

  switch(env->state) {
  case ENV_ATTACK:
    if (env->p_ind == env->max_samples[0]) {
      env->p_inc = (env->amps[2] - env->amps[1]) / (FTYPE)env->max_samples[1];
      env->state = ENV_DECAY;
    }
    break;
  case ENV_DECAY:
    if (env->p_ind == env->max_samples[0] + env->max_samples[1]) {
      env->p_inc = (env->amps[3] - env->amps[2]) / (FTYPE)env->max_samples[2];
      env->state = ENV_SUSTAIN;
    }
    break;
  case ENV_SUSTAIN:
    if (env->p_ind == (env->max_samples[0] + env->max_samples[1] + env->max_samples[2])) {
      if (sustain) {
        env->p_ind--;
        if (env->prev_sample > 0.0) {
          env->prev_sample += env->decay_rate;
        } else if (env->prev_sample < 0.0) {
          env->prev_sample = 0.0;
          env_set_release(env);
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
