#ifndef SIMPLE_SYNTH_H
#define SIMPLE_SYNTH_H

#include "../lib/macros.h"

#include "../ugen/operator.h"

#include "voice.h"

typedef struct simple_synth_params_t {
  FTYPE decay_rate;
  FTYPE fback_s;
} simple_synth_params;

void simple_synth_init(MonoVoice mv, mono_voice_params params);
void simple_synth_cleanup(MonoVoice mv);
void simple_synth_note_on(MonoVoice mv, uint8_t midi_note, FTYPE velocity);
void simple_synth_note_off(MonoVoice mv);
void simple_synth_play_chunk(MonoVoice mv, FTYPE bufs[2][CHUNK_SIZE]);

#endif
