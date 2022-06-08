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
  const double max_amp = 1.0;

  size_t midi_note = 45;
  double sample_rate = DEFAULT_SAMPLE_RATE;
  double tone_freq = midi_note_to_freq_table[midi_note]; // middle C
  double dur = 5; // in seconds
  double note_duration = 0.3; // in seconds

  size_t N = sample_rate * dur;
  double scale = max_amp / (double)N;

  Mixer mixer = mixer_init(NULL, 0, 1.0);
  Channel chans = mixer->busses[0].channels;

  Instrument instr = instrument_init(chans, NUM_CHANNELS);
  instrument_play_config(instr, midi_note, note_duration);

  int n, m;
  double amp;
  double sample;
  double samples[NUM_CHANNELS];
  double e_sample;
  for (n = m = 0; n < N; n++) {
    amp = ((double)n) * scale;
    instrument_play_resume(instr);
    if (m++ == CHUNK_SIZE) {
      mixer_commit(mixer);
      m = 0;
      if (!instrument_playing(instr)) {
        instrument_play_config(instr, ++midi_note, note_duration);
        if (midi_note >= 128) {
          midi_note = 21;
        }
      }
    }

  }

  instrument_cleanup(instr);
  mixer_cleanup(mixer);
  return 0;
}
