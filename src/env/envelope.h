#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <stdbool.h>
#include <stdint.h>

#include "../lib/macros.h"

#define env_spent(env) ((env)->p_ind >= \
((env)->max_samples[ENV_ATTACK] + \
 (env)->max_samples[ENV_DECAY] + \
 (env)->max_samples[ENV_SUSTAIN] + \
 (env)->max_samples[ENV_RELEASE]))

#define env_max_duration(env) \
((env)->max_samples[ENV_ATTACK] + \
 (env)->max_samples[ENV_DECAY] + \
 (env)->max_samples[ENV_SUSTAIN] + \
 (env)->max_samples[ENV_RELEASE])

typedef enum env_state_e {
  ENV_ATTACK,
  ENV_DECAY,
  ENV_SUSTAIN,
  ENV_RELEASE,
  ENV_NUM_STAGES
} env_state;

typedef struct envelope_params_t {
  FTYPE amps[ENV_NUM_STAGES];
  FTYPE rates[ENV_NUM_STAGES];
  FTYPE decay_rate;
  FTYPE sample_rate;
} envelope_params;

typedef struct envelope_t {
  FTYPE sample_rate;
  // between 0 and 1
  FTYPE amps[ENV_NUM_STAGES];

  //ugen_type curves[4];
  FTYPE rates[ENV_NUM_STAGES]; // 1.0 / (duration of each stage in seconds)
  uint32_t max_samples[ENV_NUM_STAGES]; // number of samples to play for each stage
  uint32_t p_ind; // sample counter for state changes
  FTYPE p_inc;
  FTYPE prev_sample;
  FTYPE decay_rate; // 0 means infinite sustain while mv->sustain is true

  env_state state;
} *Envelope;

Envelope env_init(envelope_params params);
Envelope env_init_default();

void env_cleanup(Envelope env);
void env_reset(Envelope env);
void env_set_release(Envelope env);

void env_set_amplitudes(Envelope env, FTYPE amps[ENV_NUM_STAGES]);
void env_set_rate(Envelope env, FTYPE rate/*in 1.0 / seconds*/, env_state stage);
void env_set_duration(Envelope env, FTYPE duration/*in seconds*/, env_state stage);
void env_set_sample_rate(Envelope env, FTYPE sample_rate);
FTYPE env_sample(Envelope env, bool sustain); // sampling an envelope after it expires should return 0.
void env_sample_chunk(Envelope env, bool sustain, FTYPE *buf);

#endif
