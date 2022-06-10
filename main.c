#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include "src/lib/macros.h"

#include "src/env/envelope.h"
#include "src/voice/voice.h"
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
  double note_duration = 2.4; // in seconds

  size_t N = sample_rate * dur;
  size_t M = sample_rate * 0.3;//note_duration;

  Mixer mixer = mixer_init(NULL, 0, 1.0);
  Channel chans = mixer->busses[0].channels;

  Voice voice = voice_init(chans, NUM_CHANNELS);

  int n, m, j = 0;
  for (n = m = 0; n < N; n++, m++) {
    if (m == CHUNK_SIZE) {
      voice_play_chunk(voice);
      mixer_commit(mixer);
      m = 0;
    }
    // TODO user input to change parameters
    if (n % M == 0) {
      voice_play_config(voice, midi_notes[j++], note_duration);
      if (j == 15) {
        j = 0;
      }
    }
  }

  voice_cleanup(voice);
  mixer_cleanup(mixer);
  return 0;
}
