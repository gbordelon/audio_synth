#include <stdlib.h>
#include <stdio.h>

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../midi/midi.h"
#include "../ugen/operator.h"
#include "../ugen/sin.h"
#include "../ugen/ugen.h"

#include "voice.h"
#include "dx7.h"

void
dx7_init(MonoVoice mv, mono_voice_params params)
{
  mv->env = env_init_default();
  mv->params.dx7.alg = params.dx7.alg;

  mv->params.dx7.ops[0] = operator_init_default();
  mv->params.dx7.ops[0]->mult = params.dx7.patch.mult[0];
  mv->params.dx7.ops[0]->gain_c = params.dx7.patch.gain[0];
  mv->params.dx7.ops[1] = operator_init_default();
  mv->params.dx7.ops[1]->mult = params.dx7.patch.mult[1];
  mv->params.dx7.ops[1]->gain_c = params.dx7.patch.gain[1];
  mv->params.dx7.ops[2] = operator_init_default();
  mv->params.dx7.ops[2]->mult = params.dx7.patch.mult[2];
  mv->params.dx7.ops[2]->gain_c = params.dx7.patch.gain[2];
  mv->params.dx7.ops[3] = operator_init_default();
  mv->params.dx7.ops[3]->mult = params.dx7.patch.mult[3];
  mv->params.dx7.ops[3]->gain_c = params.dx7.patch.gain[3];
  mv->params.dx7.ops[4] = operator_init_default();
  mv->params.dx7.ops[4]->mult = params.dx7.patch.mult[4];
  mv->params.dx7.ops[4]->gain_c = params.dx7.patch.gain[4];
  mv->params.dx7.ops[5] = operator_init_default();
  mv->params.dx7.ops[5]->mult = params.dx7.patch.mult[5];
  mv->params.dx7.ops[5]->gain_c = params.dx7.patch.gain[5];

  mv->params.dx7.patch.detune[0] = params.dx7.patch.detune[0];
  mv->params.dx7.patch.detune[1] = params.dx7.patch.detune[1];
  mv->params.dx7.patch.detune[2] = params.dx7.patch.detune[2];
  mv->params.dx7.patch.detune[3] = params.dx7.patch.detune[3];
  mv->params.dx7.patch.detune[4] = params.dx7.patch.detune[4];
  mv->params.dx7.patch.detune[5] = params.dx7.patch.detune[5];
}

void
dx7_cleanup(MonoVoice mv)
{
  operator_cleanup(mv->params.dx7.ops[0]);
  operator_cleanup(mv->params.dx7.ops[1]);
  operator_cleanup(mv->params.dx7.ops[2]);
  operator_cleanup(mv->params.dx7.ops[3]);
  operator_cleanup(mv->params.dx7.ops[4]);
  operator_cleanup(mv->params.dx7.ops[5]);
  env_cleanup(mv->env);
}


void
dx7_note_on(MonoVoice mv, uint8_t midi_note)
{
  // TODO switch on all 32 algs
  operator_set_gain(mv->params.dx7.ops[0], mv->velocity * 0.5);
//  operator_set_gain(mv->params.dx7.ops[1], mv->velocity);
//  operator_set_gain(mv->params.dx7.ops[2], mv->velocity);
//  operator_set_gain(mv->params.dx7.ops[3], mv->velocity);
//  operator_set_gain(mv->params.dx7.ops[4], mv->velocity);
//  operator_set_gain(mv->params.dx7.ops[5], mv->velocity);

  operator_set_fc(mv->params.dx7.ops[0], midi_note_to_freq_table[midi_note], mv->params.dx7.patch.detune[0]);
  operator_set_fc(mv->params.dx7.ops[1], midi_note_to_freq_table[midi_note], mv->params.dx7.patch.detune[1]); 
  operator_set_fc(mv->params.dx7.ops[2], midi_note_to_freq_table[midi_note], mv->params.dx7.patch.detune[2]); 
  operator_set_fc(mv->params.dx7.ops[3], midi_note_to_freq_table[midi_note], mv->params.dx7.patch.detune[3]); 
  operator_set_fc(mv->params.dx7.ops[4], midi_note_to_freq_table[midi_note], mv->params.dx7.patch.detune[4]); 
  operator_set_fc(mv->params.dx7.ops[5], midi_note_to_freq_table[midi_note], mv->params.dx7.patch.detune[5]); 

  if (!mono_voice_playing(mv)) {
    operator_reset(mv->params.dx7.ops[0]);
    operator_reset(mv->params.dx7.ops[1]);
    operator_reset(mv->params.dx7.ops[2]);
    operator_reset(mv->params.dx7.ops[3]);
    operator_reset(mv->params.dx7.ops[4]);
    operator_reset(mv->params.dx7.ops[5]);
    env_reset(mv->env);
  }
  mv->sustain = true;
  mv->cur_dur = 0;
}

void
dx7_note_off(MonoVoice mv)
{
  mv->sustain = false;
}

/*
 *
Math.exp (cents * Math.log(2) / 1200.0) * (carrier_freq_in_Hz)
 */
FTYPE
dx7_play_sample(MonoVoice mv)
{
  FTYPE rv;
  switch (mv->params.dx7.alg) {
  case DX7_1:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // feedback for 6
    operator_set_fm(mv->params.dx7.ops[5], rv * mv->params.dx7.ops[5]->fc);
    // prepare 5
    operator_set_fm(mv->params.dx7.ops[4], rv * mv->params.dx7.ops[4]->fc);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // prepare 4
    operator_set_fm(mv->params.dx7.ops[3], rv * mv->params.dx7.ops[3]->fc);
    // sample 4
    rv = operator_sample(mv->params.dx7.ops[3], mv->sustain);
    // prepare 3
    operator_set_fm(mv->params.dx7.ops[2], rv * mv->params.dx7.ops[2]->fc);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // prepare 1
    operator_set_fm(mv->params.dx7.ops[0], rv * mv->params.dx7.ops[0]->fc);
    // sample 1
    rv = operator_sample(mv->params.dx7.ops[0], mv->sustain);
    // sample 3
    rv += operator_sample(mv->params.dx7.ops[2], mv->sustain);
    break;
  case DX7_2:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // prepare 5
    operator_set_fm(mv->params.dx7.ops[4], rv * mv->params.dx7.ops[4]->fc);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // prepare 4
    operator_set_fm(mv->params.dx7.ops[3], rv * mv->params.dx7.ops[3]->fc);
    // sample 4
    rv = operator_sample(mv->params.dx7.ops[3], mv->sustain);
    // prepare 3
    operator_set_fm(mv->params.dx7.ops[2], rv * mv->params.dx7.ops[2]->fc);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // feedback for 2
    operator_set_fm(mv->params.dx7.ops[1], rv * mv->params.dx7.ops[1]->fc);
    // prepare 1
    operator_set_fm(mv->params.dx7.ops[0], rv * mv->params.dx7.ops[0]->fc);
    // sample 1
    rv = operator_sample(mv->params.dx7.ops[0], mv->sustain);
    // sample 3
    rv += operator_sample(mv->params.dx7.ops[2], mv->sustain);
    break;
  case DX7_3:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // feedback for 6
    operator_set_fm(mv->params.dx7.ops[5], rv * mv->params.dx7.ops[5]->fc);
    // prepare 5
    operator_set_fm(mv->params.dx7.ops[4], rv * mv->params.dx7.ops[4]->fc);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // prepare 4
    operator_set_fm(mv->params.dx7.ops[3], rv * mv->params.dx7.ops[3]->fc);
    // sample 3
    rv = operator_sample(mv->params.dx7.ops[2], mv->sustain);
    // prepare 2
    operator_set_fm(mv->params.dx7.ops[1], rv * mv->params.dx7.ops[1]->fc);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // prepare 1
    operator_set_fm(mv->params.dx7.ops[0], rv * mv->params.dx7.ops[0]->fc);
    // sample 1
    rv = operator_sample(mv->params.dx7.ops[0], mv->sustain);
    // sample 4
    rv += operator_sample(mv->params.dx7.ops[3], mv->sustain);
    break;
  case DX7_4:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // prepare 5
    operator_set_fm(mv->params.dx7.ops[4], rv * mv->params.dx7.ops[4]->fc);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // prepare 4
    operator_set_fm(mv->params.dx7.ops[3], rv * mv->params.dx7.ops[3]->fc);
    // sample 3
    rv = operator_sample(mv->params.dx7.ops[2], mv->sustain);
    // prepare 2
    operator_set_fm(mv->params.dx7.ops[1], rv * mv->params.dx7.ops[1]->fc);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // prepare 1
    operator_set_fm(mv->params.dx7.ops[0], rv * mv->params.dx7.ops[0]->fc);
    // sample 4
    rv = operator_sample(mv->params.dx7.ops[3], mv->sustain);
    // feedback for 6
    operator_set_fm(mv->params.dx7.ops[5], rv * mv->params.dx7.ops[5]->fc);
    // sample 1
    rv += operator_sample(mv->params.dx7.ops[0], mv->sustain);
    break;
  default:
  // fall through
  case DX7_32:
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    operator_set_fm(mv->params.dx7.ops[5], rv * mv->params.dx7.ops[5]->fc);
    rv += operator_sample(mv->params.dx7.ops[0], mv->sustain);
    rv += operator_sample(mv->params.dx7.ops[1], mv->sustain);
    rv += operator_sample(mv->params.dx7.ops[2], mv->sustain);
    rv += operator_sample(mv->params.dx7.ops[3], mv->sustain);
    rv += operator_sample(mv->params.dx7.ops[4], mv->sustain);
    break;
  }
  return rv;
}

void
dx7_play_chunk(MonoVoice mv, FTYPE bufs[2][CHUNK_SIZE])
{
  FTYPE *t_sample = bufs[0];
  FTYPE *e_sample = bufs[1];

  int i;
  for (i = 0; i < CHUNK_SIZE; i++, t_sample++) {
    *t_sample = dx7_play_sample(mv);
  }
  env_sample_chunk(mv->env, mv->sustain, e_sample);

/*
  // for now overwrite the amplitude envelope
  for (i = 0; i < CHUNK_SIZE; i++, e_sample++) {
    *e_sample = 1.0;
  }
*/
  mv->cur_dur += CHUNK_SIZE;
}
