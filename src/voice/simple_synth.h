#ifndef SIMPLE_SYNTH_H
#define SIMPLE_SYNTH_H

#include "../lib/macros.h"

#include "voice.h"

void simple_synth_init(MonoVoice mv);
void simple_synth_cleanup(MonoVoice mv);
void simple_synth_note_on(MonoVoice mv, uint8_t midi_note);
void simple_synth_note_off(MonoVoice mv);
void simple_synth_play_chunk(MonoVoice mv, FTYPE bufs[2][CHUNK_SIZE]);

#endif
