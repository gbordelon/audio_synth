#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"
#include "channel.h"

#define page_01_boundary (CHANNEL_BUFFER_LEN>>2)
#define page_12_boundary (CHANNEL_BUFFER_LEN>>1)
#define page_23_boundary (page_01_boundary|page_12_boundary)
#define page_30_boundary CHANNEL_BUFFER_LEN

#define dirty_page_0 0x01
#define dirty_page_1 0x02
#define dirty_page_2 0x04
#define dirty_page_3 0x08

#define clean_page_0(c) ((c)->dirty_map=(c)->dirty_map&0xfe)
#define clean_page_1(c) ((c)->dirty_map=(c)->dirty_map&0xfd)
#define clean_page_2(c) ((c)->dirty_map=(c)->dirty_map&0xfb)
#define clean_page_3(c) ((c)->dirty_map=(c)->dirty_map&0xf7)

#define page_0_dirty(c) ((c)->dirty_map&dirty_page_0)
#define page_1_dirty(c) ((c)->dirty_map&dirty_page_1)
#define page_2_dirty(c) ((c)->dirty_map&dirty_page_2)
#define page_3_dirty(c) ((c)->dirty_map&dirty_page_3)

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
    // calloc should set everything 0 including the buf array
    //c->offset_r = c->page_r = c->dirty_map = 0;
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

int
channel_read(Channel chan, FTYPE *buf_w)
{
  FTYPE *buf_r;
  if (!channel_ready(chan)) {
    return 0;
  }

  buf_r = NULL;
  if (chan->page_r == 0 && page_0_dirty(chan)) {
    buf_r = chan->u_buf.page_0.buf;
    clean_page_0(chan);
  } else if (chan->page_r == 1 && page_1_dirty(chan)) {
    buf_r = chan->u_buf.page_1.buf;
    clean_page_1(chan);
  } else if (chan->page_r == 2 && page_2_dirty(chan)) {
    buf_r = chan->u_buf.page_2.buf;
    clean_page_2(chan);
  } else if (chan->page_r == 3 && page_3_dirty(chan)) {
    buf_r = chan->u_buf.page_3.buf;
    clean_page_3(chan);
  }

  if (buf_r) {
    memmove(buf_w, buf_r, CHUNK_SIZE * sizeof(FTYPE));
    memset(buf_r, 0, CHUNK_SIZE * sizeof(FTYPE));
    chan->page_r++;
    if (chan->page_r == BUS_BUFFER_LEN) {
      chan->page_r = 0;
    }
    return CHUNK_SIZE;
  }

  return -1;
}

/*
 * eventually write and read concurrently
 * i only want the mixer to read completed pages
 * this implies some state to indicate pending vs completed reads
 * block on writing if all pages are dirty?
 */
// TODO write_page function. more complex arithmetic with wrapping to consider
void
channel_write(Channel chan, FTYPE sample)
{
  // TODO move this write into the predicated section after i have a way to force the mixer to read a page or have a callback mechanism
  // TODO be sure to adjust the predicates, which right now are based on the offset_w being incremented beforehand

  if ((chan->offset_w == 0 && (chan->dirty_map & dirty_page_0))
  ||  (chan->offset_w == page_01_boundary && (chan->dirty_map & dirty_page_1)) 
  ||  (chan->offset_w == page_12_boundary && (chan->dirty_map & dirty_page_2)) 
  ||  (chan->offset_w == page_23_boundary && (chan->dirty_map & dirty_page_3))) {
    return;
  }

  if (chan->offset_w == 0) {
    chan->dirty_map |= dirty_page_0;
  } else if (chan->offset_w == page_01_boundary) {
    chan->dirty_map |= dirty_page_1;
  } else if (chan->offset_w == page_12_boundary) {
    chan->dirty_map |= dirty_page_2;
  } else if (chan->offset_w == page_23_boundary) {
    chan->dirty_map |= dirty_page_3;
  }

  chan->u_buf.buf[chan->offset_w++] = sample;

  if (chan->offset_w == page_30_boundary) {
      chan->offset_w = 0;
  }
}
