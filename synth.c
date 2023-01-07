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
/* gmic */

  chans = gmix->busses[2].channels;
  gmic = voice_init(chans, NUM_CHANNELS, VOICE_MIC_IN, params_mv);

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

  my_midi_data msg_raw;
  int32_t *msg = (int32_t *)&(msg_raw.u.data[0]);
  int command;    /* the current command */
  uint8_t note_ind;
  int spin;
  int num_active_notes = 0;
  int instrument_select = 0;
  int i;
  for (;;) {
    // read midi messages
    spin = Pm_Dequeue(midi_to_main, &msg_raw);
    if (spin) {
      command = Pm_MessageStatus(*msg) & MIDI_CODE_MASK;

      if (command == MIDI_SYSEX || msg_raw.sysex_size > 0) {
        printf("sysex_size %u: msg:", msg_raw.sysex_size);
        int j;
        for (j = 0; j < msg_raw.sysex_size; j++) {
          if (j % 4 == 0) {
            printf(" ");
          }
          printf("%2x", msg_raw.u.sysex_data[j]);
        }
        printf("\n");
      } else if (command == MIDI_ON_NOTE) {
        if (active_voices[instrument_select][Pm_MessageData1(*msg)] == 64) {
          note_ind = voice_note_on(gsynth[instrument_select], Pm_MessageData1(*msg), Pm_MessageData2(*msg));
          active_voices[instrument_select][Pm_MessageData1(*msg)] = note_ind;
          num_active_notes++;
        } else {
        }
      } else if (command == MIDI_OFF_NOTE) {
        if (active_voices[instrument_select][Pm_MessageData1(*msg)] < 64) {
          voice_note_off(gsynth[instrument_select], active_voices[instrument_select][Pm_MessageData1(*msg)]);
          active_voices[instrument_select][Pm_MessageData1(*msg)] = 64;
          num_active_notes--;
        }
      } else if (command == MIDI_CTRL) {
        if (Pm_MessageData1(*msg) < MIDI_ALL_SOUND_OFF) {
        } else {
        }
      } else if (command == MIDI_CH_PROGRAM) {
        instrument_select = Pm_MessageData1(*msg);
        if (instrument_select != 0 && instrument_select != 1) {
          instrument_select = 0;
        }
      } else {
      }
    } 
  }

  midi_stop();
  midi_cleanup();
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
