#include <stdlib.h>

#include "../lib/macros.h"
#include "channel.h"

Channel
channels_alloc(size_t num_channels)
{
  Channel chan = calloc(num_channels, sizeof(struct channel_t));
  return chan;
}

void
channels_free(Channel chans, size_t num_channels)
{
  free(chans);
}

Channel
channels_init(size_t num_channels)
{
  Channel c, chans = channels_alloc(num_channels);

  for(c = chans; c - chans < num_channels; c++) {
    c->page_r = c->page_w = c->offset_r = c->offset_w = 0;
  }

  return chans;
}

void
channels_cleanup(Channel chans, size_t num_channels)
{
  channels_free(chans, num_channels);
}

Channel
channel_init()
{
  return channels_init(1);
}

void
channel_cleanup(Channel chan)
{
  channels_cleanup(chan, 1);
}
