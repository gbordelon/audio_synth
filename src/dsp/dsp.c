#include <stdlib.h>

#include "../lib/macros.h"

#include "dsp.h"
#include "pan.h"

DSP_callback
dsp_alloc()
{
  return calloc(1, sizeof(struct dsp_callback_t));
}

void
dsp_free(DSP_callback cb)
{
  free(cb);
}

void
dsp_mono_noop(FTYPE *L, FTYPE control)
{
  return;
}

DSP_callback
dsp_init()
{
  DSP_callback cb = dsp_alloc();
  cb->next = NULL;
  cb->control_ugen = NULL;
  cb->fn_type = DSP_MONO_L;
  cb->fn_u.mono = dsp_mono_noop;

  return cb;
}

DSP_callback
dsp_init_default()
{
  DSP_callback cb = dsp_init();
  cb->next = NULL;
  cb->control_ugen = ugen_init_constant();
  ugen_set_scale(cb->control_ugen, 0.5, 0.5);
  cb->fn_type = DSP_STEREO;
  cb->fn_u.stereo = stereo_pan;

  return cb;
}

void
dsp_cleanup(DSP_callback head)
{
  for (; head; head = head->next) {
    if (head->control_ugen) {
      ugen_cleanup(head->control_ugen);
    }
    dsp_free(head);
  }
}

void
dsp_add_to_chain(DSP_callback head, DSP_callback new_head)
{
  new_head->next = head;
}

/*
 * TODO take into account functions in the chain which generate control signals for the next stage.
 */
void
stereo_fx_chain(DSP_callback cb, FTYPE *L, FTYPE *R)
{
  FTYPE ctrl;
  for (; cb; cb = cb->next) {
    if (cb->control_ugen) {
      ctrl = ugen_sample(cb->control_ugen);
    } else {
      ctrl = 0.0; // TODO or 1.0?
    }
    switch (cb->fn_type) {
    case DSP_MONO_L:
      cb->fn_u.mono(L, ctrl);
      break;
    case DSP_MONO_R:
      cb->fn_u.mono(R, ctrl);
      break;
    case DSP_STEREO:
      // fallthrough
    default:
      cb->fn_u.stereo(L, R, ctrl);
      break;
    }
  }
}
