#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <stdbool.h>
#include <stdint.h>
#include "../lib/macros.h"

// TODO support other function types for more interesting envelope shapes.
// assume linear function between sections of the envelope.
// e.g. circular, sinusoidal, quadratic, cubic, quartic, exponential, logarithmic.
// I'm guessing I would precompute a table for each function, like with the oscillators
enum env_curve {
  ENV_LINEAR,
};

enum env_state {
  ENV_ATTACK,
  ENV_DECAY,
  ENV_SUSTAIN,
  ENV_RELEASE
};

typedef struct envelope_t {
  // between 0 and 1
  FTYPE a_amp;
  FTYPE d_amp;
  FTYPE s_amp;
  FTYPE r_amp;

  // amount time spend in each envelope section represented as a number of samples
  // sum to duration of tone for the sample rate.
  uint32_t a_dur;
  uint32_t d_dur;
  uint32_t s_dur;
  uint32_t r_dur;

  enum env_curve a_curve;
  enum env_curve d_curve;
  enum env_curve s_curve;
  enum env_curve r_curve;

  uint32_t p_ind; // phase index
  uint32_t dur; // phase index
  enum env_state state;
  FTYPE *table;
} *Envelope;

Envelope env_init(uint32_t duration); // at 48khz this allows for ~1000s tone length
// TODO more powerful constructor
void env_cleanup(Envelope env);

void env_set_duration(Envelope env, uint32_t duration);
FTYPE env_sample(Envelope env); // sampling an envelope after it expires should return 0.
void env_sample_chunk(Envelope env, bool sustain, FTYPE *buf);

#endif
