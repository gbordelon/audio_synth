#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"

#include "buffer.h"
#include "fx.h"

extern FX_unit fx_unit_head;

#include <stdio.h>
/*
 * Read from the chunk buffer into the caller's buffer
 */
void
fx_unit_buffer_read_chunk(fx_unit_idx idx, FTYPE buf[2 * CHUNK_SIZE])
{
  memcpy(buf, fx_unit_head[idx].state.u.buffer.buf, 2 * CHUNK_SIZE * sizeof(FTYPE));
  memset(fx_unit_head[idx].state.u.buffer.buf, 0, 2 * CHUNK_SIZE * sizeof(FTYPE));
  fx_unit_head[idx].state.u.buffer.read_idx = 0;
}

/*
 * Write to the chunk buffer from the caller's buffer
 * interleaved stereo frames
 */
void
fx_unit_buffer_write_chunk(fx_unit_idx idx, FTYPE buf[NUM_CHANNELS * CHUNK_SIZE], size_t write_idx)
{
#define read_idx (fx_unit_head[idx].state.u.buffer.read_idx)
  size_t cpy_amt = NUM_CHANNELS * CHUNK_SIZE;
  memcpy(fx_unit_head[idx].state.u.buffer.buf, buf + read_idx, cpy_amt * sizeof(FTYPE));
  read_idx += cpy_amt;
  if (read_idx >= CHUNK_SIZE * NUM_CHANNELS) {
    read_idx = 0;
    fx_unit_head[idx].state.u.buffer.write_idx = 0;
  }
#undef read_idx
}

/*
 * processing a frame should copy from the chunk buffer into the output buffer
 */
void
fx_unit_buffer_process_read(fx_unit_idx idx)
{
#define write_idx (fx_unit_head[idx].state.u.buffer.write_idx)
#define read_idx (fx_unit_head[idx].state.u.buffer.read_idx)
  if (read_idx == 0) {
#undef read_idx
    memcpy(fx_unit_head[idx].output_buffer.lrc, &fx_unit_head[idx].state.u.buffer.buf[write_idx], 2 * sizeof(FTYPE));
    write_idx += 2;
#undef write_idx
    fx_unit_head[idx].output_buffer.dirty = true;
  }
}

/*
 * processing a frame should copy the parent frame into the output buffer and the chunk buffer
 */
void
fx_unit_buffer_process_write(fx_unit_idx idx)
{
  FX_unit parent = &fx_unit_head[fx_unit_head[idx].parents[0]];
  memcpy(fx_unit_head[idx].output_buffer.lrc, parent->output_buffer.lrc, 2 * sizeof(FTYPE));
  fx_unit_head[idx].output_buffer.dirty = true;
#define read_idx (fx_unit_head[idx].state.u.buffer.read_idx)
  memcpy(&fx_unit_head[idx].state.u.buffer.buf[read_idx], parent->output_buffer.lrc, 2 * sizeof(FTYPE));
  read_idx += 2;
#undef read_idx
}

void
fx_unit_buffer_process_frame(fx_unit_idx idx)
{
  // switch on fx_unit_head[idx].state.u.buffer.buffer_output
  if (fx_unit_head[idx].state.u.buffer.buffer_output) {
    fx_unit_buffer_process_write(idx);
  } else {
    fx_unit_buffer_process_read(idx);
  } 
}

void
fx_unit_buffer_cleanup(FX_unit_state state)
{
  // do nothing
}

void
fx_unit_buffer_set_params(FX_unit_state state, FX_unit_params params)
{
  state->u.buffer.buffer_output = params->u.buffer.buffer_output;
}

void
fx_unit_buffer_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_buffer_set_params(state, params);
}

fx_unit_idx
fx_unit_buffer_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_buffer_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_buffer_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_buffer_reset;

  fx_unit_buffer_reset(&fx_unit_head[idx].state, params);
  return idx;
}

fx_unit_params
fx_unit_buffer_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_BUFFER;
  params.u.buffer.buffer_output = false;

  return params;
}
