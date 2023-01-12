#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../midi/midi.h"
#include "../ugen/imp.h"
#include "../ugen/sin.h"
#include "../ugen/ugen.h"

#include "voice.h"
#include "mic_in.h"

// declared in src/mac_audio/audio_unit.c
extern FTYPE *_mic_output_buffer_L;
extern FTYPE *_mic_output_buffer_R;
extern size_t _mic_output_read_index;

void
mic_in_init(MonoVoice mv, mono_voice_params params)
{
}

void
mic_in_cleanup(MonoVoice mv)
{
}


void
mic_in_note_on(MonoVoice mv, uint8_t midi_note, FTYPE velocity)
{
  mv->sustain = true;
}

void
mic_in_note_off(MonoVoice mv)
{
  mv->sustain = false;
}

void
mic_in_play_chunk(MonoVoice mv, FTYPE bufs[2][CHUNK_SIZE])
{
  size_t cpy_amt = CHUNK_SIZE;
  if ((cpy_amt + _mic_output_read_index) >= CHUNK_SIZE * NUM_CHANNELS * 8) {
    cpy_amt = CHUNK_SIZE * NUM_CHANNELS * 8 - _mic_output_read_index;
  }
  memcpy(bufs[0], _mic_output_buffer_L + _mic_output_read_index, cpy_amt * sizeof(FTYPE));
  memcpy(bufs[1], _mic_output_buffer_R + _mic_output_read_index, cpy_amt * sizeof(FTYPE));
  memset(_mic_output_buffer_L + _mic_output_read_index, 0, cpy_amt * sizeof(FTYPE));
  memset(_mic_output_buffer_R + _mic_output_read_index, 0, cpy_amt * sizeof(FTYPE));
  _mic_output_read_index += cpy_amt;
  if (_mic_output_read_index >= CHUNK_SIZE * NUM_CHANNELS * 8) {
    _mic_output_read_index = 0;
  }
  mv->cur_dur += cpy_amt;
}
