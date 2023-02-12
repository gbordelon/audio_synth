#include <math.h>
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

/*
 * 45 tunable parameters
 */
void
dx7_e_piano_1(mono_voice_params *params)
{
  params->dx7.alg = DX7_5; // [0, 31]
  params->dx7.fback_s = 1.0/7.0; // [0, 1]
  params->dx7.decay_rate = 0.99; // [0, 1]

  // [0,1]
  params->dx7.patch.env_amps[0][0] = 0.0;
  params->dx7.patch.env_amps[0][1] = 1.0;
  params->dx7.patch.env_amps[0][2] = 0.55;
  params->dx7.patch.env_amps[0][3] = 0.1;

  params->dx7.patch.env_amps[1][0] = 0.0;
  params->dx7.patch.env_amps[1][1] = 1.0;
  params->dx7.patch.env_amps[1][2] = 0.75;
  params->dx7.patch.env_amps[1][3] = 0.1;

  params->dx7.patch.env_amps[2][0] = 0.0;
  params->dx7.patch.env_amps[2][1] = 1.0;
  params->dx7.patch.env_amps[2][2] = 0.75;
  params->dx7.patch.env_amps[2][3] = 0.1;

  params->dx7.patch.env_amps[3][0] = 0.0;
  params->dx7.patch.env_amps[3][1] = 1.0;
  params->dx7.patch.env_amps[3][2] = 0.95;
  params->dx7.patch.env_amps[3][3] = 0.1;

  params->dx7.patch.env_amps[4][0] = 0.0;
  params->dx7.patch.env_amps[4][1] = 1.0;
  params->dx7.patch.env_amps[4][2] = 0.75;
  params->dx7.patch.env_amps[4][3] = 0.1;

  params->dx7.patch.env_amps[5][0] = 0.0;
  params->dx7.patch.env_amps[5][1] = 1.0;
  params->dx7.patch.env_amps[5][2] = 0.95;
  params->dx7.patch.env_amps[5][3] = 0.1;

  // (0, 10]
  params->dx7.patch.env_rates[0][0] = 9.5;
  params->dx7.patch.env_rates[0][1] = 5.0;
  params->dx7.patch.env_rates[0][2] = 0.5;
  params->dx7.patch.env_rates[0][3] = 6.7;

  params->dx7.patch.env_rates[1][0] = 9.5;
  params->dx7.patch.env_rates[1][1] = 10.0;
  params->dx7.patch.env_rates[1][2] = 0.7;
  params->dx7.patch.env_rates[1][3] = 7.8;

  params->dx7.patch.env_rates[2][0] = 9.5;
  params->dx7.patch.env_rates[2][1] = 4.0;
  params->dx7.patch.env_rates[2][2] = 0.4;
  params->dx7.patch.env_rates[2][3] = 5.0;

  params->dx7.patch.env_rates[3][0] = 9.5;
  params->dx7.patch.env_rates[3][1] = 5.8;
  params->dx7.patch.env_rates[3][2] = 0.4;
  params->dx7.patch.env_rates[3][3] = 5.0;

  params->dx7.patch.env_rates[4][0] = 9.5;
  params->dx7.patch.env_rates[4][1] = 4.0;
  params->dx7.patch.env_rates[4][2] = 0.4;
  params->dx7.patch.env_rates[4][3] = 5.0;

  params->dx7.patch.env_rates[5][0] = 9.5;
  params->dx7.patch.env_rates[5][1] = 5.8;
  params->dx7.patch.env_rates[5][2] = 0.4;
  params->dx7.patch.env_rates[5][3] = 5.0;

  // [0, 1200.0]
  params->dx7.patch.detune[0] = 0.0;
  params->dx7.patch.detune[1] = 0.0;
  params->dx7.patch.detune[2] = 0.0;
  params->dx7.patch.detune[3] = 0.0;
  params->dx7.patch.detune[4] = 0.0;
  params->dx7.patch.detune[5] = 0.0;

  // [0, 10]
  params->dx7.patch.vel_s[0] = 0.4;
  params->dx7.patch.vel_s[1] = 1.0;
  params->dx7.patch.vel_s[2] = 0.4;
  params->dx7.patch.vel_s[3] = 0.8;
  params->dx7.patch.vel_s[4] = 0.0;
  params->dx7.patch.vel_s[5] = 0.8;

  // [0, 100]
  params->dx7.patch.mult[0] = 1.0;
  params->dx7.patch.mult[1] = 14.0;
  params->dx7.patch.mult[2] = 1.0;
  params->dx7.patch.mult[3] = 1.0;
  params->dx7.patch.mult[4] = 1.0;
  params->dx7.patch.mult[5] = 1.0;

  // [0, 1]
  params->dx7.patch.pan[0] = 0.5;
  params->dx7.patch.pan[1] = 0.5;
  params->dx7.patch.pan[2] = 0.25;
  params->dx7.patch.pan[3] = 0.5;
  params->dx7.patch.pan[4] = 0.75;
  params->dx7.patch.pan[5] = 0.5;

  // set carriers [0,1]
  params->dx7.patch.gain[0] = 1.0/8.0;
  params->dx7.patch.gain[1] = 0.60; 
  params->dx7.patch.gain[2] = 1.0/8.0;
  params->dx7.patch.gain[3] = 0.90;
  params->dx7.patch.gain[4] = 1.0/8.0;
  params->dx7.patch.gain[5] = 0.80;
}

void
dx7_init(MonoVoice mv, mono_voice_params params)
{
  mv->op_num = 6;
  mv->ops = calloc(mv->op_num, sizeof(struct operator_t *));

  int i;
  for (i = 0; i < mv->op_num; i++) {
    mv->ops[i] = operator_init_default();
    mv->ops[i]->detune = params.dx7.patch.detune[i];
    mv->ops[i]->gain_c = params.dx7.patch.gain[i];
    mv->ops[i]->mult = params.dx7.patch.mult[i];
    mv->ops[i]->vel_s = params.dx7.patch.vel_s[i];
    mv->ops[i]->pan = params.dx7.patch.pan[i];

    mv->ops[i]->env_u.env->decay_rate = params.dx7.decay_rate;
    env_set_amplitudes(mv->ops[i]->env_u.env, params.dx7.patch.env_amps[i]);
    env_set_rate(mv->ops[i]->env_u.env,
                     params.dx7.patch.env_rates[i][ENV_ATTACK], ENV_ATTACK);
    env_set_rate(mv->ops[i]->env_u.env,
                     params.dx7.patch.env_rates[i][ENV_DECAY], ENV_DECAY);
    env_set_rate(mv->ops[i]->env_u.env,
                     params.dx7.patch.env_rates[i][ENV_SUSTAIN], ENV_SUSTAIN);
    env_set_rate(mv->ops[i]->env_u.env,
                     params.dx7.patch.env_rates[i][ENV_RELEASE], ENV_RELEASE);
    mv->ops[i]->env_u.env->p_ind = env_max_duration(mv->ops[i]->env_u.env);
  }

  mv->params.dx7.alg = params.dx7.alg;
  mv->params.dx7.fback_s = pow(2.0, 7.0 * params.dx7.fback_s - 7.0);
}

void
dx7_cleanup(MonoVoice mv)
{
  int i;
  for (i = 0; i < mv->op_num; i++) {
    operator_cleanup(mv->ops[i]);
  }
  free(mv->ops);
}

void
dx7_note_on(MonoVoice mv, uint8_t midi_note, FTYPE velocity)
{
  int i;
  for (i = 0; i < mv->op_num; i++) {
    operator_set_velocity(mv->ops[i], velocity);
    operator_set_fc(mv->ops[i], midi_note_to_freq_table[midi_note]);

    operator_reset(mv->ops[i]);
  }
  mv->sustain = true;
  mv->cur_dur = 0;
}

void
dx7_note_off(MonoVoice mv)
{
  int i;
  for (i = 0; i < mv->op_num; i++) {
    operator_release(mv->ops[i]);
  }
  mv->sustain = false;
}

void
dx7_play_sample(MonoVoice mv, FTYPE *L, FTYPE *R)
{
  FTYPE rv;
  switch (mv->params.dx7.alg) {
  case DX7_1:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    break;
  case DX7_2:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // feedback for 2
    operator_set_mod(mv->ops[1], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    break;
  case DX7_3:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L += rv * (1.0 - mv->ops[3]->pan);
    *R += rv * (mv->ops[3]->pan);
    break;
  case DX7_4:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    *L = rv * (1.0 - mv->ops[3]->pan);
    *R = rv * (mv->ops[3]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L += rv * (1.0 - mv->ops[0]->pan);
    *R += rv * (mv->ops[0]->pan);
    break;
  case DX7_5:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L = rv * (1.0 - mv->ops[4]->pan);
    *R = rv * (mv->ops[4]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L += rv * (1.0 - mv->ops[0]->pan);
    *R += rv * (mv->ops[0]->pan);
    break;
  case DX7_6:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    *L = rv * (1.0 - mv->ops[4]->pan);
    *R = rv * (mv->ops[4]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L += rv * (1.0 - mv->ops[0]->pan);
    *R += rv * (mv->ops[0]->pan);
    break;
  case DX7_7:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L += rv * (1.0 - mv->ops[0]->pan);
    *R += rv * (mv->ops[0]->pan);
    break;
  case DX7_8:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // sample 4
    rv += operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // feedback for 4
    operator_set_mod(mv->ops[3], rv * mv->params.dx7.fback_s / mv->ops[3]->gain_c);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L += rv * (1.0 - mv->ops[0]->pan);
    *R += rv * (mv->ops[0]->pan);
    break;
  case DX7_9:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // sample 4
    rv += operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // feedback for 2
    operator_set_mod(mv->ops[1], rv * mv->params.dx7.fback_s / mv->ops[1]->gain_c);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L += rv * (1.0 - mv->ops[0]->pan);
    *R += rv * (mv->ops[0]->pan);
    break;
  case DX7_10:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // feedback for 3
    operator_set_mod(mv->ops[2], rv * mv->params.dx7.fback_s / mv->ops[2]->gain_c);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L += rv * (1.0 - mv->ops[3]->pan);
    *R += rv * (mv->ops[3]->pan);
    break;
  case DX7_11:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L += rv * (1.0 - mv->ops[3]->pan);
    *R += rv * (mv->ops[3]->pan);
    break;
  case DX7_12:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // sample 4
    rv += operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // feedback for 2
    operator_set_mod(mv->ops[1], rv * mv->params.dx7.fback_s / mv->ops[1]->gain_c);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    break;
  case DX7_13:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // sample 4
    rv += operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    break;
  case DX7_14:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    break;
  case DX7_15:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // feedback for 2
    operator_set_mod(mv->ops[1], rv * mv->params.dx7.fback_s / mv->ops[1]->gain_c);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    break;
  case DX7_16:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // sample 3
    rv += operator_sample(mv->ops[2], mv->sustain);
    // sample 2
    rv += operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_17:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // feedback for 2
    operator_set_mod(mv->ops[1], rv * mv->params.dx7.fback_s / mv->ops[1]->gain_c);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // sample 3
    rv += operator_sample(mv->ops[2], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_18:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // feedback for 3
    operator_set_mod(mv->ops[2], rv * mv->params.dx7.fback_s / mv->ops[2]->gain_c);
    // sample 4
    rv += operator_sample(mv->ops[3], mv->sustain);
    // sample 2
    rv += operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_19:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L = rv * (1.0 - mv->ops[4]->pan);
    *R = rv * (mv->ops[4]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L += rv * (1.0 - mv->ops[3]->pan);
    *R += rv * (mv->ops[3]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L += rv * (1.0 - mv->ops[0]->pan);
    *R += rv * (mv->ops[0]->pan);
    break;
  case DX7_20:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // feedback for 3
    operator_set_mod(mv->ops[2], rv * mv->params.dx7.fback_s / mv->ops[2]->gain_c);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L = rv * (1.0 - mv->ops[3]->pan);
    *R = rv * (mv->ops[3]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L += rv * (1.0 - mv->ops[1]->pan);
    *R += rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_21:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // feedback for 3
    operator_set_mod(mv->ops[2], rv * mv->params.dx7.fback_s / mv->ops[2]->gain_c);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L = rv * (1.0 - mv->ops[4]->pan);
    *R = rv * (mv->ops[4]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L = rv * (1.0 - mv->ops[3]->pan);
    *R = rv * (mv->ops[3]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L += rv * (1.0 - mv->ops[1]->pan);
    *R += rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_22:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L += rv * (1.0 - mv->ops[4]->pan);
    *R += rv * (mv->ops[4]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L += rv * (1.0 - mv->ops[3]->pan);
    *R += rv * (mv->ops[3]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_23:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L = rv * (1.0 - mv->ops[4]->pan);
    *R = rv * (mv->ops[4]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L = rv * (1.0 - mv->ops[3]->pan);
    *R = rv * (mv->ops[3]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L += rv * (1.0 - mv->ops[1]->pan);
    *R += rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_24:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L += rv * (1.0 - mv->ops[4]->pan);
    *R += rv * (mv->ops[4]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L += rv * (1.0 - mv->ops[3]->pan);
    *R += rv * (mv->ops[3]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L = rv * (1.0 - mv->ops[1]->pan);
    *R = rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_25:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L = rv * (1.0 - mv->ops[4]->pan);
    *R = rv * (mv->ops[4]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L = rv * (1.0 - mv->ops[3]->pan);
    *R = rv * (mv->ops[3]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L = rv * (1.0 - mv->ops[2]->pan);
    *R = rv * (mv->ops[2]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L += rv * (1.0 - mv->ops[1]->pan);
    *R += rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_26:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L += rv * (1.0 - mv->ops[3]->pan);
    *R += rv * (mv->ops[3]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L = rv * (1.0 - mv->ops[1]->pan);
    *R = rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_27:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // sample 5
    rv += operator_sample(mv->ops[4], mv->sustain);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    // prepare 2
    operator_set_mod(mv->ops[1], rv);
    // feedback for 3
    operator_set_mod(mv->ops[2], rv * mv->params.dx7.fback_s / mv->ops[2]->gain_c);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L = rv * (1.0 - mv->ops[3]->pan);
    *R = rv * (mv->ops[3]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L += rv * (1.0 - mv->ops[1]->pan);
    *R += rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_28:
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // feedback for 5
    operator_set_mod(mv->ops[4], rv * mv->params.dx7.fback_s / mv->ops[4]->gain_c);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    // prepare 1
    operator_set_mod(mv->ops[0], rv);
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    *L += rv * (1.0 - mv->ops[5]->pan);
    *R += rv * (mv->ops[5]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_29:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L = rv * (1.0 - mv->ops[4]->pan);
    *R = rv * (mv->ops[4]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L += rv * (1.0 - mv->ops[1]->pan);
    *R += rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L += rv * (1.0 - mv->ops[0]->pan);
    *R += rv * (mv->ops[0]->pan);
    break;
  case DX7_30:
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    // feedback for 5
    operator_set_mod(mv->ops[4], rv * mv->params.dx7.fback_s / mv->ops[4]->gain_c);
    // prepare 4
    operator_set_mod(mv->ops[3], rv);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    // prepare 3
    operator_set_mod(mv->ops[2], rv);
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    *L += rv * (1.0 - mv->ops[5]->pan);
    *R += rv * (mv->ops[5]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L += rv * (1.0 - mv->ops[2]->pan);
    *R += rv * (mv->ops[2]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L = rv * (1.0 - mv->ops[1]->pan);
    *R = rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  case DX7_31:
    // sample 6
    rv = operator_sample(mv->ops[5], mv->sustain);
    // prepare 5
    operator_set_mod(mv->ops[4], rv);
    // feedback for 6
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    // sample 5
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L = rv * (1.0 - mv->ops[4]->pan);
    *R = rv * (mv->ops[4]->pan);
    // sample 4
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L = rv * (1.0 - mv->ops[3]->pan);
    *R = rv * (mv->ops[3]->pan);
    // sample 3
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L = rv * (1.0 - mv->ops[2]->pan);
    *R = rv * (mv->ops[2]->pan);
    // sample 2
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L = rv * (1.0 - mv->ops[1]->pan);
    *R = rv * (mv->ops[1]->pan);
    // sample 1
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  default:
  // fall through
  case DX7_32:
    rv = operator_sample(mv->ops[5], mv->sustain);
    operator_set_mod(mv->ops[5], rv * mv->params.dx7.fback_s / mv->ops[5]->gain_c);
    *L = rv * (1.0 - mv->ops[5]->pan);
    *R = rv * (mv->ops[5]->pan);
    rv = operator_sample(mv->ops[4], mv->sustain);
    *L = rv * (1.0 - mv->ops[4]->pan);
    *R = rv * (mv->ops[4]->pan);
    rv = operator_sample(mv->ops[3], mv->sustain);
    *L = rv * (1.0 - mv->ops[3]->pan);
    *R = rv * (mv->ops[3]->pan);
    rv = operator_sample(mv->ops[2], mv->sustain);
    *L = rv * (1.0 - mv->ops[2]->pan);
    *R = rv * (mv->ops[2]->pan);
    rv = operator_sample(mv->ops[1], mv->sustain);
    *L = rv * (1.0 - mv->ops[1]->pan);
    *R = rv * (mv->ops[1]->pan);
    rv = operator_sample(mv->ops[0], mv->sustain);
    *L = rv * (1.0 - mv->ops[0]->pan);
    *R = rv * (mv->ops[0]->pan);
    break;
  }
}

void
dx7_play_chunk(MonoVoice mv, FTYPE bufs[2][CHUNK_SIZE])
{
  FTYPE *l_sample = bufs[0];
  FTYPE *r_sample = bufs[1];

  int i;
  for (i = 0; i < CHUNK_SIZE; i++, l_sample++, r_sample++) {
    dx7_play_sample(mv, l_sample, r_sample);
  }
  mv->cur_dur += CHUNK_SIZE;
}
