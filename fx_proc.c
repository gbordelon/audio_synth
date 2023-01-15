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

int
main(int argc, char * argv[])
{
  icky_global_program_name = argv[0];
  //set_signal_handler();

  ugen_generate_tables();
  printf("wavetables generated\n");

  fx_unit_params params;

/* waveshaper */
  params = fx_unit_waveshaper_default();
  fx_unit_idx waveshaper = fx_unit_waveshaper_init(&params);
  // one parent, the mac audio input buffer
  fx_unit_add_parent_ref(waveshaper, 0);
/* */

/* bitcrusher */
//  params = fx_unit_bitcrusher_default();
//  fx_unit_idx bitcrusher = fx_unit_bitcrusher_init(&params);
  // one parent, the mac audio input buffer
//  fx_unit_add_parent_ref(bitcrusher, 0);
/* */

/* envelope follower */
  params = fx_unit_envelope_follower_default();
  fx_unit_idx envelope_follower = fx_unit_envelope_follower_init(&params);
  // one parent, the mac audio input buffer
  fx_unit_idx env2 = fx_unit_envelope_follower_set_parent(envelope_follower, waveshaper);
/* */

/* audio delay */
  params = fx_unit_audio_delay_default();
  fx_unit_idx audio_delay = fx_unit_audio_delay_init(&params);
  // one parent, the mac audio input buffer
  fx_unit_add_parent_ref(audio_delay, env2);
/* */

/* control signal generator */
  params = fx_unit_signal_source_constant_default();
  fx_unit_idx signal_source = fx_unit_signal_source_init(&params);
  // no parents
/* */

/* control signal joiner */
  memset(&params.u, 0, sizeof(params.u));
  params.t = FX_UNIT_CONTROL_JOINER;
  fx_unit_idx control_joiner = fx_unit_control_joiner_init(&params);
  // two parents
  // 1st is LR channels (from audio delay)
  fx_unit_add_parent_ref(control_joiner, audio_delay);
  // 2nd is C channel (signal source)
  fx_unit_add_parent_ref(control_joiner, signal_source);
/* */

/* panner */
  params = fx_unit_pan_default();
  fx_unit_idx pan = fx_unit_pan_init(&params);
  // one parent
  fx_unit_add_parent_ref(pan, control_joiner);

  // adjust current fx chain appropriately
  fx_unit_replace_parent_ref(1, pan);
/* */

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
