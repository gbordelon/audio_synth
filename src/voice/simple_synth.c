#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../midi/midi.h"
#include "../ugen/imp.h"
#include "../ugen/sin.h"
#include "../ugen/ugen.h"

#include "voice.h"
#include "simple_synth.h"

void
simple_synth_init(MonoVoice mv, mono_voice_params params)
{
  mv->params.ss.ops[0] = operator_init(UGEN_OSC_SIN, OPERATOR_ENV, 0.7);
  mv->params.ss.ops[1] = operator_init(UGEN_OSC_IMP, OPERATOR_ENV, 0.25);
  operator_set_mult(mv->params.ss.ops[1], 11.0);
  operator_set_vel_s(mv->params.ss.ops[1], 0);

  // variable gain for modulator
  ugen_set_gain(mv->params.ss.ops[1]->ugen, ugen_init_sin(0.5));
  ugen_set_scale(mv->params.ss.ops[1]->ugen->gain, 0.3, 0.8);

  // variable duty cycle for modulator
  ugen_set_duty_cycle(mv->params.ss.ops[1]->ugen, ugen_init_sin(0.1));
  ugen_set_scale(mv->params.ss.ops[1]->ugen->u.impulse.duty_cycle, 0.1, 0.2);

  mv->env = mv->params.ss.ops[0]->env_u.env;
  mv->params.ss.fback_s = 0.0;
}

void
simple_synth_cleanup(MonoVoice mv)
{
  operator_cleanup(mv->params.ss.ops[0]);
  operator_cleanup(mv->params.ss.ops[1]);
}


void
simple_synth_note_on(MonoVoice mv, uint8_t midi_note)
{
  operator_set_velocity(mv->params.ss.ops[0], mv->velocity);
  operator_set_fc(mv->params.ss.ops[0], midi_note_to_freq_table[midi_note]);

  operator_set_fc(mv->params.ss.ops[1], midi_note_to_freq_table[midi_note]);

  if (!mono_voice_playing(mv)) {
    operator_reset(mv->params.ss.ops[0]);
    operator_reset(mv->params.ss.ops[1]);
  }
  mv->sustain = true;
  mv->cur_dur = 0;
}

void
simple_synth_note_off(MonoVoice mv)
{
  env_set_release(mv->params.ss.ops[0]->env_u.env);
  env_set_release(mv->params.ss.ops[1]->env_u.env);

  mv->sustain = false;
}

void
simple_synth_play_sample(MonoVoice mv, FTYPE *L, FTYPE *R)
{
  // sample 2
  FTYPE rv = operator_sample(mv->params.ss.ops[1], mv->sustain);
  // feedback for 2
  operator_set_mod(mv->params.dx7.ops[1], rv * mv->params.ss.fback_s / mv->params.ss.ops[1]->gain_c);
  // prepare 1
  operator_set_mod(mv->params.dx7.ops[0], rv);
  // sample 1
  rv = operator_sample(mv->params.ss.ops[0], mv->sustain);
  *L = rv * (1.0 - mv->params.ss.ops[0]->pan);
  *R = rv * (mv->params.ss.ops[0]->pan);
}

void
simple_synth_play_chunk(MonoVoice mv, FTYPE bufs[3][CHUNK_SIZE])
{
  FTYPE *l_sample = bufs[0];
  FTYPE *r_sample = bufs[1];
  FTYPE *e_sample = bufs[2];

  int i;
  for (i = 0; i < CHUNK_SIZE; i++, l_sample++, r_sample++) {
    simple_synth_play_sample(mv, l_sample, r_sample);
  }
  for (i = 0; i < CHUNK_SIZE; i++, e_sample++) {
    *e_sample = 1.0;
  }

  mv->cur_dur += CHUNK_SIZE;
}
