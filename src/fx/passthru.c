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
#define bm (fx_unit_head[idx].state.u.passthru.allow)
  if (bm & (FX_PASS_L | FX_PASS_R | FX_PASS_C)) {
    memcpy(dst, src, 3 * sizeof(FTYPE));
  } else {
    if (bm & FX_PASS_L) {
      dst[FX_L] = src[FX_L];
    }

    if (bm & FX_PASS_R) {
      dst[FX_R] = src[FX_R];
    }

    if (bm & FX_PASS_C) {
      dst[FX_C] = src[FX_C];
    }
  }
#undef bm
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
  state->u.passthru.allow = params->u.passthru.allow;
}

void
fx_unit_passthru_reset(FX_unit_state state, FX_unit_params params)
{
  fx_unit_passthru_set_params(state, params);
}

fx_unit_idx
fx_unit_passthru_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_passthru_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_passthru_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_passthru_reset;

  fx_unit_passthru_reset(&fx_unit_head[idx].state, params);

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
  params.u.passthru.allow = FX_PASS_L | FX_PASS_R | FX_PASS_C;

  return params;
}

