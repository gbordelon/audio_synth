#ifndef FX_BUFFER_H
#define FX_BUFFER_H

#include <stdbool.h>

#include "../lib/macros.h"

/*
 * collect CHUNK_SIZE interleaved stereo frames
 */
typedef struct fx_unit_buffer_state_t {
  FTYPE buf[NUM_CHANNELS * CHUNK_SIZE];
  bool buffer_output; // output buffer or input buffer
  size_t read_idx; // reset when buffer is filled by audio driver
  size_t write_idx; // reset when buffer is read by audio driver
} fx_unit_buffer_state;

typedef fx_unit_buffer_state *FX_unit_buffer_state;

typedef struct fx_unit_buffer_params_t {
  bool buffer_output; // alternative is buffer input
} fx_unit_buffer_params;

typedef struct fx_unit_buffer_params_t *FX_unit_buffer_params;

// forward decl
typedef struct fx_unit_params_t fx_unit_params;
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_buffer_init(FX_unit_params params);
void fx_unit_buffer_read_chunk(fx_unit_idx idx, FTYPE buf[NUM_CHANNELS * CHUNK_SIZE]);
void fx_unit_buffer_write_chunk(fx_unit_idx idx, FTYPE buf[NUM_CHANNELS * CHUNK_SIZE], size_t write_idx);

fx_unit_params fx_unit_buffer_default();

#endif
