#ifndef VOICE_H
#define VOICE_H

#include <stdint.h>
#include <stdbool.h>

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../ugen/ugen.h"
#include "../pcm/channel.h"

#define NUM_VOICES 64

#define voice_playing(i) ((i)->sustain)

typedef struct mono_voice_t {
  Ugen *ugens;
  size_t ugen_num;
  Envelope env;
  size_t max_dur;
  size_t cur_dur;
  bool sustain;
  FTYPE velocity;
} *MonoVoice;

typedef struct voice_t {
  Channel channels;
  size_t channel_num;
  Envelope env_proto;
  MonoVoice voices;
  size_t voice_num;
  Ugen pan; // 0 full left. 1 full right.
} *Voice;

Voice voice_init(Channel channels, size_t channel_num);
void voice_cleanup();
void voice_play_chunk(Voice voice);
void voice_play_config(Voice voice, uint8_t midi_note, FTYPE dur);

uint8_t voice_note_on(Voice voice, uint8_t midi_note, uint8_t midi_velocity);
void voice_note_off(Voice voice, uint8_t mono_voice_index);

#endif
