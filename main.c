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
#include "src/dsp/class_a_tube_pre.h"
#include "src/dsp/dsp.h"
#include "src/dsp/reverb_tank.h"
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
Voice gsynth[2];
Voice gmic;

int
main(int argc, char * argv[])
{
  icky_global_program_name = argv[0];
  //set_signal_handler();

  ugen_generate_tables();
  printf("wavetables generated\n");

  gmix = mixer_init(3, 0.707);
  printf("mixer initialized.\n");

/* gsynth */
  Channel chans = gmix->busses[0].channels;
  audio_delay_params params_ad;
  audio_filter_params params_af;
  mono_voice_params params_mv = {0};
  Ugen ug;

  dx7_e_piano_1(&params_mv);
  gsynth[0] = voice_init(chans, NUM_CHANNELS, VOICE_DX7, params_mv);
  chans = gmix->busses[1].channels;
  gsynth[1] = voice_init(chans, NUM_CHANNELS, VOICE_SIMPLE_SYNTH, params_mv);

/*
  // panning
  ug = ugen_init_tri(0.05, DEFAULT_SAMPLE_RATE);
  ugen_set_scale(ug, 0.3, 0.7);
  dsp_set_control_ugen(gsynth[0]->fx_chain, ug);
*/
  // add gsynth fx backward
  DSP_callback dsp_fx_l, dsp_fx_r;

  {
  dsp_fx_l = dsp_init_tester_default();
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);
  }

  {
  dsp_fx_l = dsp_init_reverb_tank_default();
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);
  }

  // last is delay
  {
  dsp_fx_l = dsp_init_audio_delay_default(); // stereo
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);
  }

  // then env follower or phaser or chorus or flanger or vibrato
  {
  dsp_fx_l = dsp_init_envelope_follower_default();
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_modulated_delay_chorus_default();
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_modulated_delay_flanger_default();
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_modulated_delay_vibrato_default();
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_phase_shifter_default();
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);
  }

  //add_filter(gsynth[0], dsp_fx, params_af, AF_HPF2, 400.0, 5.707, 0.0);
  //add_filter(gsynth[0], dsp_fx, params_af, AF_LPF2, 4000.0, 5.707, 0.0);

  // first is distortion
  {
  dsp_fx_l = dsp_init_bitcrusher();
  dsp_set_bitcrusher_param(&dsp_fx_l->state, 5.5);
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_class_a_tube_pre_default();
  //gsynth[0]->fx_chain = dsp_add_to_chain(gsynth[0]->fx_chain, dsp_fx_l);
  }
/* end gsynth[0] */

/* gmic */
  chans = gmix->busses[2].channels;
  gmic = voice_init(chans, NUM_CHANNELS, VOICE_MIC_IN, params_mv);

  // set slow triangle stereo pan on gmic
  //gmic->fx_chain = dsp_init_stereo_pan();
  ug = ugen_init_tri(0.08, DEFAULT_SAMPLE_RATE);
  ugen_set_scale(ug, 0.3, 0.7);
  //dsp_set_control_ugen(gmic->fx_chain, ug);

  dsp_fx_l = dsp_init_reverb_tank_default();
  //gmic->fx_chain = dsp_add_to_chain(gmic->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_tester_default();
  //gmic->fx_chain = dsp_add_to_chain(gmic->fx_chain, dsp_fx_l);

  // last is delay
  dsp_fx_l = dsp_init_audio_delay_default(); // stereo
  //gmic->fx_chain = dsp_add_to_chain(gmic->fx_chain, dsp_fx_l);

  // then env follower or phaser or chorus or flanger or vibrato
  dsp_fx_l = dsp_init_envelope_follower_default();
  //gmic->fx_chain = dsp_add_to_chain(gmic->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_modulated_delay_chorus_default();
  //gmic->fx_chain = dsp_add_to_chain(gmic->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_modulated_delay_flanger_default();
  //gmic->fx_chain = dsp_add_to_chain(gmic->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_modulated_delay_vibrato_default();
  //gmic->fx_chain = dsp_add_to_chain(gmic->fx_chain, dsp_fx_l);

  dsp_fx_l = dsp_init_phase_shifter_default();
  //gmic->fx_chain = dsp_add_to_chain(gmic->fx_chain, dsp_fx_l);

  // telephone style filter uses a LPF at 4k and a HPF at 400
  //add_filter(gmic, dsp_fx, params_af, AF_HPF2, 400.0, 5.707, 0.0);
  //add_filter(gmic, dsp_fx, params_af, AF_LPF2, 4000.0, 5.707, 0.0);

  dsp_fx_l = dsp_init_bitcrusher();
  dsp_set_bitcrusher_param(&dsp_fx_l->state, 7.5);
  //gmic->fx_chain = dsp_add_to_chain(gmic->fx_chain, dsp_fx_l);

/* end gmic */

  printf("instrument initialized.\n");

  AudioComponentInstance audio_unit_io = audio_unit_io_init();
  printf("AudioUnit io initialized.\n");

  uint8_t active_voices[2][128];
  memset(active_voices, 64, 2*128);

  PmQueue *midi_to_main = midi_listener_init();
  midi_start();
  printf("MIDI in initialized.\n");

  audio_unit_go(audio_unit_io);
  printf("Synth started.\n");
  fflush(stdout);

  voice_note_on(gmic, 30, 127);

  int32_t msg;
  int command;    /* the current command */
  uint8_t note_ind;
  int spin;
  int num_active_notes = 0;
  int instrument_select = 0;
  for (;;) {
    // read midi messages
    spin = Pm_Dequeue(midi_to_main, &msg);
    if (spin) {
      command = Pm_MessageStatus(msg) & MIDI_CODE_MASK;
      if (command == MIDI_ON_NOTE) {
        if (active_voices[instrument_select][Pm_MessageData1(msg)] == 64) {
          note_ind = voice_note_on(gsynth[instrument_select], Pm_MessageData1(msg), Pm_MessageData2(msg));
          active_voices[instrument_select][Pm_MessageData1(msg)] = note_ind;
          num_active_notes++;
        } else {
        }
      } else if (command == MIDI_OFF_NOTE) {
        if (active_voices[instrument_select][Pm_MessageData1(msg)] < 64) {
          voice_note_off(gsynth[instrument_select], active_voices[instrument_select][Pm_MessageData1(msg)]);
          active_voices[instrument_select][Pm_MessageData1(msg)] = 64;
          num_active_notes--;
        }
      } else if (command == MIDI_CTRL) {
        if (Pm_MessageData1(msg) < MIDI_ALL_SOUND_OFF) {
        } else {
        }
      } else if (command == MIDI_CH_PROGRAM) {
        instrument_select = Pm_MessageData1(msg);
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
