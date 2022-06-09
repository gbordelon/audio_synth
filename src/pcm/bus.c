#include <stdlib.h>
#include <string.h>

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

// read the next page from each channel
// apply gain and fx
// TODO don't assume 2 channels for the bus
int
bus_read(Bus bus, FTYPE buf_w[2][CHUNK_SIZE])
{
  static FTYPE read_buf[2][CHUNK_SIZE];
  Channel left = bus->channels;
  Channel right = bus->channels + 1;
  FTYPE *read_buf_l = read_buf[0];
  FTYPE *read_buf_r = read_buf[1];

  FTYPE *write_buf_l = *buf_w;
  FTYPE *write_buf_r = *(buf_w + 1);

  // TODO check for read ready
  channel_read(left, read_buf_l);
  channel_read(right, read_buf_r);

  int i;
  for (i = 0; i < CHUNK_SIZE; i++, write_buf_l++, write_buf_r++, read_buf_l++, read_buf_r++) {
    *write_buf_l = *read_buf_l * bus->gain;
    *write_buf_r = *read_buf_r * bus->gain;
  }

  //memset(read_buf, 0, CHUNK_SIZE * 2 * sizeof(FTYPE));
  return CHUNK_SIZE;
}


