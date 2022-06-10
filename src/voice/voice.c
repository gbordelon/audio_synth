#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "voice.h"
#include "../env/envelope.h"
#include "../lib/macros.h"
#include "../osc/imp.h"
#include "../osc/osc.h"
#include "../midi/midi.h"

MonoVoice
mono_voice_alloc()
{
  return calloc(NUM_VOICES, sizeof(struct mono_voice_t));
}

void
mono_voice_free(MonoVoice voice)
{
  free(voice);
}

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
  rv->voices = mono_voice_alloc();
  rv->voice_num = NUM_VOICES;

  MonoVoice mv;
  for (mv = rv->voices; mv - rv->voices < NUM_VOICES; mv++) {
    mv->oscillators = osc_alloc_many(2);
    mv->osc_num = 2;
    osc_set(mv->oscillators, OSC_SIN, midi_note_to_freq_table[45]);
    osc_set(mv->oscillators + 1, OSC_IMP, midi_note_to_freq_table[45] * 7.0 / 2.0);
    imp_set_duty_cycle(mv->oscillators + 1, 0.1);

    mv->env = env_init(DEFAULT_SAMPLE_RATE);
    mv->max_dur = 0;
    mv->cur_dur = 0;
  }

  return rv;
}

void
voice_cleanup(Voice voice)
{
  // TODO for channel in channels: reduce refcount
  MonoVoice mv;
  for (mv = voice->voices; mv - voice->voices < NUM_VOICES; mv++) {
    osc_cleanup(mv->oscillators);
    env_cleanup(mv->env);
  }
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
  static FTYPE accum[CHUNK_SIZE];
  FTYPE *t, *e, *a;

  memset(accum, 0, CHUNK_SIZE * sizeof(FTYPE));

  // iterate over 64 voices
  MonoVoice mv;
  for (mv = voice->voices; mv - voice->voices < NUM_VOICES; mv++) {
    if (voice_playing(mv)) {
      osc_sample_chunk(mv->oscillators, mv->oscillators + 1, t_sample);
      env_sample_chunk(mv->env, e_sample);

      for (a = accum, t = t_sample, e = e_sample; t - t_sample < CHUNK_SIZE; t++, e++, a++) {
        *a += *t * *e;
      }
      mv->cur_dur += CHUNK_SIZE;
    }
  }

  // TODO check rv
  channel_write(left, accum);
  channel_write(right, accum);
}

// TODO currently assumes two oscillators in a phase modulation setup
void
voice_play_config(Voice voice, uint8_t midi_note, FTYPE dur /* in seconds */)
{
  MonoVoice mv;
  for (mv = voice->voices; mv - voice->voices < NUM_VOICES; mv++) {
    if (!voice_playing(mv)) {
      mv->cur_dur = 0;
      mv->max_dur = DEFAULT_SAMPLE_RATE * dur;

      env_reset(mv->env);
      env_set_duration(mv->env, mv->max_dur);

      // for osc in oscillators
      //    set freq
      //    reset phase
      osc_set_freq(mv->oscillators, midi_note_to_freq_table[midi_note]);
      osc_set_freq(mv->oscillators + 1, midi_note_to_freq_table[midi_note] * 7.0 / 2.0);
      osc_reset_phase(mv->oscillators);
      osc_reset_phase(mv->oscillators + 1);
      break;
    }
  }
}
