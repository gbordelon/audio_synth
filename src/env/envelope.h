#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <stdbool.h>
#include <stdint.h>

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#define env_spent(env) ((env)->p_ind >= ((env)->max_samples[0] + (env)->max_samples[1] + (env)->max_samples[2] + (env)->max_samples[3]))

typedef enum env_state_e {
  ENV_ATTACK,
  ENV_DECAY,
  ENV_SUSTAIN,
  ENV_RELEASE
} env_state;

typedef struct envelope_t {
  // between 0 and 1
  FTYPE amps[4];
  Ugen ugens[4];

  //ugen_type curves[4];
  FTYPE durs[4]; // duration of each stage in seconds
  uint32_t max_samples[4]; // number of samples to play for each stage
  uint32_t p_ind; // sample counter for state changes
  FTYPE prev_sample;

  env_state state;
} *Envelope;

Envelope env_init_default();
// TODO more powerful constructor

void env_cleanup(Envelope env);
void env_reset(Envelope env);

void env_set_duration(Envelope env, FTYPE duration/*in seconds*/, env_state stage);
FTYPE env_sample(Envelope env, bool sustain); // sampling an envelope after it expires should return 0.
void env_sample_chunk(Envelope env, bool sustain, FTYPE *buf);

#endif
