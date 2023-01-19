#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../fx/fx.h"
#include "../midi/midi.h"
#include "../ugen/ugen.h"

#include "voice.h"

#include "dx7.h"
#include "mic_in.h"
#include "simple_synth.h"

MonoVoice
mono_voice_alloc()
{
  return calloc(NUM_VOICES, sizeof(struct mono_voice_t));
}

void
mono_voice_free(MonoVoice voice)
{
  free(voice);
}

Voice
voice_alloc()
{
  return calloc(1, sizeof(struct voice_t));
}

void
voice_free(Voice voice)
{
  free(voice);
}

Voice
voice_init(fx_unit_idx output, instrument_e instrument, mono_voice_params params)
{
  Voice rv = voice_alloc();
  rv->voices = mono_voice_alloc();
  rv->voice_num = NUM_VOICES;

  fx_unit_params p = fx_unit_buffer_default();
  rv->buffer = fx_unit_buffer_init(&p);
  fx_unit_parent_ref_add(output, rv->buffer);

  switch(instrument) {
  case VOICE_DX7:
    rv->fns.init = dx7_init;
    rv->fns.cleanup = dx7_cleanup;
    rv->fns.note_on = dx7_note_on;
    rv->fns.note_off = dx7_note_off;
    rv->fns.play_chunk = dx7_play_chunk;
    break;
  case VOICE_MIC_IN:
    rv->voice_num = 1;
    rv->fns.init = mic_in_init;
    rv->fns.cleanup = mic_in_cleanup;
    rv->fns.note_on = mic_in_note_on;
    rv->fns.note_off = mic_in_note_off;
    rv->fns.play_chunk = mic_in_play_chunk;
    break;
  case VOICE_SIMPLE_SYNTH:
    //fall through
  default:
    rv->fns.init = simple_synth_init;
    rv->fns.cleanup = simple_synth_cleanup;
    rv->fns.note_on = simple_synth_note_on;
    rv->fns.note_off = simple_synth_note_off;
    rv->fns.play_chunk = simple_synth_play_chunk;
    break;
  }

  MonoVoice mv;
  for (mv = rv->voices; mv - rv->voices < rv->voice_num; mv++) {
    rv->fns.init(mv, params);
    mv->cur_dur = 0;
  }

  return rv;
}

Voice
voice_init_default(fx_unit_idx output)
{
  mono_voice_params params = {0};

  Voice rv = voice_init(output, VOICE_SIMPLE_SYNTH, params);

  return rv;
}

void
voice_cleanup(Voice voice)
{
  // TODO for channel in channels: reduce refcount
  MonoVoice mv;
  for (mv = voice->voices; mv - voice->voices < voice->voice_num; mv++) {
    voice->fns.cleanup(mv);
  }
  mono_voice_free(voice->voices);
  fx_unit_cleanup(voice->buffer);
  voice_free(voice);
}

#include <stdio.h>
void
voice_play_chunk(Voice voice)
{
  static FTYPE samples[2][CHUNK_SIZE];
  static FTYPE accum[CHUNK_SIZE << 1];
  FTYPE *sL, *sR, *L, *R;

  memset(accum, 0, (CHUNK_SIZE << 1) * sizeof(FTYPE));

  bool peak = false;
  // iterate over 64 voices
  MonoVoice mv;
  for (mv = voice->voices; mv - voice->voices < voice->voice_num; mv++) {
    if (mono_voice_playing(mv)) {
      voice->fns.play_chunk(mv, samples);
      for (L = accum, R = accum + 1, sL = samples[0], sR = samples[1];
           R - accum < (NUM_CHANNELS * CHUNK_SIZE);
           sL++, sR++, L += 2, R += 2) {
        *L += *sL;
        *R += *sR;
        peak = peak || fabs(*L) > 1.0 || fabs(*R) > 1.0;
      }
    }
  }
  if (peak) {
    printf("peaked\n");
  }

  fx_unit_buffer_write_chunk(voice->buffer, accum, 0);
}

uint8_t
voice_note_on(Voice voice, uint8_t midi_note, uint8_t midi_velocity)
{
  static const FTYPE INV_127 = 1.0 / 127.0;

  MonoVoice mv;
  for (mv = voice->voices; mv - voice->voices < voice->voice_num; mv++) {
    if (!mono_voice_playing(mv)) {
      mv->cur_dur = 0;
      voice->fns.note_on(mv, midi_note, ((FTYPE)midi_velocity) * INV_127);
      return mv - voice->voices;
    }
  }

  return 64;
}

void
voice_note_off(Voice voice, uint8_t mono_voice_index)
{
  MonoVoice mv = voice->voices + mono_voice_index;

  voice->fns.note_off(mv);
}

/*
 * return true if envelope still has samples remaining.
 * but only if the voice has been triggered.
 */
bool
mono_voice_playing(MonoVoice mv)
{
  if (mv->op_num == 0) {
    return mv->sustain;
  }

  Operator *op;
  bool rv = false;

  for (op = mv->ops; op - mv->ops < mv->op_num; op++) {
    rv = rv
       || ((*op)->e_type == OPERATOR_UGEN && mv->sustain)
       || ((*op)->e_type == OPERATOR_ENV && !env_spent((*op)->env_u.env))
       || ((*op)->e_type == OPERATOR_NONE && mv->sustain);
  }
  return rv;
}
