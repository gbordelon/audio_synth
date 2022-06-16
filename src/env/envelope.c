#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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
env_populate_table_section_linear(FTYPE start, FTYPE end, size_t num_steps, FTYPE *buf)
{
  FTYPE step = (end - start) / (FTYPE)num_steps;
  FTYPE *b;

  *buf = start;
  for (b = buf; b - buf + 1 < num_steps; b++) {
    *(b + 1) = *b + step;
  }
}

// TODO support more than just linear
void
env_populate_table(Envelope env)
{
  env_populate_table_section_linear(env->amps[0], env->amps[1], env->durs[0], env->table);
  env_populate_table_section_linear(env->amps[1], env->amps[2], env->durs[1], env->table + env->durs[0]);
  env_populate_table_section_linear(env->amps[2], env->amps[3], env->durs[2], env->table + env->durs[0] + env->durs[1]);
  env_populate_table_section_linear(env->amps[3], 0.0, env->durs[3], env->table + env->durs[0] + env->durs[1] + env->durs[2]);
}

void
env_default_amp(Envelope env)
{
  env->amps[0] = 0.5;
  env->amps[1] = 0.9;
  env->amps[2] = 0.4;
  env->amps[3] = 0.2;
}

void
env_default_dur(Envelope env, uint32_t duration)
{
  env->durs[0] = floor(0.05 * (FTYPE)duration);
  env->durs[1] = floor(0.05 * (FTYPE)duration);
  env->durs[2] = floor(0.85 * (FTYPE)duration);
  env->durs[3] = floor(0.05 * (FTYPE)duration);

  // TODO find a better way to deal with the rounding slop
  env->durs[3] += duration - (env->durs[0] + env->durs[1] + env->durs[2] + env->durs[3]);
  env_set_duration(env, duration);
}

void
env_default_curve(Envelope env)
{
  env->curves[0] = ENV_LINEAR;
  env->curves[1] = ENV_LINEAR;
  env->curves[2] = ENV_LINEAR;
  env->curves[3] = ENV_LINEAR;
}

void
env_default(Envelope env, uint32_t duration)
{
  env_default_amp(env);
  env_default_curve(env);
  env_default_dur(env, duration);
}

void
env_reset(Envelope env)
{
  env->p_ind = 0;
  env->state = ENV_ATTACK;
}

void
env_set_duration(Envelope env, uint32_t duration)
{
  env_reset(env);
  if (env->dur < duration) {
    env->table = realloc(env->table, 1 * duration * sizeof(FTYPE));
    // null checks
  }
  if (env->dur != duration) {
    env->dur = duration;
    env_populate_table(env);
  }
}

Envelope
env_init(uint32_t duration)
{
  Envelope env = env_alloc();
  // null checks

  env_default(env, duration);

  return env;
}

void
env_cleanup(Envelope env)
{
  env_free(env);
}

FTYPE
env_sample(Envelope env)
{
  if (env->p_ind >= env->dur) {
    return 0.0;
  }

  return env->table[env->p_ind++];
}

void
env_sample_chunk(Envelope env, bool sustain, FTYPE *buf)
{
  int i;
  for (i = 0; i < CHUNK_SIZE; i++, buf++) {
    *buf = (env->p_ind >= env->dur)
           ? 0.0
           : env->table[env->p_ind++];

    switch(env->state) {
    case ENV_ATTACK:
      if (env->p_ind == env->durs[0]) {
        env->state = ENV_DECAY;
      }
      break;
    case ENV_DECAY:
      if (env->p_ind == env->durs[0] + env->durs[1]) {
        env->state = ENV_SUSTAIN;
      }
      break;
    case ENV_SUSTAIN:
      // TODO do something more interesting?
      if (sustain) {
        env->p_ind--;
      }
      if (env->p_ind == env->durs[0] + env->durs[1] + env->durs[2]) {
        env->state = ENV_RELEASE;
      }
      break;
    case ENV_RELEASE:
      break;
    } 
  }
}
