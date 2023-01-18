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
#include "src/fx/two_band_shelving_filter.h"
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
#undef idx_print_type

int
main(int argc, char * argv[])
{
  icky_global_program_name = argv[0];
  //set_signal_handler();

  ugen_generate_tables();
  printf("wavetables generated\n");

  fx_unit_params params, params2, params3;
  FX_compound_unit prev;

/* panner */
  params = fx_unit_pan_default();
  params2 = fx_unit_signal_source_constant_default();
  FX_compound_unit pan = fx_compound_unit_pan_init(&params, &params2);

  // adjust current fx chain appropriately
  fx_unit_insert_as_parent(1, pan);
  prev = pan;
/* */

/* reverb tank*/
  params = fx_unit_reverb_tank_default();
  params2 = fx_unit_reverb_tank_shelving_filter_default();
  params3 = fx_unit_reverb_tank_sum_default();
  FX_compound_unit reverb_tank = fx_compound_unit_reverb_tank_init(&params, &params2, &params3);

  // adjust current fx chain appropriately
  fx_compound_unit_insert_as_parent(prev, reverb_tank);
  prev = reverb_tank;
/* */

/* audio delay */
  params = fx_unit_audio_delay_default();
  FX_compound_unit audio_delay = fx_compound_unit_audio_delay_init(&params);
//  fx_compound_unit_insert_as_parent(prev, audio_delay);
//  prev = audio_delay;
/* */

/* envelope follower */
  params = fx_unit_envelope_follower_default();
  params2 = fx_unit_envelope_follower_audio_filter_default();
  params3 = fx_unit_envelope_follower_audio_detector_default();
  FX_compound_unit envelope_follower = fx_compound_unit_envelope_follower_init(&params, &params2, &params3);
//  fx_compound_unit_insert_as_parent(prev, envelope_follower);
//  prev = envelope_follower;
/* */

/* chorus */
  params = fx_unit_modulated_delay_chorus_default();
  params2 = fx_unit_modulated_delay_audio_delay_chorus_default();
  params3 = fx_unit_modulated_delay_signal_source_chorus_default();
  FX_compound_unit chorus = fx_compound_unit_modulated_delay_init(&params, &params2, &params3);
//  fx_compound_unit_insert_as_parent(prev, chorus);
//  prev = chorus;
/* */

/* vibrato */
  params = fx_unit_modulated_delay_vibrato_default();
  params2 = fx_unit_modulated_delay_audio_delay_vibrato_default();
  params3 = fx_unit_modulated_delay_signal_source_vibrato_default();
  FX_compound_unit vibrato = fx_compound_unit_modulated_delay_init(&params, &params2, &params3);
//  fx_compound_unit_insert_as_parent(prev, vibrato);
//  prev = vibrato;
/* */

/* two_band_shelving_filter */
  params = fx_unit_two_band_shelving_filter_default();
  params.u.two_band_shelving_filter.low_shelf_fc = 400.0;
  params.u.two_band_shelving_filter.low_shelf_boost_cut_db = 3.0;
  params.u.two_band_shelving_filter.high_shelf_fc = 1200.0;
  params.u.two_band_shelving_filter.high_shelf_boost_cut_db = -24.0;
  FX_compound_unit two_band_shelving_filter = fx_compound_unit_two_band_shelving_filter_init(&params);
//  fx_compound_unit_insert_as_parent(prev, two_band_shelving_filter);
//  prev = two_band_shelving_filter;
/* */

/* comb_filter */
  params = fx_unit_comb_filter_default();
  // 3,100 E, 35,100 D
  params.u.comb_filter.delay_ms = 3.5; // lower values have more mid scoop. different values match different notes
  params.u.comb_filter.rt60_ms = 100; // lower has more fundamental, higher has more harmonics
  FX_compound_unit comb_filter = fx_compound_unit_comb_filter_init(&params);
  fx_compound_unit_insert_as_parent(prev, comb_filter);
  prev = comb_filter;
/* */

/* phase_shifter */
  params = fx_unit_phase_shifter_default();
  params2 = fx_unit_phase_shifter_signal_source_default();
  FX_compound_unit phase_shifter = fx_compound_unit_phase_shifter_init(&params, &params2);
//  fx_compound_unit_insert_as_parent(prev, phase_shifter);
//  prev = phase_shifter;
/* */

/* waveshaper */
  params = fx_unit_waveshaper_default();
  FX_compound_unit waveshaper = fx_compound_unit_waveshaper_init(&params);
  fx_compound_unit_insert_as_parent(prev, waveshaper);
  prev = waveshaper;
/* */

/* bitcrusher */
  params = fx_unit_bitcrusher_default();
  params.u.bitcrusher.quantized_bit_depth = 6.0;
  FX_compound_unit bitcrusher = fx_compound_unit_bitcrusher_init(&params);
//  fx_compound_unit_insert_as_parent(prev, bitcrusher);
//  prev = bitcrusher;
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
