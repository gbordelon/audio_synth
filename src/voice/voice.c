#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"
#include "../midi/midi.h"

#include "voice.h"

// TODO create more instruments
#include "simple_synth.h"

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
  rv->env_proto = env_init(DEFAULT_SAMPLE_RATE);

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
  env_cleanup(voice->env_proto);
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
  //size_t rv = 0;
  //rv = channel_write(left, accum);
  //rv = channel_write(right, accum);
  channel_write(left, accum);
  channel_write(right, accum);
}

uint8_t
voice_note_on(Voice voice, uint8_t midi_note, uint8_t midi_velocity)
{
  MonoVoice mv;
  for (mv = voice->voices; mv - voice->voices < NUM_VOICES; mv++) {
    if (!voice_playing(mv)) {
      mv->cur_dur = 0;
      mv->max_dur = DEFAULT_SAMPLE_RATE * 1.0;
      mv->velocity = ((FTYPE)midi_velocity) / 127.0;
      simple_synth_note_on(mv, midi_note);
      return mv - voice->voices;
    }
  }

  return 64;
}

void
voice_note_off(Voice voice, uint8_t mono_voice_index)
{
  MonoVoice mv = voice->voices + mono_voice_index;
  if (voice_playing(mv)) {
    simple_synth_note_off(mv);
  } else {
    // TODO what?
  }
}
