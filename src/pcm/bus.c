#include <stdlib.h>

#include "../lib/macros.h"
#include "bus.h"

Bus
busses_alloc(size_t n)
{
  Bus bus = calloc(n, sizeof(struct bus_t));
  return bus;
}

Bus
bus_alloc()
{
  return busses_alloc(1);
}

void
bus_free(Bus bus)
{
  free(bus);
}

void
_bus_init(Bus bus, size_t num_channels, FTYPE gain)
{
  bus->num_channels = num_channels;
  bus->gain = gain;
  bus->channels = channels_init(num_channels);
}

Bus
bus_init(size_t num_channels, FTYPE gain)
{
  Bus bus = bus_alloc();
  _bus_init(bus, num_channels, gain);
  return bus;
}

void
_bus_cleanup(Bus bus)
{
  channels_cleanup(bus->channels, bus->num_channels);
}

void
bus_cleanup(Bus bus)
{
  _bus_cleanup(bus);
  bus_free(bus);
}

Bus
_busses_init(size_t num_busses, size_t *num_channels_per_bus, FTYPE *gain_per_bus)
{
  Bus busses = busses_alloc(num_busses);

  Bus b;
  for (b = busses; (b - busses) < num_busses; b++, num_channels_per_bus++, gain_per_bus++) {
    _bus_init(b, *num_channels_per_bus, *gain_per_bus);
  }

  return busses;
}

Bus
busses_init()
{
  size_t num_channels = NUM_CHANNELS;
  FTYPE gain = 1.0;
  return _busses_init(1, &num_channels, &gain);
}

void
busses_cleanup(Bus busses, size_t num_busses)
{
  Bus b;
  for (b = busses; b - busses < num_busses; b++) {
    _bus_cleanup(b);
  }

  free(busses);
}
