#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include "src/pcm/pcm.h"
#include "src/wav/wav.h"
#include "src/osc/osc.h"
#include "src/osc/saw.h"
#include "src/osc/sin.h"
#include "src/osc/squ.h"
#include "src/osc/tri.h"
#include "src/lib/macros.h"

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
  Osc mod_gen = sin_alloc(tone_freq, sample_freq);
  Riff_chunk rc = riff_alloc(3, 2); // floats and stereo
  BYTE *frames = riff_alloc_frames(rc, N);

  int n;
  double amp;
  double sample;
  float samples[2];
  for (n = 0; n < N; n++) {
    amp = ((double)n) * scale;
    sample = osc_sample_phase_osc(car_gen, mod_gen);
    //sample = osc_sample(car_gen);
    samples[0] = (float)(amp * sample);
    samples[1] = (float)((max_amp - amp) * sample);
    riff_set_frame(rc, frames, n, samples);
  }

  riff_append_frames(rc, frames, n);
  riff_write_wav_file(rc, "/Users/guy.bordelon/sandbox/c_projects/audio_synth/test.wav");

  riff_free_frames(frames);
  riff_free(rc);
  sin_free(mod_gen);
  sin_free(car_gen);
  return 0;
}
