#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <AudioUnit/AudioUnit.h>

#include "src/lib/macros.h"
#include "src/lib/signals.h"

#include "src/mac_audio/audio_unit.h"

#include "src/dsp/audio_filter.h"
#include "src/dsp/bitcrusher.h"
#include "src/dsp/dsp.h"
#include "src/env/envelope.h"
#include "src/midi/midi.h"
#include "src/pcm/mixer.h"
#include "src/pcm/pcm.h"
#include "src/ugen/sin.h"
#include "src/ugen/ugen.h"
#include "src/voice/voice.h"
#include "src/voice/dx7.h"

#define add_filter(vc, fx, params, filter_type, Fc, Q, dB)\
  (params).alg=(filter_type);\
  (params).fc=(Fc);\
  (params).q=(Q);\
  (params).boost_cut_db=(dB);\
  fx##_l = dsp_init_audio_filter(params);\
  fx##_r = dsp_init_audio_filter(params);\
  (fx##_r)->fn_type = DSP_MONO_R;\
  (vc)->fx_chain = dsp_add_to_chain(vc->fx_chain, fx##_l);\
  (vc)->fx_chain = dsp_add_to_chain(vc->fx_chain, fx##_r)

extern char const * icky_global_program_name;

Mixer gmix;
Voice gsynth;
Voice gmic;

int
main(int argc, char * argv[])
{
  icky_global_program_name = argv[0];
  //set_signal_handler();

  printf("generating wave tables... ");
  fflush(stdout);
  clock_t t = clock();
  ugen_generate_tables();
  t = clock() - t;
  double tt = t / (double)CLOCKS_PER_SEC;
  printf("took %f seconds.\n", tt);

  gmix = mixer_init(2, 1.0);
  gmix->gain = 0.7;
  printf("mixer initialized.\n");

/* gsynth */
  Channel chans = gmix->busses[0].channels;
  //gsynth = voice_init_default(chans, NUM_CHANNELS);
  audio_filter_params params;
  mono_voice_params mv_params = {0};
  Ugen ug;

  dx7_e_piano_1(&mv_params);
  gsynth = voice_init(chans, NUM_CHANNELS, VOICE_DX7, mv_params);
  //gsynth = voice_init(chans, NUM_CHANNELS, VOICE_SIMPLE_SYNTH, mv_params);

  DSP_callback cb = dsp_init_envelope_follower_default();
  //gsynth->fx_chain = dsp_add_to_chain(gsynth->fx_chain, cb);
  cb = dsp_init_envelope_follower_default();
  cb->fn_type = DSP_MONO_R;
  //gsynth->fx_chain = dsp_add_to_chain(gsynth->fx_chain, cb);

  DSP_callback dsp_fx_l, dsp_fx_r;
  // precede env follower with a bitcrusher on each channel
  dsp_fx_l = dsp_init_bitcrusher();
  dsp_set_bitcrusher_param(&dsp_fx_l->state, 4.5);

  dsp_fx_r = dsp_init_bitcrusher();
  dsp_fx_r->fn_type = DSP_MONO_R;
  dsp_set_bitcrusher_param(&dsp_fx_r->state, 4.5);

  //gsynth->fx_chain = dsp_add_to_chain(gsynth->fx_chain, dsp_fx_l);
  //gsynth->fx_chain = dsp_add_to_chain(gsynth->fx_chain, dsp_fx_r);

  //add_filter(gsynth, dsp_fx, params, AF_HPF2, 400.0, 5.707, 0.0);
  //add_filter(gsynth, dsp_fx, params, AF_LPF2, 4000.0, 5.707, 0.0);

/*
  // panning
  ug = ugen_init_tri(0.05);
  ugen_set_scale(ug, 0.3, 0.7);
  dsp_set_control_ugen(gsynth->fx_chain, ug);
*/
/* end gsynth */

/* gmic */
  chans = gmix->busses[1].channels;
  gmic = voice_init(chans, NUM_CHANNELS, VOICE_MIC_IN, mv_params);

  // set slow triangle stereo pan on gmic
  gmic->fx_chain = dsp_init_stereo_pan();
//  ug = ugen_init_tri(0.08);
//  ugen_set_scale(ug, 0.3, 0.7);
//  dsp_set_control_ugen(gmic->fx_chain, ug);

  // telephone style filter uses a LPF at 4k and a HPF at 400
  add_filter(gmic, dsp_fx, params, AF_HPF2, 400.0, 5.707, 0.0);
  add_filter(gmic, dsp_fx, params, AF_LPF2, 4000.0, 5.707, 0.0);

/* end gmic */

  printf("instrument initialized.\n");

  AudioComponentInstance audio_unit_io = audio_unit_io_init();
  printf("AudioUnit io initialized.\n");

  uint8_t active_voices[128];
  memset(active_voices, 64, 128);

  PmQueue *midi_to_main = midi_listener_init();
  midi_start();
  printf("MIDI in initialized.\n");

  audio_unit_go(audio_unit_io);
  printf("Synth started.\n");
  fflush(stdout);

  //voice_note_on(gmic, 30, 127);

  int32_t msg;
  int command;    /* the current command */
  uint8_t note_ind;
  int spin;
  int num_active_notes = 0;
  for (;;) {
    // read midi messages
    spin = Pm_Dequeue(midi_to_main, &msg);
    if (spin) {
      command = Pm_MessageStatus(msg) & MIDI_CODE_MASK;
      if (command == MIDI_ON_NOTE) {
        if (active_voices[Pm_MessageData1(msg)] == 64) {
          note_ind = voice_note_on(gsynth, Pm_MessageData1(msg), Pm_MessageData2(msg));
          active_voices[Pm_MessageData1(msg)] = note_ind;
          num_active_notes++;
        } else {
        }
      } else if (command == MIDI_OFF_NOTE) {
        if (active_voices[Pm_MessageData1(msg)] < 64) {
          voice_note_off(gsynth, active_voices[Pm_MessageData1(msg)]);
          active_voices[Pm_MessageData1(msg)] = 64;
          num_active_notes--;
        }
      } else if (command == MIDI_CTRL) {
        if (Pm_MessageData1(msg) < MIDI_ALL_SOUND_OFF) {
        } else {
        }
      }
    }
  }

  midi_stop();
  midi_cleanup();
  voice_cleanup(gmic);
  voice_cleanup(gsynth);
  mixer_cleanup(gmix);
  return 0;
}

/*
#include <math.h>
#include <stdio.h>

  FTYPE max = 0;
        if (*write_buf_r * *write_buf_r > max) {
          max = *write_buf_r * *write_buf_r;
        }
  printf("max sample mixed for this chunk: %f\n", sqrt(max));
*/
