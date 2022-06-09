#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "voice.h"
#include "../env/envelope.h"
#include "../lib/macros.h"
#include "../osc/imp.h"
#include "../osc/osc.h"
#include "../midi/midi.h"

Voice
voice_alloc()
{
  return calloc(1, sizeof(struct voice_t));
}

void
voice_free(Voice voice)
{
  free(voice);
}

Voice
voice_init(Channel channels, size_t channel_num)
{
  Voice rv = voice_alloc();
  rv->channels = channels;
  rv->channel_num = channel_num;
  // TODO for channel in channels: increment refcount
  rv->oscillators = osc_alloc_many(2);
  rv->osc_num = 2;
  osc_set(rv->oscillators, OSC_SIN, midi_note_to_freq_table[45]);
  osc_set(rv->oscillators + 1, OSC_IMP, midi_note_to_freq_table[45] * 7.0 / 2.0);
  imp_set_duty_cycle(rv->oscillators + 1, 0.1);

  rv->env = env_init(DEFAULT_SAMPLE_RATE);
  rv->max_dur = 0;
  rv->cur_dur = 0;

  return rv;
}

void
voice_cleanup(Voice voice)
{
  // TODO for channel in channels: reduce refcount
  osc_cleanup(voice->oscillators);
  env_cleanup(voice->env);
  voice_free(voice);
}

/*
 * 1024 samples at 48000/sec means 21.33 milliseconds of play time.
 * ignoring user input during a chunk means a control signal will be 21.33 ms latent
 */
void
voice_play_chunk(Voice voice)
{
  Channel left = voice->channels;
  Channel right = voice->channels + 1;

  static FTYPE t_sample[CHUNK_SIZE];
  static FTYPE e_sample[CHUNK_SIZE];
  FTYPE *t = t_sample;
  FTYPE *e = e_sample;

  Osc car_gen = voice->oscillators;
  Osc mod_gen = voice->oscillators + 1;

  int i;
  for (i = 0; i < CHUNK_SIZE; i++, t++, e++) {
    *t = osc_sample_phase_osc(car_gen, mod_gen);
    *e = env_sample(voice->env);
  }

  // TODO check rv
  channel_write(left, t_sample);
  channel_write(right, t_sample);

  voice->cur_dur += CHUNK_SIZE;
}

// TODO support other voices
// currently assumes two oscillators in a phase modulation setup
void
voice_play_config(Voice voice, uint8_t midi_note, FTYPE dur /* in seconds */)
{
  voice->cur_dur = 0;
  voice->max_dur = DEFAULT_SAMPLE_RATE * dur;

  env_reset(voice->env);
  env_set_duration(voice->env, voice->max_dur);

  // for osc in oscillators
  //    set freq
  //    reset phase
  osc_set_freq(voice->oscillators, midi_note_to_freq_table[midi_note]);
  osc_set_freq(voice->oscillators + 1, midi_note_to_freq_table[midi_note] * 7.0 / 2.0);
  osc_reset_phase(voice->oscillators);
  osc_reset_phase(voice->oscillators + 1);
}
