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
  Osc mod_gen = sin_alloc(tone_freq * 7.0 / 2.0, sample_freq);

  Mmap_t map = mmap_init();


  int m;
  BYTE *map_frames = calloc(4096 * 2, sizeof(double));

  int n;
  double amp;
  double sample;
  double samples[2];
  for (n = m = 0; n < N; n++) {
    amp = ((double)n) * scale;
    sample = osc_sample_phase_osc(car_gen, mod_gen);
    samples[0] = amp * sample;
    samples[1] = (max_amp - amp) * sample;

    *(FTYPE *)(map_frames + m * 2 * sizeof(FTYPE)) = (FTYPE)samples[0];
    *(FTYPE *)(map_frames + (m * 2 + 1) * sizeof(FTYPE)) = (FTYPE)samples[1];

    if (m++ >= 4095) {
      while(mmap_write(map, map_frames, 4096 * 2 * sizeof(FTYPE)) == 0);
      memset(map_frames, 0, 4096 * 2 * sizeof(FTYPE));
      m = 0;
    }
  }

  mmap_clean(map);
  sin_free(mod_gen);
  sin_free(car_gen);
  return 0;
}
