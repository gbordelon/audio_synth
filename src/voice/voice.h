#ifndef VOICE_H
#define VOICE_H

#include <stdint.h>
#include <stdbool.h>

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../osc/osc.h"
#include "../pcm/channel.h"

#define NUM_VOICES 64

#define voice_playing(i) ((i)->sustain)

typedef struct mono_voice_t {
  Osc oscillators;
  size_t osc_num;
  Envelope env;
  size_t max_dur;
  size_t cur_dur;
  bool sustain;
} *MonoVoice;

typedef struct voice_t {
  Channel channels;
  size_t channel_num;
  MonoVoice voices;
  size_t voice_num;
} *Voice;

Voice voice_init(Channel channels, size_t channel_num);
void voice_cleanup();
void voice_play_chunk(Voice voice);
void voice_play_config(Voice voice, uint8_t midi_note, FTYPE dur);

uint8_t voice_note_on(Voice voice, uint8_t midi_note);
void voice_note_off(Voice voice, uint8_t mono_voice_index);

#endif
