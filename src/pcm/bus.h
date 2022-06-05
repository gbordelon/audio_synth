#ifndef BUS_H
#define BUS_H

#include "../lib/macros.h"
#include "channel.h"

typedef struct bus_t {
  Channel channels;
  size_t num_channels;
  FTYPE gain;
} *Bus;

Bus bus_init(size_t num_channels, FTYPE gain);
void bus_cleanup(Bus bus);
Bus busses_init();
void busses_cleanup(Bus busses, size_t num_busses);

#endif
