#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"
#include "../map/mmap.h"

#include "mixer.h"
#include "bus.h"

/*
 * Currently the rendering of audio only supports 32-bit floating points.
 * There are some references to 'float' instead of 'FTYPE' in this file to reflect that.
 */
Mixer
mixer_alloc(FTYPE gain)
{
  Mixer mix = calloc(1, sizeof(struct mixer_t));
  return mix;
}

void
mixer_free(Mixer mix)
{
  free(mix);
}

Mixer
mixer_init(Bus busses, size_t num_busses, FTYPE gain)
{
  Mixer mix = mixer_alloc(gain);
  if (!num_busses) {
    busses = bus_default_init();
    num_busses = 1;
  }
  mix->busses = busses;
  mix->num_busses = num_busses;
  mix->gain = 1.0;

  mix->map = mmap_init();
  mix->write_buf = calloc(MMAP_SIZE, sizeof(FTYPE));
  mix->needs_write = false;

  return mix;
}

void
mixer_cleanup(Mixer mix)
{
  if (mix->busses) {
    busses_cleanup(mix->busses, mix->num_busses);
  }
  mmap_clean(mix->map);
  free(mix->write_buf);
  mixer_free(mix);
}

// interleave samples to create stereo frames
void
mixer_update(Mixer mix)
{
  static FTYPE read_buf[NUM_CHANNELS][CHUNK_SIZE] = {0};
  int i,j, rv;
  Bus bus;
  FTYPE *read_buf_l, *read_buf_r;
  FTYPE *write_buf_l, *write_buf_r;

  for (j = 0, bus = mix->busses; j < mix->num_busses; j++, bus++) {
    // TODO check for read failure
    rv = bus_read(bus, read_buf);

    if (rv > 0) {
      for (i = 0, read_buf_l = read_buf[0], read_buf_r = read_buf[1], write_buf_l = mix->write_buf, write_buf_r = write_buf_l + 1;
           i < CHUNK_SIZE;
           i++, write_buf_l+=NUM_CHANNELS, write_buf_r+=NUM_CHANNELS, read_buf_l++, read_buf_r++) {
        *write_buf_l += *read_buf_l * mix->gain;
        *write_buf_r += *read_buf_r * mix->gain;
      }
      memset(read_buf, 0, CHUNK_SIZE * NUM_CHANNELS * sizeof(FTYPE));
    } else {
    }
  }
  // TODO scale or clip amplitudes for values outside the range [-1.0,1.0]
}
