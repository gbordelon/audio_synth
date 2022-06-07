#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"
#include "../map/mmap.h"

#include "mixer.h"
#include "bus.h"

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
    busses = busses_init();
    num_busses = 1;
  }
  mix->busses = busses;
  mix->num_busses = num_busses;
  mix->gain = 1.0;

  mix->map = mmap_init();
  mix->write_buf = calloc(MMAP_SIZE, sizeof(FTYPE));

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

// TODO operate on busses, not channels here
// let the bus code operate on channels
void
mixer_update(Mixer mix)
{
  Channel left = &mix->busses[0].channels[0];
  Channel right = &mix->busses[0].channels[1];
  FTYPE buf_l[CHUNK_SIZE] = {0};
  FTYPE buf_r[CHUNK_SIZE] = {0};
  FTYPE *write_buf_l = (FTYPE *)mix->write_buf;
  FTYPE *write_buf_r = write_buf_l + 1;
  FTYPE *read_buf_l = buf_l;
  FTYPE *read_buf_r = buf_r;

  // TODO check for read ready
  channel_read(left, buf_l);
  channel_read(right, buf_r);

  int i;
  for (i = 0; i < CHUNK_SIZE; i++, write_buf_l+=2, write_buf_r+=2, read_buf_l++, read_buf_r++) {
    *write_buf_l = *read_buf_l * mix->gain;
    *write_buf_r = *read_buf_r * mix->gain;
  }
}

void
mixer_commit(Mixer mix)
{
  mixer_update(mix);
  while(mmap_write(mix->map, mix->write_buf, MMAP_SIZE * sizeof(FTYPE)) == 0);
  memset(mix->write_buf, 0, MMAP_SIZE * sizeof(FTYPE));
}
