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

void
mixer_commit(Mixer mix)
{
  while(mmap_write(mix->map, mix->write_buf, MMAP_SIZE * sizeof(FTYPE)) == 0);
  memset(mix->write_buf, 0, MMAP_SIZE * sizeof(FTYPE));
}
