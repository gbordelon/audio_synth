#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <stdbool.h>
#include <stdint.h>
#include "../lib/macros.h"

// TODO support other function types for more interesting envelope shapes.
// assume linear function between sections of the envelope.
// e.g. circular, sinusoidal, quadratic, cubic, quartic, exponential, logarithmic.
// I'm guessing I would precompute a table for each function, like with the oscillators
typedef enum env_curve_e {
  ENV_LINEAR,
} env_curve;

typedef enum env_state_e {
  ENV_ATTACK,
  ENV_DECAY,
  ENV_SUSTAIN,
  ENV_RELEASE
} env_state;

typedef struct envelope_t {
  // between 0 and 1
  FTYPE amps[4];

  // amount time spend in each envelope section represented as a number of samples
  // sum to duration of tone for the sample rate.
  uint32_t durs[4];
  env_curve curves[4];

  uint32_t p_ind; // phase index
  uint32_t dur; // phase index
  env_state state;
  FTYPE *table;
} *Envelope;

Envelope env_init(uint32_t duration); // at 48khz this allows for ~1000s tone length
// TODO more powerful constructor
void env_cleanup(Envelope env);

void env_set_duration(Envelope env, uint32_t duration);
FTYPE env_sample(Envelope env); // sampling an envelope after it expires should return 0.
void env_sample_chunk(Envelope env, bool sustain, FTYPE *buf);

#endif
