#ifndef MIXER_H
#define MIXER_H

#include <stdbool.h>

#include "../lib/macros.h"
#include "../map/mmap.h"

#include "bus.h"

typedef struct mixer_t {
  Bus busses;
  size_t num_busses;
  FTYPE gain;
  Mmap_t map;
  FTYPE *write_buf;
  bool needs_write;
} *Mixer;

Mixer mixer_init(Bus busses, size_t num_busses, FTYPE gain);
void mixer_cleanup(Mixer mix);
void mixer_update(Mixer mix);
//void mixer_commit(Mixer mix);

#endif
