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

#include "src/map/mmap.h"

int
main()
{
  const double max_amp = 1.0;

  double sample_freq = DEFAULT_SAMPLE_FREQUENCY;
  double tone_freq = 261.626; // middle C
  double dur = 5; // in seconds

  size_t N = sample_freq * dur;
  double scale = max_amp / (double)N;

  Osc car_gen = sin_alloc(tone_freq, sample_freq);
  Osc mod_gen = sin_alloc(3 /*tone_freq * 7.0 / 2.0*/, sample_freq);

  // TODO
  // initialize mixer
  // mixer should be the one who knows about mmap so move that code
  // reading/writing functions for busses and channels
  // use the writing functions here
  // use the reading functions in mixer code

  Mmap_t map = mmap_init();

  BYTE *map_frames = calloc(MMAP_SIZE, sizeof(FTYPE));

  int n, m;
  double amp;
  double sample;
  double samples[NUM_CHANNELS];
  for (n = m = 0; n < N; n++) {
    amp = ((double)n) * scale;
    sample = osc_sample_phase_osc(car_gen, mod_gen);
    samples[0] = amp * sample;
    samples[1] = (max_amp - amp) * sample;

    *(FTYPE *)(map_frames + m * NUM_CHANNELS * sizeof(FTYPE)) = (FTYPE)samples[0];
    *(FTYPE *)(map_frames + (m * NUM_CHANNELS + 1) * sizeof(FTYPE)) = (FTYPE)samples[1];

    if (m++ >= CHUNK_SIZE - 1) {
      while(mmap_write(map, map_frames, MMAP_SIZE * sizeof(FTYPE)) == 0);
      memset(map_frames, 0, MMAP_SIZE * sizeof(FTYPE));
      m = 0;
    }
  }

  mmap_clean(map);
  sin_free(mod_gen);
  sin_free(car_gen);
  return 0;
}
