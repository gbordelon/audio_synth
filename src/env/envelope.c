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
env_set_rate(Envelope env, FTYPE rate/*in 1.0 / seconds*/, env_state stage)
{
  env->rates[stage] = rate;
  env->max_samples[stage] = floor(env->sample_rate / rate) - 1;
}

void
env_set_duration(Envelope env, FTYPE duration/*in seconds*/, env_state stage)
{
  env_set_rate(env, 1.0 / duration, stage);
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
env_init(envelope_params p)
{
  Envelope env = env_alloc();
  // null checks

  env->decay_rate = p.decay_rate;
  env->sample_rate = p.sample_rate;
  env->p_inc = 0.0;

  int i;
  for (i = ENV_ATTACK; i < ENV_NUM_STAGES; i++) {
    env->amps[i] = p.amps[i];
    env->rates[i] = p.rates[i];
    env->max_samples[i] = floor(p.sample_rate / env->rates[i]) - 1;
  }

  // init the phase index as complete
  env->p_ind = env_max_duration(env);
  env->state = ENV_RELEASE;
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
