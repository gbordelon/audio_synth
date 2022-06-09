#ifndef CHANNEL_H
#define CHANNEL_H

#include "../lib/macros.h"

#define BUS_BUFFER_LEN 4
#define CHANNEL_BUFFER_LEN (BUS_BUFFER_LEN*CHUNK_SIZE)

#define channel_ready(c) (c->dirty_map>0)

/*
 * TODO
 * eventually write and read concurrently
 * i only want the mixer to read completed pages
 * this implies some state to indicate pending vs completed reads
 * block on writing if all pages are dirty?
 */

typedef struct channel_t {
  union {
    FTYPE buf[CHANNEL_BUFFER_LEN];
    struct {
      FTYPE buf[CHANNEL_BUFFER_LEN>>2];
      FTYPE q[CHANNEL_BUFFER_LEN>>2];
      FTYPE qq[CHANNEL_BUFFER_LEN>>2];
      FTYPE qqq[CHANNEL_BUFFER_LEN>>2];
    } page_0;
    struct {
      FTYPE q[CHANNEL_BUFFER_LEN>>2];
      FTYPE buf[CHANNEL_BUFFER_LEN>>2];
      FTYPE qq[CHANNEL_BUFFER_LEN>>2];
      FTYPE qqq[CHANNEL_BUFFER_LEN>>2];
    } page_1;
    struct {
      FTYPE q[CHANNEL_BUFFER_LEN>>2];
      FTYPE qq[CHANNEL_BUFFER_LEN>>2];
      FTYPE buf[CHANNEL_BUFFER_LEN>>2];
      FTYPE qqq[CHANNEL_BUFFER_LEN>>2];
    } page_2;
    struct {
      FTYPE q[CHANNEL_BUFFER_LEN>>2];
      FTYPE qq[CHANNEL_BUFFER_LEN>>2];
      FTYPE qqq[CHANNEL_BUFFER_LEN>>2];
      FTYPE buf[CHANNEL_BUFFER_LEN>>2];
    } page_3;
  } u_buf;
  uint8_t dirty_map:4;
  uint8_t page_r:4;
  uint16_t offset_w;
  FTYPE gain;
} *Channel;

Channel channel_init();
void channel_cleanup(Channel chan);

Channel channels_init(size_t num_channels);
void channels_cleanup(Channel chans, size_t num_channels);

size_t channel_write(Channel chan, FTYPE *page);
void channel_write_single(Channel chan, FTYPE sample);
int channel_read(Channel chan, FTYPE *buf_w);

#endif
