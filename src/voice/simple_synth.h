#ifndef SIMPLE_SYNTH_H
#define SIMPLE_SYNTH_H

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../midi/midi.h"
#include "../osc/imp.h"
#include "../osc/osc.h"

#include "voice.h"

void
simple_synth_init(MonoVoice mv)
{
  mv->oscillators = osc_alloc_many(2);
  mv->osc_num = 2;
  osc_set(mv->oscillators, OSC_SIN, midi_note_to_freq_table[45]);
  osc_set(mv->oscillators + 1, OSC_IMP, midi_note_to_freq_table[45] * 7.0 / 2.0);
  imp_set_duty_cycle(mv->oscillators + 1, 0.1);

  mv->env = env_init(DEFAULT_SAMPLE_RATE);
}

void
simple_synth_cleanup(MonoVoice mv)
{
  osc_cleanup(mv->oscillators);
  env_cleanup(mv->env);
}

void
simple_synth_play_config(MonoVoice mv, uint8_t midi_note)
{
  env_set_duration(mv->env, mv->max_dur);

  // for osc in oscillators
  //    set freq
  //    reset phase
  osc_set_freq(mv->oscillators, midi_note_to_freq_table[midi_note]);
  osc_set_freq(mv->oscillators + 1, midi_note_to_freq_table[midi_note] * 7.0 / 2.0);
  osc_reset_phase(mv->oscillators);
  osc_reset_phase(mv->oscillators + 1);
}

void
simple_synth_play_chunk(MonoVoice mv, FTYPE bufs[2][CHUNK_SIZE])
{
  FTYPE *t_sample = *bufs;
  FTYPE *e_sample = *(bufs + 1);

  osc_sample_chunk(mv->oscillators, mv->oscillators + 1, t_sample);
  env_sample_chunk(mv->env, e_sample);

  mv->cur_dur += CHUNK_SIZE;
}
#endif
