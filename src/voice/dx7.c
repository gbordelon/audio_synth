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
dx7_e_piano_1(mono_voice_params *params)
{
  params->dx7.alg = DX7_5;
  params->dx7.fback_s = 0.0/7.0; // between 0 and 1

  params->dx7.patch.env_amps[0][0] = 0.0;
  params->dx7.patch.env_amps[0][1] = 1.0;
  params->dx7.patch.env_amps[0][2] = 0.75;
  params->dx7.patch.env_amps[0][3] = 0.1;

  params->dx7.patch.env_amps[1][0] = 0.0;
  params->dx7.patch.env_amps[1][1] = 1.0;
  params->dx7.patch.env_amps[1][2] = 0.75;
  params->dx7.patch.env_amps[1][3] = 0.1;

  params->dx7.patch.env_amps[2][0] = 0.0;
  params->dx7.patch.env_amps[2][1] = 1.0;
  params->dx7.patch.env_amps[2][2] = 0.95;
  params->dx7.patch.env_amps[2][3] = 0.1;

  params->dx7.patch.env_amps[3][0] = 0.0;
  params->dx7.patch.env_amps[3][1] = 1.0;
  params->dx7.patch.env_amps[3][2] = 0.95;
  params->dx7.patch.env_amps[3][3] = 0.1;

  params->dx7.patch.env_amps[4][0] = 0.0;
  params->dx7.patch.env_amps[4][1] = 1.0;
  params->dx7.patch.env_amps[4][2] = 0.95;
  params->dx7.patch.env_amps[4][3] = 0.1;

  params->dx7.patch.env_amps[5][0] = 0.0;
  params->dx7.patch.env_amps[5][1] = 1.0;
  params->dx7.patch.env_amps[5][2] = 0.95;
  params->dx7.patch.env_amps[5][3] = 0.1;


  params->dx7.patch.env_durs[0][0] = 0.1/0.95;
  params->dx7.patch.env_durs[0][1] = 0.1/0.25;
  params->dx7.patch.env_durs[0][2] = 0.1/0.25;
  params->dx7.patch.env_durs[0][3] = 0.1/0.67;

  params->dx7.patch.env_durs[1][0] = 0.1/0.95;
  params->dx7.patch.env_durs[1][1] = 0.1/0.50;
  params->dx7.patch.env_durs[1][2] = 0.1/0.35;
  params->dx7.patch.env_durs[1][3] = 0.1/0.78;

  params->dx7.patch.env_durs[2][0] = 0.1/0.95;
  params->dx7.patch.env_durs[2][1] = 0.1/0.20;
  params->dx7.patch.env_durs[2][2] = 0.1/0.20;
  params->dx7.patch.env_durs[2][3] = 0.1/0.50;

  params->dx7.patch.env_durs[3][0] = 0.1/0.95;
  params->dx7.patch.env_durs[3][1] = 0.1/0.29;
  params->dx7.patch.env_durs[3][2] = 0.1/0.20;
  params->dx7.patch.env_durs[3][3] = 0.1/0.50;

  params->dx7.patch.env_durs[4][0] = 0.1/0.95;
  params->dx7.patch.env_durs[4][1] = 0.1/0.20;
  params->dx7.patch.env_durs[4][2] = 0.1/0.20;
  params->dx7.patch.env_durs[4][3] = 0.1/0.50;

  params->dx7.patch.env_durs[5][0] = 0.1/0.95;
  params->dx7.patch.env_durs[5][1] = 0.1/0.29;
  params->dx7.patch.env_durs[5][2] = 0.1/0.20;
  params->dx7.patch.env_durs[5][3] = 0.1/0.50;


  params->dx7.patch.detune[0] = 0.0;
  params->dx7.patch.detune[1] = 0.0;
  params->dx7.patch.detune[2] = 0.0;
  params->dx7.patch.detune[3] = 0.0;
  params->dx7.patch.detune[4] = 0.0;
  params->dx7.patch.detune[5] = 0.0;

  params->dx7.patch.vel_s[0] = 0.4;
  params->dx7.patch.vel_s[1] = 1.0;
  params->dx7.patch.vel_s[2] = 0.4;
  params->dx7.patch.vel_s[3] = 0.8;
  params->dx7.patch.vel_s[4] = 0.0;
  params->dx7.patch.vel_s[5] = 0.8;

  params->dx7.patch.mult[0] = 1.0;
  params->dx7.patch.mult[1] = 14.0;
  params->dx7.patch.mult[2] = 1.0;
  params->dx7.patch.mult[3] = 1.0;
  params->dx7.patch.mult[4] = 1.0;
  params->dx7.patch.mult[5] = 1.0;

  // set modulators > 1
  // set carriers [0,1]
  params->dx7.patch.gain[0] = 2.0/10.0;
  params->dx7.patch.gain[1] = 1.0; 
  params->dx7.patch.gain[2] = 2.0/10.0;
  params->dx7.patch.gain[3] = 1.5;
  params->dx7.patch.gain[4] = 2.0/10.0;
  params->dx7.patch.gain[5] = 2.0;
}

void
dx7_init(MonoVoice mv, mono_voice_params params)
{
  mv->env = env_init_default();
  mv->params.dx7.alg = params.dx7.alg;
  mv->params.dx7.fback_s = pow(2.0, 7.0 * params.dx7.fback_s - 7.0);

  mv->params.dx7.ops[0] = operator_init_default();
  mv->params.dx7.ops[0]->detune = params.dx7.patch.detune[0];
  mv->params.dx7.ops[0]->gain_c = params.dx7.patch.gain[0];
  mv->params.dx7.ops[0]->mult = params.dx7.patch.mult[0];
  mv->params.dx7.ops[0]->vel_s = params.dx7.patch.vel_s[0];

  env_set_amplitudes(mv->params.dx7.ops[0]->env_u.env, params.dx7.patch.env_amps[0]);
  env_set_duration(mv->params.dx7.ops[0]->env_u.env,
                   params.dx7.patch.env_durs[0][ENV_ATTACK], ENV_ATTACK);
  env_set_duration(mv->params.dx7.ops[0]->env_u.env,
                   params.dx7.patch.env_durs[0][ENV_DECAY], ENV_DECAY);
  env_set_duration(mv->params.dx7.ops[0]->env_u.env,
                   params.dx7.patch.env_durs[0][ENV_SUSTAIN], ENV_SUSTAIN);
  env_set_duration(mv->params.dx7.ops[0]->env_u.env,
                   params.dx7.patch.env_durs[0][ENV_RELEASE], ENV_RELEASE);

  mv->params.dx7.ops[1] = operator_init_default();
  mv->params.dx7.ops[1]->detune = params.dx7.patch.detune[1];
  mv->params.dx7.ops[1]->gain_c = params.dx7.patch.gain[1];
  mv->params.dx7.ops[1]->mult = params.dx7.patch.mult[1];
  mv->params.dx7.ops[1]->vel_s = params.dx7.patch.vel_s[1];

  mv->params.dx7.ops[2] = operator_init_default();
  mv->params.dx7.ops[2]->detune = params.dx7.patch.detune[2];
  mv->params.dx7.ops[2]->gain_c = params.dx7.patch.gain[2];
  mv->params.dx7.ops[2]->mult = params.dx7.patch.mult[2];
  mv->params.dx7.ops[2]->vel_s = params.dx7.patch.vel_s[2];

  mv->params.dx7.ops[3] = operator_init_default();
  mv->params.dx7.ops[3]->detune = params.dx7.patch.detune[3];
  mv->params.dx7.ops[3]->gain_c = params.dx7.patch.gain[3];
  mv->params.dx7.ops[3]->mult = params.dx7.patch.mult[3];
  mv->params.dx7.ops[3]->vel_s = params.dx7.patch.vel_s[3];

  mv->params.dx7.ops[4] = operator_init_default();
  mv->params.dx7.ops[4]->detune = params.dx7.patch.detune[4];
  mv->params.dx7.ops[4]->gain_c = params.dx7.patch.gain[4];
  mv->params.dx7.ops[4]->mult = params.dx7.patch.mult[4];
  mv->params.dx7.ops[4]->vel_s = params.dx7.patch.vel_s[4];

  mv->params.dx7.ops[5] = operator_init_default();
  mv->params.dx7.ops[5]->detune = params.dx7.patch.detune[5];
  mv->params.dx7.ops[5]->gain_c = params.dx7.patch.gain[5];
  mv->params.dx7.ops[5]->mult = params.dx7.patch.mult[5];
  mv->params.dx7.ops[5]->vel_s = params.dx7.patch.vel_s[5];
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
  operator_set_velocity(mv->params.dx7.ops[0], mv->velocity);
  operator_set_velocity(mv->params.dx7.ops[1], mv->velocity);
  operator_set_velocity(mv->params.dx7.ops[2], mv->velocity);
  operator_set_velocity(mv->params.dx7.ops[3], mv->velocity);
  operator_set_velocity(mv->params.dx7.ops[4], mv->velocity);
  operator_set_velocity(mv->params.dx7.ops[5], mv->velocity);

  operator_set_fc(mv->params.dx7.ops[0], midi_note_to_freq_table[midi_note]);
  operator_set_fc(mv->params.dx7.ops[1], midi_note_to_freq_table[midi_note]); 
  operator_set_fc(mv->params.dx7.ops[2], midi_note_to_freq_table[midi_note]); 
  operator_set_fc(mv->params.dx7.ops[3], midi_note_to_freq_table[midi_note]); 
  operator_set_fc(mv->params.dx7.ops[4], midi_note_to_freq_table[midi_note]); 
  operator_set_fc(mv->params.dx7.ops[5], midi_note_to_freq_table[midi_note]); 

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

FTYPE
dx7_play_sample(MonoVoice mv)
{
  FTYPE rv;
  switch (mv->params.dx7.alg) {
  case DX7_1:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->params.dx7.ops[5], rv * mv->params.dx7.fback_s / mv->params.dx7.ops[5]->gain_c);
    // prepare 5
    operator_set_mod(mv->params.dx7.ops[4], rv);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->params.dx7.ops[3], rv);
    // sample 4
    rv = operator_sample(mv->params.dx7.ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->params.dx7.ops[2], rv);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->params.dx7.ops[0], rv);
    // sample 1
    rv = operator_sample(mv->params.dx7.ops[0], mv->sustain);
    // sample 3
    rv += operator_sample(mv->params.dx7.ops[2], mv->sustain);
    break;
  case DX7_2:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->params.dx7.ops[4], rv);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->params.dx7.ops[3], rv);
    // sample 4
    rv = operator_sample(mv->params.dx7.ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->params.dx7.ops[2], rv);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // feedback for 2
    operator_set_mod(mv->params.dx7.ops[1], rv * mv->params.dx7.fback_s / mv->params.dx7.ops[5]->gain_c);
    // prepare 1
    operator_set_mod(mv->params.dx7.ops[0], rv);
    // sample 1
    rv = operator_sample(mv->params.dx7.ops[0], mv->sustain);
    // sample 3
    rv += operator_sample(mv->params.dx7.ops[2], mv->sustain);
    break;
  case DX7_3:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->params.dx7.ops[5], rv * mv->params.dx7.fback_s / mv->params.dx7.ops[5]->gain_c);
    // prepare 5
    operator_set_mod(mv->params.dx7.ops[4], rv);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->params.dx7.ops[3], rv);
    // sample 3
    rv = operator_sample(mv->params.dx7.ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->params.dx7.ops[1], rv);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->params.dx7.ops[0], rv);
    // sample 1
    rv = operator_sample(mv->params.dx7.ops[0], mv->sustain);
    // sample 4
    rv += operator_sample(mv->params.dx7.ops[3], mv->sustain);
    break;
  case DX7_4:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->params.dx7.ops[4], rv);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->params.dx7.ops[3], rv);
    // sample 3
    rv = operator_sample(mv->params.dx7.ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->params.dx7.ops[1], rv);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->params.dx7.ops[0], rv);
    // sample 4
    rv = operator_sample(mv->params.dx7.ops[3], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->params.dx7.ops[5], rv * mv->params.dx7.fback_s / mv->params.dx7.ops[5]->gain_c);
    // sample 1
    rv += operator_sample(mv->params.dx7.ops[0], mv->sustain);
    break;
  case DX7_5:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->params.dx7.ops[5], rv * mv->params.dx7.fback_s / mv->params.dx7.ops[5]->gain_c);
    // prepare 5
    operator_set_mod(mv->params.dx7.ops[4], rv);
    // sample 4
    rv = operator_sample(mv->params.dx7.ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->params.dx7.ops[2], rv);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->params.dx7.ops[0], rv);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // sample 3
    rv += operator_sample(mv->params.dx7.ops[2], mv->sustain);
    // sample 1
    rv += operator_sample(mv->params.dx7.ops[0], mv->sustain);
    break;
  case DX7_6:
    // sample 6
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->params.dx7.ops[4], rv);
    // sample 4
    rv = operator_sample(mv->params.dx7.ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->params.dx7.ops[2], rv);
    // sample 2
    rv = operator_sample(mv->params.dx7.ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->params.dx7.ops[0], rv);
    // sample 5
    rv = operator_sample(mv->params.dx7.ops[4], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->params.dx7.ops[5], rv * mv->params.dx7.fback_s / mv->params.dx7.ops[5]->gain_c);
    // sample 3
    rv += operator_sample(mv->params.dx7.ops[2], mv->sustain);
    // sample 1
    rv += operator_sample(mv->params.dx7.ops[0], mv->sustain);
    break;
  default:
  // fall through
  case DX7_32:
    rv = operator_sample(mv->params.dx7.ops[5], mv->sustain);
    operator_set_mod(mv->params.dx7.ops[5], rv * mv->params.dx7.fback_s / mv->params.dx7.ops[5]->gain_c);
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
