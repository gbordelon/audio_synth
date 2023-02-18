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

#include <stdio.h>
void
voice_set_params(Voice voice, voice_params *p)
{
  instrument_e current_instrument = VOICE_NONE;
  if (voice->fns.init == dx7_init) {
    current_instrument = VOICE_DX7;
  } else if (voice->fns.init == simple_synth_init) {
    current_instrument = VOICE_SIMPLE_SYNTH;
  }

  MonoVoice mv;
  if (current_instrument != p->instrument) {
    if (voice->voices != NULL) {
      for (mv = voice->voices; mv - voice->voices < voice->voice_num; mv++) {
        voice->fns.cleanup(mv);
      }
    }

    switch (p->instrument) {
    case VOICE_DX7:
      voice->fns.init = dx7_init;
      voice->fns.cleanup = dx7_cleanup;
      voice->fns.note_on = dx7_note_on;
      voice->fns.note_off = dx7_note_off;
      voice->fns.play_chunk = dx7_play_chunk;
      break;
    case VOICE_MIC_IN:
      voice->voice_num = 1;
      voice->fns.init = mic_in_init;
      voice->fns.cleanup = mic_in_cleanup;
      voice->fns.note_on = mic_in_note_on;
      voice->fns.note_off = mic_in_note_off;
      voice->fns.play_chunk = mic_in_play_chunk;
      break;
    case VOICE_SIMPLE_SYNTH:
      //fall through
    default:
      voice->fns.init = simple_synth_init;
      voice->fns.cleanup = simple_synth_cleanup;
      voice->fns.note_on = simple_synth_note_on;
      voice->fns.note_off = simple_synth_note_off;
      voice->fns.play_chunk = simple_synth_play_chunk;
      break;
    }

    if (voice->voices == NULL) {
      voice->voices = mono_voice_alloc();
      voice->voice_num = NUM_VOICES;
    }

    for (mv = voice->voices; mv - voice->voices < voice->voice_num; mv++) {
      voice->fns.init(mv, p->mvp);
      mv->cur_dur = 0;
    }
    cli_menu_add_menu(voice->menu, voice->voices[0].ops[0]->menu);
    if (p->instrument == VOICE_SIMPLE_SYNTH || p->instrument == VOICE_DX7) {
      cli_menu_add_menu(voice->menu, voice->voices[0].ops[1]->menu);
      cli_menu_add_menu(voice->menu, voice->voices[0].ops[2]->menu);
      cli_menu_add_menu(voice->menu, voice->voices[0].ops[3]->menu);
    }
    if (p->instrument == VOICE_DX7) {
      cli_menu_add_menu(voice->menu, voice->voices[0].ops[4]->menu);
      cli_menu_add_menu(voice->menu, voice->voices[0].ops[5]->menu);
    }
  } else {
    for (mv = voice->voices; mv - voice->voices < voice->voice_num; mv++) {
      voice->fns.init(mv, p->mvp);
      mv->cur_dur = 0;
    }
  }
}

Voice
voice_init(fx_unit_idx output, instrument_e instrument, mono_voice_params params)
{
  Voice rv = voice_alloc();

  fx_unit_params fxp = fx_unit_buffer_default();
  rv->buffer = fx_unit_buffer_init(&fxp);
  fx_unit_parent_ref_add(output, rv->buffer);

  voice_params p = {
    .instrument = instrument,
    .mvp = params
  };

  // create voice menu
  rv->menu = cli_menu_init(CLI_MENU, "Instrument Menu", "Edit instrument parameters");
  // attach a copy of params
  rv->tunables.p = p;
  // track tunables for freeing
  size_t num_tunables = 3;
  if (instrument == VOICE_DX7) {
    num_tunables = 4;
  }
  rv->tunables.ts = calloc(num_tunables, sizeof(Tunable));

  tunable_arg args[2];
  tunable_fn fn;
  args[0].v = (void *)rv;
  args[1].v = (void *)&rv->tunables.p;
  fn.f2pp = voice_set_params;

  voice_set_params(rv, &p);

  // create instrument tunable
  Tunable t = tunable_init(TUNABLE_INT32,
      TUNABLE_RANGE_0_127,
      &rv->tunables.p.instrument,
      args,
      ARITY_2_PP,
      &fn,
      "Instrument Choice [1-2], 1: simple synth, 2: dx7 simulator"
  );
  rv->tunables.ts[0] = t;
  cli_menu_add_tunable(rv->menu, t);

  // create feedback sensitivity tunable
  t = tunable_init(TUNABLE_DOUBLE,
      TUNABLE_RANGE_0_1,
      (instrument == VOICE_SIMPLE_SYNTH ? &rv->tunables.p.mvp.ss.fback_s : &rv->tunables.p.mvp.dx7.fback_s),
      args,
      ARITY_2_PP,
      &fn,
      "Feedback Sensitivity [0-1]"
  );
  rv->tunables.ts[1] = t;
  cli_menu_add_tunable(rv->menu, t);

  // create feedback sensitivity tunable
  t = tunable_init(TUNABLE_DOUBLE,
      TUNABLE_RANGE_0_1,
      (instrument == VOICE_SIMPLE_SYNTH ? &rv->tunables.p.mvp.ss.decay_rate : &rv->tunables.p.mvp.dx7.decay_rate),
      args,
      ARITY_2_PP,
      &fn,
      "Decay Rate [0-1]"
  );
  rv->tunables.ts[2] = t;
  cli_menu_add_tunable(rv->menu, t);

  if (instrument == VOICE_DX7) {
    // create algorithm tunable
    t = tunable_init(TUNABLE_INT32,
        TUNABLE_RANGE_0_127,
        &rv->tunables.p.mvp.dx7.alg,
        args,
        ARITY_2_PP,
        &fn,
        "Algorithm [0-31], 0: alg 1, 1: alg 2, etc."
    );
    rv->tunables.ts[3] = t;
    cli_menu_add_tunable(rv->menu, t);
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

  cli_menu_cleanup(voice->menu);
  int i;
  for (i = 0; i < 3; i++) {
    tunable_cleanup(voice->tunables.ts[i]);
  }
  if (voice->tunables.p.instrument == VOICE_DX7) {
    tunable_cleanup(voice->tunables.ts[3]);
  }
  free(voice->tunables.ts);
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
  // iterate over NUM_VOICES voices
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
