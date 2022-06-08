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

// TODO support more than just linear
void
env_populate_table(Envelope env)
{
  int i;

  FTYPE step = (env->d_amp - env->a_amp) / (FTYPE)env->a_dur;
  env->table[0] = env->a_amp + step;
  for (i = 1; i < env->a_dur; i++) {
    env->table[i] = env->table[i-1] + step;
  }

  step = (env->s_amp - env->d_amp) / (FTYPE)env->d_dur;
  for (; i < env->a_dur + env->d_dur; i++) {
    env->table[i] = env->table[i-1] + step;
  }

  step = (env->r_amp - env->s_amp) / (FTYPE)env->s_dur;
  for (; i < env->a_dur + env->d_dur + env->s_dur; i++) {
    env->table[i] = env->table[i-1] + step;
  }

  step = (0.0 - env->r_amp) / (FTYPE)env->r_dur;
  for (; i < env->a_dur + env->d_dur + env->s_dur + env->r_dur; i++) {
    env->table[i] = env->table[i-1] + step;
  }
}

void
env_default(Envelope env, uint32_t duration)
{
  env->a_amp = 0.5;
  env->d_amp = 0.9;
  env->s_amp = 0.4;
  env->r_amp = 0.2;

  env->a_dur = floor(0.05 * (FTYPE)duration);
  env->d_dur = floor(0.05 * (FTYPE)duration);
  env->s_dur = floor(0.85 * (FTYPE)duration);
  env->r_dur = floor(0.05 * (FTYPE)duration);

  // TODO find a better way to deal with the rounding slop
  env->r_dur += duration - (env->a_dur + env->d_dur + env->s_dur + env->r_dur);

  env->a_curve = ENV_LINEAR;
  env->d_curve = ENV_LINEAR;
  env->s_curve = ENV_LINEAR;
  env->r_curve = ENV_LINEAR;
}

Envelope
env_init(uint32_t duration)
{
  Envelope env = env_alloc();
  // null checks

  env->p_ind = 0;
  env->dur = duration;
  env->table = calloc(1, duration * sizeof(FTYPE));
  // null checks

  env_default(env, duration);
  env_populate_table(env);

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
