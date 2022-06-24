#ifndef VOICE_H
#define VOICE_H

#include <stdint.h>
#include <stdbool.h>

#include "../lib/macros.h"

#include "../dsp/dsp.h"
#include "../env/envelope.h"
#include "../ugen/ugen.h"
#include "../pcm/channel.h"

#define NUM_VOICES 64

typedef struct mono_voice_t {
  Ugen *ugens;
  size_t ugen_num;
  Envelope env;
  size_t max_dur;
  size_t cur_dur;
  bool sustain;
  FTYPE velocity;
} *MonoVoice;

typedef struct mv_fns_t {
  void (*init)(MonoVoice mv);
  void (*cleanup)(MonoVoice mv);
  void (*note_on)(MonoVoice mv, uint8_t velocity);
  void (*note_off)(MonoVoice mv);
  void (*play_chunk)(MonoVoice mv, FTYPE samples[2][CHUNK_SIZE]);
} mv_fns;

typedef struct voice_t {
  Channel channels;
  size_t channel_num;
  Envelope env_proto;
  MonoVoice voices;
  size_t voice_num;
  mv_fns fns;
  DSP_callback fx_chain;
} *Voice;

typedef enum {
  VOICE_SIMPLE_SYNTH,
  VOICE_MIC_IN
} instrument_e;

Voice voice_init(Channel channels, size_t channel_num, instrument_e instrument);
Voice voice_init_default(Channel channels, size_t channel_num);

void voice_cleanup();
void voice_play_chunk(Voice voice);
void voice_play_config(Voice voice, uint8_t midi_note, FTYPE dur);

uint8_t voice_note_on(Voice voice, uint8_t midi_note, uint8_t midi_velocity);
void voice_note_off(Voice voice, uint8_t mono_voice_index);

bool mono_voice_playing(MonoVoice mv);
#endif
