#ifndef VOICE_H
#define VOICE_H

#include <stdint.h>
#include <stdbool.h>

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../ugen/operator.h"
#include "../fx/fx.h"

#include "dx7.h"
#include "simple_synth.h"

#include "../cli/cli.h"
#include "../tunable/tunable.h"

#define NUM_VOICES 32

typedef enum {
  VOICE_NONE,
  VOICE_SIMPLE_SYNTH,
  VOICE_DX7,
  VOICE_MIC_IN
} instrument_e;

typedef union mono_voice_params_u {
  dx7_params dx7;
  simple_synth_params ss;
} mono_voice_params;

typedef struct voice_params_t {
  instrument_e instrument;
  mono_voice_params mvp;
} voice_params;

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
  void (*play_chunk)(MonoVoice mv, FTYPE samples[2][CHUNK_SIZE]);
} mv_fns;

typedef struct voice_t {
  MonoVoice voices;
  size_t voice_num;
  mv_fns fns;
  fx_unit_idx buffer;

  Cli_menu menu;
  struct {
    voice_params p;
    Tunable *ts;
  } tunables;
} *Voice;

Voice voice_init(fx_unit_idx output, instrument_e instrument, mono_voice_params params);
Voice voice_init_default(fx_unit_idx output);

void voice_set_params(Voice voice, voice_params *p);

void voice_cleanup();
void voice_play_chunk(Voice voice);
void voice_play_config(Voice voice, uint8_t midi_note, FTYPE dur);

uint8_t voice_note_on(Voice voice, uint8_t midi_note, uint8_t midi_velocity);
void voice_note_off(Voice voice, uint8_t mono_voice_index);

bool mono_voice_playing(MonoVoice mv);

#endif
