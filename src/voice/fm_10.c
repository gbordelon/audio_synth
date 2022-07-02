#include <stdlib.h>
#include <stdio.h>

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../midi/midi.h"
#include "../ugen/sin.h"
#include "../ugen/ugen.h"

#include "voice.h"
#include "fm_10.h"

void
fm_10_init(MonoVoice mv, mono_voice_params params)
{
  mv->ugen_num = 2;
  mv->ugens = calloc(mv->ugen_num, sizeof(Ugen*));
  mv->ugens[0] = ugen_init_sin(midi_note_to_freq_table[45]); // sine carrier
  mv->ugens[1] = ugen_init_sin(midi_note_to_freq_table[45]); // sine modulator

  ugen_set_gain_c(mv->ugens[0], 1.0); // set by velocity in note_on

  mv->env = env_init_default();
  FTYPE amps[4] = {0.0, 1.0, 0.75, 0.65}; // brass
  //FTYPE amps[4] = {0.0, 1.0, 1.0, 1.0}; // woodwind
  env_set_amplitudes(mv->env, amps);
  env_set_duration(mv->env, 0.1, ENV_ATTACK);
  env_set_duration(mv->env, 0.1, ENV_DECAY);
  env_set_duration(mv->env, 0.3, ENV_SUSTAIN);
  env_set_duration(mv->env, 0.1, ENV_RELEASE);
  mv->params.fm_10.p6 = params.fm_10.p6;
  mv->params.fm_10.p7 = params.fm_10.p7;
  mv->params.fm_10.p8 = params.fm_10.p8;

  mv->params.fm_10.e = env_init_default();
  env_set_amplitudes(mv->params.fm_10.e, amps);
  env_set_duration(mv->params.fm_10.e, 0.1, ENV_ATTACK);
  env_set_duration(mv->params.fm_10.e, 0.1, ENV_DECAY);
  env_set_duration(mv->params.fm_10.e, 0.3, ENV_SUSTAIN);
  env_set_duration(mv->params.fm_10.e, 0.1, ENV_RELEASE);
}

void
fm_10_cleanup(MonoVoice mv)
{
  ugen_cleanup(mv->ugens[0]);
  ugen_cleanup(mv->ugens[1]);
  env_cleanup(mv->env);
  env_cleanup(mv->params.fm_10.e);
}


void
fm_10_note_on(MonoVoice mv, uint8_t midi_note)
{
  ugen_set_gain_c(mv->ugens[0], mv->velocity * 0.5);
  ugen_set_freq(mv->ugens[1], midi_note_to_freq_table[midi_note] * mv->params.fm_10.p6);
  mv->params.fm_10.carrier = midi_note_to_freq_table[midi_note];

  // only reset phase if the note is not currently playing otherwise might hear blips
  if (!mono_voice_playing(mv)) {
    env_reset(mv->env);
    env_reset(mv->params.fm_10.e);
    ugen_reset_phase(mv->ugens[0]);
    ugen_reset_phase(mv->ugens[1]);
  }
  mv->sustain = true;
  mv->cur_dur = 0;
}

void
fm_10_note_off(MonoVoice mv)
{
  mv->sustain = false;
}

void
fm_10_play_chunk(MonoVoice mv, FTYPE bufs[3][CHUNK_SIZE])
{
  FTYPE *l_sample = bufs[0];
  FTYPE *r_sample = bufs[1];
  FTYPE *e_sample = bufs[2];

  FTYPE mod_sample = 0, mod_env = 0, p6;
  p6 = mv->params.fm_10.p6 * mv->params.fm_10.carrier;
  int i;
  for (i = 0; i < CHUNK_SIZE; i++, l_sample++, r_sample++) {
    // sample ug1
    mod_sample = ugen_sample(mv->ugens[1]);
    // sample ug5
    mod_env = env_sample(mv->params.fm_10.e, mv->sustain);
    // scale ug5 by DEV2
    mod_env *= p6 * (mv->params.fm_10.p8 - mv->params.fm_10.p7);
    // mod_env will hold the output of ug6
    mod_env += p6 * mv->params.fm_10.p7;

    // scale ug1 by ug6 and sum with p5 to get ug2
    // feed ug2 into ug3
    FTYPE freq = mod_env * mod_sample + mv->params.fm_10.carrier;
    ugen_set_freq(mv->ugens[0], freq);
    // sample ug3
    *l_sample = *r_sample = ugen_sample(mv->ugens[0]);
  }
  env_sample_chunk(mv->env, mv->sustain, e_sample);

  mv->cur_dur += CHUNK_SIZE;
}
