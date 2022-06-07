#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include "src/pcm/pcm.h"
#include "src/osc/osc.h"
#include "src/osc/saw.h"
#include "src/osc/sin.h"
#include "src/osc/squ.h"
#include "src/osc/tri.h"
#include "src/lib/macros.h"

#include "src/pcm/mixer.h"

int
main()
{
  const double max_amp = 1.0;

  double sample_freq = DEFAULT_SAMPLE_FREQUENCY;
  double tone_freq = 261.626; // middle C
  double dur = 5; // in seconds

  size_t N = sample_freq * dur;
  double scale = max_amp / (double)N;

  Mixer mixer = mixer_init(NULL, 0, 1.0);
  mixer->gain = 1.0;

  Osc car_gen = sin_alloc(tone_freq, sample_freq);
  Osc mod_gen = sin_alloc(3 /*tone_freq * 7.0 / 2.0*/, sample_freq);

  Channel left = &mixer->busses[0].channels[0];
  Channel right = &mixer->busses[0].channels[1];

  int n, m;
  double amp;
  double sample;
  double samples[NUM_CHANNELS];
  for (n = m = 0; n < N; n++) {
    amp = ((double)n) * scale;
    sample = osc_sample_phase_osc(car_gen, mod_gen);
    samples[0] = amp * sample;
    samples[1] = (max_amp - amp) * sample;

    channel_write(left, samples[0]);
    channel_write(right, samples[1]);

    if (m++ == CHUNK_SIZE) {
      mixer_commit(mixer);
      m = 0;
    }
  }

  sin_free(mod_gen);
  sin_free(car_gen);
  mixer_cleanup(mixer);
  return 0;
}
