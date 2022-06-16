#ifndef SIMPLE_SYNTH_H
#define SIMPLE_SYNTH_H

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../midi/midi.h"
#include "../ugen/imp.h"
#include "../ugen/osc.h"

#include "voice.h"

void
simple_synth_init(MonoVoice mv)
{
  mv->oscillators = calloc(2, sizeof(Osc*));
  *mv->oscillators = osc_init(UGEN_CONSTANT, UGEN_OSC_SIN, 1.0, midi_note_to_freq_table[45]);
  *(mv->oscillators + 1) = osc_init(UGEN_CONSTANT, UGEN_OSC_IMP, 1.0, midi_note_to_freq_table[45] * 7.0);
  mv->osc_num = 2;
  osc_set_velocity_gain(*mv->oscillators, 0.7);
  osc_set_velocity_gain(*(mv->oscillators + 1), 1.0);
  imp_set_duty_cycle(*(mv->oscillators + 1), 0.1);

  ugen_set_mod((*mv->oscillators)->frequency,(*(mv->oscillators + 1))->frequency);

  mv->env = env_init(DEFAULT_SAMPLE_RATE);
}

void
simple_synth_cleanup(MonoVoice mv)
{
  osc_cleanup(*mv->oscillators);
  osc_cleanup(*(mv->oscillators + 1));
  env_cleanup(mv->env);
}

void
simple_synth_note_on(MonoVoice mv, uint8_t midi_note)
{
  env_set_duration(mv->env, mv->max_dur);

  osc_set_tone_freq(*mv->oscillators, midi_note_to_freq_table[midi_note]);
  osc_set_velocity_gain(*mv->oscillators, 0.7);
  osc_set_tone_freq(*(mv->oscillators + 1), midi_note_to_freq_table[midi_note] * 7.0);// * 7.0 / 2.0);
  osc_set_velocity_gain(*(mv->oscillators + 1), mv->velocity);


  // only reset phase if the note is not currently playing otherwise might hear blips
  if (!voice_playing(mv)) {
    mv->sustain = true;
    osc_reset_phase(*mv->oscillators);
    osc_reset_phase(*(mv->oscillators + 1));
  }
}

void
simple_synth_note_off(MonoVoice mv)
{
  mv->sustain = false;
}

void
simple_synth_play_chunk(MonoVoice mv, FTYPE bufs[2][CHUNK_SIZE])
{
  FTYPE *t_sample = *bufs;
  FTYPE *e_sample = *(bufs + 1);

  osc_chunk_sample(*mv->oscillators, t_sample);
  env_sample_chunk(mv->env, mv->sustain, e_sample);

  mv->cur_dur += CHUNK_SIZE;
}

#endif
