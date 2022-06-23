#ifndef MIC_IN_H
#define MIC_IN_H

#include "../lib/macros.h"

#include "voice.h"

void mic_in_init(MonoVoice mv);
void mic_in_cleanup(MonoVoice mv);
void mic_in_note_on(MonoVoice mv, uint8_t midi_note);
void mic_in_note_off(MonoVoice mv);
void mic_in_play_chunk(MonoVoice mv, FTYPE bufs[2][CHUNK_SIZE]);

#endif