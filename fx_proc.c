#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <AudioUnit/AudioUnit.h>

#include "src/lib/macros.h"
// debugging
#include "src/lib/signals.h"

#include "src/mac_audio/audio_unit.h"

#include "src/midi/midi.h"

#include "src/fx/fx.h"
#include "src/fx/bitcrusher.h"
#include "src/fx/envelope_follower.h"
#include "src/fx/signal_source.h"
#include "src/fx/waveshaper.h"

extern char const * icky_global_program_name;
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
    printf(str);
    free(str);
  }
  printf("%u: ", unit_idx);
  idx_print_type(unit_idx);
  printf("\n");
  
  for (i = 0; i < fx_unit_head[unit_idx].num_parents; i++) {
    DEBUG_FX_UNIT(indent + 2, fx_unit_head[unit_idx].parents[i]);
  }
}

int
main(int argc, char * argv[])
{
  icky_global_program_name = argv[0];
  //set_signal_handler();

  ugen_generate_tables();
  printf("wavetables generated\n");

  fx_unit_params params, params2, params3;

/* panner */
  params = fx_unit_pan_default();
  params2 = fx_unit_signal_source_ugen_default();
  FX_compound_unit pan = fx_compound_unit_pan_init(&params, &params2);

  // adjust current fx chain appropriately
  fx_unit_insert_as_parent(1, pan);
/* */

/* audio delay */
  params = fx_unit_audio_delay_default();
  FX_compound_unit audio_delay = fx_compound_unit_audio_delay_init(&params);
  fx_compound_unit_insert_as_parent(pan, audio_delay);
/* */

/* envelope follower */
  params = fx_unit_envelope_follower_default();
  params2 = fx_unit_envelope_follower_audio_filter_default();
  params3 = fx_unit_envelope_follower_audio_detector_default();
  FX_compound_unit envelope_follower = fx_compound_unit_envelope_follower_init(&params, &params2, &params3);
  fx_compound_unit_insert_as_parent(audio_delay, envelope_follower);
/* */

/* comb_filter */
  params = fx_unit_comb_filter_default();
  FX_compound_unit comb_filter = fx_compound_unit_comb_filter_init(&params);
  fx_compound_unit_insert_as_parent(audio_delay, comb_filter);
/* */

/* waveshaper */
  params = fx_unit_waveshaper_default();
  FX_compound_unit waveshaper = fx_compound_unit_waveshaper_init(&params);
  // one parent, the mac audio input buffer
  fx_compound_unit_insert_as_parent(comb_filter, waveshaper);
/* */

///* bitcrusher */
//  params = fx_unit_bitcrusher_default();
//  FX_compound_unit bitcrusher = fx_compound_unit_bitcrusher_init(&params);
//  // one parent, the mac audio input buffer
//  fx_compound_unit_insert_as_parent(comb_filter, bitcrusher);
///* */

  AudioComponentInstance audio_unit_io = audio_unit_io_init();
  printf("AudioUnit io initialized.\n");

  PmQueue *midi_to_main = midi_listener_init();
  midi_start();
  printf("MIDI in initialized.\n");

  audio_unit_go(audio_unit_io);
  printf("FX Processor started.\n");
  fflush(stdout);

  my_midi_data msg_raw;
  int32_t *msg = (int32_t *)&(msg_raw.u.data[0]);
  int command;    /* the current command */
  int spin;
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
      } else if (command == MIDI_OFF_NOTE) {
      } else if (command == MIDI_CTRL) {
        if (Pm_MessageData1(*msg) < MIDI_ALL_SOUND_OFF) {
        } else {
        }
      } else if (command == MIDI_CH_PROGRAM) {
        instrument_select = Pm_MessageData1(*msg);
      }
    }
  }

  midi_stop();
  midi_cleanup();
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
