#ifndef VOICE_H
#define VOICE_H

#include <stdint.h>
#include <stdbool.h>

#include "../lib/macros.h"

#include "../dsp/dsp.h"
#include "../env/envelope.h"
#include "../ugen/operator.h"
#include "../pcm/channel.h"

#include "dx7.h"
#include "simple_synth.h"

#define NUM_VOICES 64

typedef union mono_voice_params_u {
  dx7_params dx7;
  simple_synth_params ss;
} mono_voice_params;

typedef struct mono_voice_t {
  Operator *ops;
  size_t op_num;
  size_t cur_dur;
  bool sustain;
  mono_voice_params params;
} *MonoVoice;

typedef struct mv_fns_t {
  void (*init)(MonoVoice mv, mono_voice_params params);
  void (*cleanup)(MonoVoice mv);
  void (*note_on)(MonoVoice mv, uint8_t midi_note, FTYPE velocity);
  void (*note_off)(MonoVoice mv);
  void (*play_chunk)(MonoVoice mv, FTYPE samples[3][CHUNK_SIZE]);
} mv_fns;

typedef struct voice_t {
  Channel channels;
  size_t channel_num;
  MonoVoice voices;
  size_t voice_num;
  mv_fns fns;
  DSP_callback fx_chain;
} *Voice;

typedef enum {
  VOICE_SIMPLE_SYNTH,
  VOICE_DX7,
  VOICE_MIC_IN
} instrument_e;

Voice voice_init(Channel channels, size_t channel_num, instrument_e instrument, mono_voice_params params);
Voice voice_init_default(Channel channels, size_t channel_num);

void voice_cleanup();
void voice_play_chunk(Voice voice);
void voice_play_config(Voice voice, uint8_t midi_note, FTYPE dur);

uint8_t voice_note_on(Voice voice, uint8_t midi_note, uint8_t midi_velocity);
void voice_note_off(Voice voice, uint8_t mono_voice_index);

bool mono_voice_playing(MonoVoice mv);

#endif
