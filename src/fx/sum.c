#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "sum.h"
#include "fx.h"

#include <math.h>
extern FX_unit fx_unit_head;

void
fx_unit_sum_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src0 (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define src1 (fx_unit_head[fx_unit_head[idx].parents[1]].output_buffer.lrc)
#define src2 (fx_unit_head[fx_unit_head[idx].parents[2]].output_buffer.lrc)
#define mix (fx_unit_head[idx].state.u.sum.mix)
  if (fx_unit_head[idx].num_parents > 2) {
    dst[FX_L] = src0[FX_L] * mix[0] + src1[FX_L] * mix[1] + src2[FX_L] * mix[2];
    dst[FX_R] = src0[FX_R] * mix[0] + src1[FX_R] * mix[1] + src2[FX_R] * mix[2];
  } else if (fx_unit_head[idx].num_parents > 1) {
    dst[FX_L] = src0[FX_L] * mix[0] + src1[FX_L] * mix[1];
    dst[FX_R] = src0[FX_R] * mix[0] + src1[FX_R] * mix[1];
  } else {
    memcpy(dst, src0, 2 * sizeof(FTYPE));
  }
  if (fabs(dst[FX_L]) > 1.0) dst[FX_L] = 0;
  if (fabs(dst[FX_R]) > 1.0) dst[FX_R] = 0;
  dst[FX_C] = src0[FX_C];
#undef mix
#undef src2
#undef src1
#undef src0
#undef dst
}

void
fx_unit_sum_cleanup(FX_unit_state state)
{
  // do nothing
}

void
fx_unit_sum_set_params(FX_unit_state state, FX_unit_params params)
{
  if (state->u.sum.mix != NULL) {
    free(state->u.sum.mix);
  }
  state->u.sum.mix = params->u.sum.mix;
}

void
fx_unit_sum_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_sum_set_params(state, params);
}

fx_unit_idx
fx_unit_sum_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_sum_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_sum_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_sum_reset;

  fx_unit_sum_reset(&fx_unit_head[idx].state, params);
  return idx;
}

FX_compound_unit
fx_compound_unit_sum_init(FX_unit_params sum_p)
{
  FX_compound_unit rv = fx_compound_unit_init(1, 1);
  // summer
  fx_unit_idx sum = fx_unit_sum_init(sum_p);

  rv->units[0] = sum;
  rv->heads[0] = sum;
  rv->tail = sum;

  return rv;
}

fx_unit_params
fx_unit_sum_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_SUM;
  params.u.sum.mix = calloc(1, sizeof(FTYPE));

  return params;
}

void
fx_unit_sum_mix_set(fx_unit_idx idx, FTYPE *mix)
{
  if (fx_unit_head[idx].state.u.sum.mix != NULL) {
    free(fx_unit_head[idx].state.u.sum.mix);
  }
  fx_unit_head[idx].state.u.sum.mix = mix;
//realloc(mix, fx_unit_head[idx].num_parents, sizeof(FTYPE));
}
