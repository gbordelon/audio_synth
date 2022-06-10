#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "voice.h"
#include "simple_synth.h"
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
    simple_synth_init(mv);
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
    simple_synth_cleanup(mv);
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

  static FTYPE samples[2][CHUNK_SIZE];
  static FTYPE accum[CHUNK_SIZE];
  FTYPE *t, *e, *a;

  memset(accum, 0, CHUNK_SIZE * sizeof(FTYPE));

  // iterate over 64 voices
  MonoVoice mv;
  for (mv = voice->voices; mv - voice->voices < NUM_VOICES; mv++) {
    if (voice_playing(mv)) {
      simple_synth_play_chunk(mv, samples);

      for (a = accum, t = samples[0], e = samples[1]; a - accum < CHUNK_SIZE; t++, e++, a++) {
        *a += *t * *e;
      }
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
      simple_synth_play_config(mv, midi_note);
      break;
    }
  }
}
