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

#include "src/midi/midi.h"
#include "src/pcm/mixer.h"
#include "src/ugen/ugen.h"
#include "src/voice/voice.h"
#include "src/voice/dx7.h"
#include "src/tunable/tunable.h"
#include "src/cli/cli.h"

extern char const * icky_global_program_name;

Voice gsynth[2];

extern FX_unit fx_unit_head;

#define idx_print_type(unit_idx) {\
const char * const fx_unit_names[] = { NAMES };\
printf( "%s", fx_unit_names[fx_unit_head[unit_idx].state.t]);\
}

void
DEBUG_FX_UNIT(size_t indent, fx_unit_idx unit_idx)
{
  int i;
  if (indent > 0) {
    char *str = calloc(indent + 1, sizeof(char));
    memset(str, ' ', indent * sizeof(char));
    printf("%s", str);
    free(str);
  }
  printf("%u: ", unit_idx);
  idx_print_type(unit_idx);
  printf("\n");
  
  for (i = 0; i < fx_unit_head[unit_idx].num_parents; i++) {
    DEBUG_FX_UNIT(indent + 2, fx_unit_head[unit_idx].parents[i]);
  }
}
#undef idx_print_type

int
main(int argc, char * argv[])
{
  icky_global_program_name = argv[0];
//  set_signal_handler();

  ugen_generate_tables();
  printf("wavetables generated\n");

  fx_unit_params params, params2, params3;
  FX_compound_unit prev;

/* init the fx lib */
  params = fx_unit_passthru_default();
  FX_compound_unit passthru = fx_compound_unit_passthru_init(&params);

  // adjust current fx chain appropriately
  fx_unit_insert_as_parent(0, passthru);
  prev = passthru;
/* */

  mono_voice_params params_mv = {0};

  dx7_e_piano_1(&params_mv);
  gsynth[0] = voice_init(0, VOICE_DX7, params_mv);
  gsynth[1] = voice_init(0, VOICE_SIMPLE_SYNTH, params_mv);

  printf("instrument initialized.\n");

  cli_menu_init_menu_system(true, false, gsynth[0]->menu, gsynth[1]->menu, NULL);
  printf("menus initialized\n");

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
    } // if (spin)
    cli_entry_point();
  }

  midi_stop();
  midi_cleanup();
  voice_cleanup(gsynth);
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
