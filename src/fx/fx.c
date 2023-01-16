#include <stdlib.h>
#include <string.h>

#include "../lib/macros.h"

#include "buffer.h"
#include "pan.h"

#include "fx.h"

FX_unit fx_unit_head = NULL;
static fx_unit_idx fx_unit_count = 0;
static size_t fx_unit_max = 16;

void
fx_units_free()
{
  free(fx_unit_head);
}

void
fx_unit_library_cleanup()
{
  fx_unit_idx idx;
  for (idx = 0; idx < fx_unit_count; idx++) {
    fx_unit_cleanup(idx);
  }
  free(fx_unit_head);
}

fx_unit_idx
fx_unit_init()
{
  if (fx_unit_head == NULL) {
    struct fx_unit_params_t params = {0};
    params.sample_rate = DEFAULT_SAMPLE_RATE;
    params.t = FX_UNIT_BUFFER;
    params.u.buffer.buffer_output = false;

    fx_unit_head = calloc(fx_unit_max, sizeof(struct fx_unit_t));
    
    fx_unit_buffer_init(&params); // should recursively call fx_unit_init() to increase count
    params.u.buffer.buffer_output = true;

    fx_unit_buffer_init(&params); // should recursively call fx_unit_init() to increase count
    fx_unit_parent_ref_add(1, 0);
  } 
  if (fx_unit_count >= fx_unit_max) {
    fx_unit_max *= 2;
    fx_unit_head = realloc(fx_unit_head, fx_unit_max * sizeof(struct fx_unit_t));
    memset(&fx_unit_head[fx_unit_count], 0, (fx_unit_max - fx_unit_count) * sizeof(struct fx_unit_t));
  }
  // parents is NULL 
  // output buffer is {0}
  // state is null

  return fx_unit_count++;
}

void
fx_unit_parents_ref_cleanup(fx_unit_idx idx)
{
  if (fx_unit_head[idx].parents != NULL) {
    free(fx_unit_head[idx].parents);
    fx_unit_head[idx].parents = NULL;
    fx_unit_head[idx].num_parents = 0;
  }
}

void
fx_unit_cleanup(fx_unit_idx idx)
{
  // cleanup state from fx_unit_head[idx]
  fx_unit_head[idx].state.f.cleanup(&fx_unit_head[idx].state);
  fx_unit_head[idx].state.t = FX_UNIT_UNUSED;

  fx_unit_parents_ref_cleanup(idx);

  // iterate over all fx units
  fx_unit_idx idx2, par_idx;
  for (idx2 = 0; idx2 < fx_unit_count; idx2++) {
    // iterate over all parents for this fx unit
    for (par_idx = 0; par_idx < fx_unit_head[idx2].num_parents; par_idx++) {
      // if an fx unit parent list refers to the cleared fx unit
      if (fx_unit_head[idx2].parents[par_idx] == idx) {
        // then clear the reference
        fx_unit_head[idx2].parents[par_idx] = FX_UNIT_IDX_NONE;
      }
    }
  }
}

void
fx_unit_parent_ref_add(fx_unit_idx idx, fx_unit_idx parent_idx)
{
  fx_unit_head[idx].parents = realloc(
      fx_unit_head[idx].parents,
      (++fx_unit_head[idx].num_parents) * sizeof(fx_unit_idx));

  fx_unit_head[idx].parents[fx_unit_head[idx].num_parents - 1] = parent_idx;
}

void
fx_unit_insert_as_parent(fx_unit_idx idx, FX_compound_unit parent)
{
  int i, j;
  for (i = 0; i < parent->num_heads; i++) {
    fx_unit_parents_ref_cleanup(parent->heads[i]);
    for (j = 0; j < fx_unit_head[idx].num_parents; j++) {
      fx_unit_parent_ref_add(parent->heads[i], fx_unit_head[idx].parents[j]);
    }
  }

  fx_unit_parents_ref_cleanup(idx);
  fx_unit_parent_ref_add(idx, parent->tail);
}

void
fx_unit_reset_output_buffers()
{
  fx_unit_idx idx;
  for (idx = 0; idx < fx_unit_count; idx++) {
    fx_unit_head[idx].output_buffer.dirty = false;
  }
}

void
fx_unit_process_frame(fx_unit_idx idx)
{
  if (idx == FX_UNIT_IDX_NONE || fx_unit_head[idx].state.t == FX_UNIT_UNUSED) {
    return;
  }

  fx_unit_idx parent_idx;
#define parent fx_unit_head[fx_unit_head[idx].parents[parent_idx]] 
  for (parent_idx = 0; parent_idx < fx_unit_head[idx].num_parents; parent_idx++) {
    if (!parent.output_buffer.dirty) {
      fx_unit_process_frame(fx_unit_head[idx].parents[parent_idx]);
    }
  }
#undef parent
  // call function determined by fx_unit_head[idx].state
  // using parent unit output buffers
  // to populate fx_unit_head[idx].output_buffer
  fx_unit_head[idx].state.f.process_frame(idx);
}

/*
 * call this fn CHUNK_SIZE times to populate an output buffer for the audio driver
 * output buffer should be CHUNK_SIZE stereo frames
 */
void
fx_unit_entry_point(FTYPE rv[2], fx_unit_idx head)
{
  fx_unit_process_frame(head);
  rv[FX_L] = fx_unit_head[head].output_buffer.lrc[FX_L];
  rv[FX_R] = fx_unit_head[head].output_buffer.lrc[FX_R];
  fx_unit_reset_output_buffers();
}

FX_compound_unit
fx_compound_unit_alloc()
{
  return calloc(1, sizeof(fx_compound_unit));
}

void
fx_compound_unit_cleanup(FX_compound_unit unit)
{
  int i;
  for (i = 0; i < unit->num_units; i++) {
    fx_unit_cleanup(unit->units[i]);
  }
  free(unit->units);
  free(unit->heads);
  free(unit);
}

FX_compound_unit
fx_compound_unit_init(size_t num_units, size_t num_heads)
{
  FX_compound_unit unit = fx_compound_unit_alloc();
  unit->units = calloc(num_units, sizeof(fx_unit_idx));
  unit->heads = calloc(num_heads, sizeof(fx_unit_idx));
  unit->num_units = num_units;
  unit->num_heads = num_heads;

  return unit;
}

void
fx_compound_unit_insert_as_parent(FX_compound_unit unit, FX_compound_unit parent)
{
  int i, j;
  for (i = 0; i < parent->num_heads; i++) {
    fx_unit_parents_ref_cleanup(parent->heads[i]);
    for (j = 0; j < fx_unit_head[unit->heads[0]].num_parents; j++) {
      fx_unit_parent_ref_add(parent->heads[i], fx_unit_head[unit->heads[0]].parents[j]);
    }
  }
  for (i = 0; i < unit->num_heads; i++) {
    fx_unit_parents_ref_cleanup(unit->heads[i]);
    fx_unit_parent_ref_add(unit->heads[i], parent->tail);
  }
}

void
fx_compound_unit_parent_ref_add(FX_compound_unit unit, fx_unit_idx parent_idx)
{
  int i;
  for (i = 0; i < unit->num_heads; i++) {
    fx_unit_parent_ref_add(unit->heads[i], parent_idx);
  }
}
