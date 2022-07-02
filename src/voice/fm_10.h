#ifndef FM_10_H
#define FM_10_H

#include "../lib/macros.h"

#include "voice.h"

typedef struct fm_10_params_t {
  Envelope e;
  FTYPE p6;
  FTYPE p7;
  FTYPE p8;
  FTYPE carrier;
} fm_10_params;

// forward decl
typedef struct mono_voice_t *MonoVoice;
typedef union mono_voice_params_u mono_voice_params;

void fm_10_init(MonoVoice mv, mono_voice_params params);
void fm_10_init_default(MonoVoice mv);
void fm_10_cleanup(MonoVoice mv);
void fm_10_note_on(MonoVoice mv, uint8_t midi_note);
void fm_10_note_off(MonoVoice mv);
void fm_10_play_chunk(MonoVoice mv, FTYPE bufs[3][CHUNK_SIZE]);

#endif
