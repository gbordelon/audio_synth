#ifndef YAMAHA_DX7_H
#define YAMAHA_DX7_H

#include "../lib/macros.h"

#include "voice.h"

typedef enum {
  DX7_1,
  DX7_2,
  DX7_3,
  DX7_4,
  DX7_5,
  DX7_6,
  DX7_7,
  DX7_8,
  DX7_9,
  DX7_10,
  DX7_11,
  DX7_12,
  DX7_13,
  DX7_14,
  DX7_15,
  DX7_16,
  DX7_17,
  DX7_18,
  DX7_19,
  DX7_20,
  DX7_21,
  DX7_22,
  DX7_23,
  DX7_24,
  DX7_25,
  DX7_26,
  DX7_27,
  DX7_28,
  DX7_29,
  DX7_30,
  DX7_31,
  DX7_32
} dx7_alg;

typedef struct dx7_patch_t {
  // carrier detune in cents
  FTYPE detune[6];
  // freq mult for each operator
  FTYPE mult[6];
  // gain for each operator
  FTYPE gain[6];
  // TODO env/lfo settings for each operator
} dx7_patch;

typedef struct dx7_params_t {
  Operator ops[6];
  dx7_alg alg;
  dx7_patch patch;
} dx7_params;

// forward decl
typedef struct mono_voice_t *MonoVoice;
typedef union mono_voice_params_u mono_voice_params;

void dx7_init(MonoVoice mv, mono_voice_params params);
void dx7_init_default(MonoVoice mv);
void dx7_cleanup(MonoVoice mv);
void dx7_note_on(MonoVoice mv, uint8_t midi_note);
void dx7_note_off(MonoVoice mv);
void dx7_play_chunk(MonoVoice mv, FTYPE bufs[2][CHUNK_SIZE]);

#endif
