#include <stdlib.h>

#include "../lib/macros.h"
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
  }
  mix->busses = busses;
  mix->num_busses = 1;

  return mix;
}

void
mixer_cleanup(Mixer mix)
{
  if (mix->busses) {
    busses_cleanup(mix->busses, mix->num_busses);
  }
  mixer_free(mix);
}

