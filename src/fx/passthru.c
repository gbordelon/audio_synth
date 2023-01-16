#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "passthru.h"
#include "fx.h"

extern FX_unit fx_unit_head;

void
fx_unit_passthru_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
  memcpy(dst, src, 3 * sizeof(FTYPE));
#undef src
#undef dst
}

void
fx_unit_passthru_cleanup(FX_unit_state state)
{
  // do nothing
}

void
fx_unit_passthru_set_params(FX_unit_state state, FX_unit_params params)
{
  // do nothing
}

void
fx_unit_passthru_reset(FX_unit_state state, FX_unit_params params)
{
  // do nothing
}

fx_unit_idx
fx_unit_passthru_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_passthru_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_passthru_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_passthru_reset;

  return idx;
}

FX_compound_unit
fx_compound_unit_passthru_init(FX_unit_params params)
{
  FX_compound_unit rv = fx_compound_unit_init(1, 1);
  fx_unit_idx passthru = fx_unit_passthru_init(params);

  rv->units[0] = passthru;
  rv->heads[0] = passthru;
  rv->tail = passthru;

  return rv;
}

fx_unit_params
fx_unit_passthru_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_PASSTHRU;

  return params;
}

