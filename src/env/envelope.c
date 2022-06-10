#include <math.h>
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
  env_populate_table_section_linear(env->a_amp, env->d_amp, env->a_dur, env->table);
  env_populate_table_section_linear(env->d_amp, env->s_amp, env->d_dur, env->table + env->a_dur);
  env_populate_table_section_linear(env->s_amp, env->r_amp, env->s_dur, env->table + env->a_dur + env->d_dur);
  env_populate_table_section_linear(env->r_amp, 0.0, env->r_dur, env->table + env->a_dur + env->d_dur + env->s_dur);
}

void
env_default_amp(Envelope env)
{
  env->a_amp = 0.5;
  env->d_amp = 0.9;
  env->s_amp = 0.4;
  env->r_amp = 0.2;
}

void
env_default_dur(Envelope env, uint32_t duration)
{
  env->a_dur = floor(0.05 * (FTYPE)duration);
  env->d_dur = floor(0.05 * (FTYPE)duration);
  env->s_dur = floor(0.85 * (FTYPE)duration);
  env->r_dur = floor(0.05 * (FTYPE)duration);

  // TODO find a better way to deal with the rounding slop
  env->r_dur += duration - (env->a_dur + env->d_dur + env->s_dur + env->r_dur);
  env_set_duration(env, duration);
}

void
env_default_curve(Envelope env)
{
  env->a_curve = ENV_LINEAR;
  env->d_curve = ENV_LINEAR;
  env->s_curve = ENV_LINEAR;
  env->r_curve = ENV_LINEAR;
}

void
env_default(Envelope env, uint32_t duration)
{
  env_default_amp(env);
  env_default_curve(env);
  env_default_dur(env, duration);
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
env_sample_chunk(Envelope env, FTYPE *buf)
{
  int i;
  for (i = 0; i < CHUNK_SIZE; i++, buf++) {
    *buf = (env->p_ind >= env->dur)
           ? 0.0
           : env->table[env->p_ind++];
  }
}
