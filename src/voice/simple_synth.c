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
  mv->ugen_num = 4;
  mv->ugens = calloc(mv->ugen_num, sizeof(Ugen*));
  mv->ugens[0] = ugen_init_sin(midi_note_to_freq_table[45]); // sin carrier
  mv->ugens[1] = ugen_init_imp(midi_note_to_freq_table[45] * 7.0, 0.1); // impulse 10% duty cycle modulator
  mv->ugens[2] = ugen_init_sin(0.5); // LFO for modulator gain
  mv->ugens[3] = ugen_init_sin(0.1); // LFO for modulator duty_cycle

  ugen_set_gain_c(mv->ugens[0], 0.7);

  ugen_set_mod(mv->ugens[0], mv->ugens[1]);
  ugen_set_gain(mv->ugens[1], mv->ugens[2]);
  ugen_set_duty_cycle(mv->ugens[1], mv->ugens[3]);

  ugen_set_scale(mv->ugens[2], 0.3, 0.8);
  ugen_set_scale(mv->ugens [3], 0.1, 0.2);

  mv->env = env_init_default();
}

void
simple_synth_cleanup(MonoVoice mv)
{
  ugen_cleanup(*mv->ugens);
  ugen_cleanup(*(mv->ugens + 1));
  ugen_cleanup(*(mv->ugens + 2));
  ugen_cleanup(*(mv->ugens + 3));
  env_cleanup(mv->env);
}


void
simple_synth_note_on(MonoVoice mv, uint8_t midi_note)
{
  FTYPE mod = round(mv->velocity * 11.0);
  ugen_set_freq(*mv->ugens, midi_note_to_freq_table[midi_note]);
  ugen_set_gain_c(*mv->ugens, mv->velocity);
  ugen_set_freq(*(mv->ugens + 1), midi_note_to_freq_table[midi_note] * mod);
  //ugen_set_gain_c(*(mv->ugens + 1), 1.0);//mv->velocity);
  //ugen_set_duty_cycle_c(*(mv->ugens + 2), 1.0 - mv->velocity);

  // only reset phase if the note is not currently playing otherwise might hear blips
  if (!mono_voice_playing(mv)) {
    env_reset(mv->env);
    ugen_reset_phase(*mv->ugens);
    ugen_reset_phase(*(mv->ugens + 1));
    //ugen_reset_phase(*(mv->ugens + 2)); // do i want to reset the phase of the LFO?
    //ugen_reset_phase(*(mv->ugens + 3)); // do i want to reset the phase of the LFO?
  }
  mv->sustain = true;
}

void
simple_synth_note_off(MonoVoice mv)
{
  mv->sustain = false;
}

void
simple_synth_play_chunk(MonoVoice mv, FTYPE bufs[3][CHUNK_SIZE])
{
  FTYPE *l_sample = bufs[0];
  FTYPE *r_sample = bufs[1];
  FTYPE *e_sample = bufs[2];

  ugen_chunk_sample(*mv->ugens, l_sample);
  memmove(r_sample, l_sample, CHUNK_SIZE * sizeof(FTYPE));
  env_sample_chunk(mv->env, mv->sustain, e_sample);

  mv->cur_dur += CHUNK_SIZE;
}
