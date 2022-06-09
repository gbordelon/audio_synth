#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include "src/lib/macros.h"

#include "src/env/envelope.h"
#include "src/instrument/instrument.h"
#include "src/osc/imp.h"
#include "src/osc/osc.h"
#include "src/osc/saw.h"
#include "src/osc/sin.h"
#include "src/osc/squ.h"
#include "src/osc/tri.h"
#include "src/pcm/mixer.h"
#include "src/pcm/pcm.h"
#include "src/midi/midi.h"


int
main()
{
  uint8_t midi_notes[15] = {45, 49, 47, 50, 49, 52, 50, 54, 52, 56, 54, 57, 56, 59, 57};
  double sample_rate = DEFAULT_SAMPLE_RATE;
  double dur = 4.8; // in seconds
  double note_duration = 0.3; // in seconds

  size_t N = sample_rate * dur;

  Mixer mixer = mixer_init(NULL, 0, 1.0);
  Channel chans = mixer->busses[0].channels;

  Instrument instr = instrument_init(chans, NUM_CHANNELS);
  instrument_play_config(instr, midi_notes[0], note_duration);

  instrument_play_chunk(instr);

  int n, m, j = 1;
  for (n = m = 0; n < N; n++, m++) {
    if (m == CHUNK_SIZE) {
      instrument_play_chunk(instr);
      mixer_commit(mixer);
      m = 0;
      // TODO user input to change parameters
      if (!instrument_playing(instr)) {
        instrument_play_config(instr, midi_notes[j++], note_duration);
        if (j == 15) {
          j = 0;
        }
      }
    }

  }

  instrument_cleanup(instr);
  mixer_cleanup(mixer);
  return 0;
}
