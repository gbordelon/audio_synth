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

FTYPE
dsp_mono_noop(FTYPE *L, dsp_state *state, FTYPE control)
{
  return control;
}

DSP_callback
dsp_init()
{
  DSP_callback cb = dsp_alloc();
  cb->next = NULL;
  cb->control = DSP_CONTROL_NONE;
  cb->fn_type = DSP_MONO_L;
  cb->fn_u.mono = dsp_mono_noop;

  return cb;
}

DSP_callback
dsp_init_default()
{
  DSP_callback cb = dsp_init();
  cb->control = DSP_CONTROL_UGEN;
  cb->ctrl_u.ugen = ugen_init_constant();
  ugen_set_scale(cb->ctrl_u.ugen, 0.5, 0.5);
  cb->fn_type = DSP_STEREO;
  cb->fn_u.stereo = stereo_pan;

  return cb;
}

void
dsp_cleanup(DSP_callback head)
{
  for (; head; head = head->next) {
    if (head->control == DSP_CONTROL_UGEN) {
      ugen_cleanup(head->ctrl_u.ugen);
    } else if (head->control == DSP_CONTROL_CALLBACK) {
      dsp_cleanup(head->ctrl_u.dsp);
    }
    dsp_free(head);
  }
}

DSP_callback
dsp_add_to_chain(DSP_callback head, DSP_callback new_head)
{
  new_head->next = head;
  return new_head;
}

void
dsp_set_control_ugen(DSP_callback cb, Ugen ugen)
{
  if (cb->control == DSP_CONTROL_UGEN) {
    ugen_cleanup(cb->ctrl_u.ugen);
  } else if (cb->control == DSP_CONTROL_CALLBACK) {
    dsp_cleanup(cb->ctrl_u.dsp);
  }
  cb->control = DSP_CONTROL_UGEN;
  cb->ctrl_u.ugen = ugen;
}

void
dsp_set_control_dsp(DSP_callback cb, DSP_callback ctrl)
{
  if (cb->control == DSP_CONTROL_UGEN) {
    ugen_cleanup(cb->ctrl_u.ugen);
  } else if (cb->control == DSP_CONTROL_CALLBACK) {
    dsp_cleanup(cb->ctrl_u.dsp);
  }
  cb->control = DSP_CONTROL_CALLBACK;
  cb->ctrl_u.dsp = ctrl;
}

/*
 * TODO take into account functions in the chain which generate control signals for the next stage.
 * probably have a return value;
 */
FTYPE
stereo_fx_chain(DSP_callback cb, FTYPE *L, FTYPE *R)
{
  FTYPE ctrl = 0.0;

  for (; cb; cb = cb->next) {
    if (cb->control == DSP_CONTROL_CALLBACK) {
      FTYPE Lc = *L, Rc = *R;
      ctrl = stereo_fx_chain(cb->ctrl_u.dsp, &Lc, &Rc);
    } else if (cb->control == DSP_CONTROL_UGEN) {
      ctrl = ugen_sample(cb->ctrl_u.ugen);
    }
    switch (cb->fn_type) {
    case DSP_MONO_L:
      ctrl = cb->fn_u.mono(L, &cb->state, ctrl);
      break;
    case DSP_MONO_R:
      ctrl = cb->fn_u.mono(R, &cb->state, ctrl);
      break;
    case DSP_STEREO:
      // fallthrough
    default:
      ctrl = cb->fn_u.stereo(L, R, &cb->state, ctrl);
      break;
    }
  }

  return ctrl;
}
