#ifndef CHANNEL_H
#define CHANNEL_H

#include <unistd.h>

#include "../lib/macros.h"

#define BUS_BUFFER_LEN 4
#define CHANNEL_BUFFER_LEN (BUS_BUFFER_LEN*CHUNK_SIZE)

typedef struct channel_t {
  FTYPE buf[CHANNEL_BUFFER_LEN];
  uint32_t page_r : 2; // only need 2 bits for 4 bus buffers
  uint32_t page_w : 2; // 2 bits
  uint32_t offset_r : 10; // 10 bits
  uint32_t offset_w : 10; // 10 bits
} *Channel;

Channel channel_init();
void channel_cleanup(Channel chan);

Channel channels_init(size_t num_channels);
void channels_cleanup(Channel chans, size_t num_channels);

#endif
