#ifndef DSP_H
#define DSP_H

#include "../lib/macros.h"

#include "../ugen/ugen.h"

typedef enum {
  DSP_MONO_L,
  DSP_MONO_R,
  DSP_STEREO
} dsp_fn_type;

typedef union {
  struct {
    FTYPE quantized_bit_depth;
  } bitcrusher;
} dsp_state;

// sample pointer, control
typedef void (*dsp_mono_fn)(FTYPE *, dsp_state *, FTYPE);

// sample pointer, sample pointer, control
typedef void (*dsp_stereo_fn)(FTYPE *, FTYPE *, dsp_state *, FTYPE);

/*
 * start with panning
 * register another callback will place it before the first e.g.
 * register a callback will cause the function to be called, then call panning
 */
typedef struct dsp_callback_t {
  struct dsp_callback_t *next;

  // TODO what about sidechain signals generated during the chain?
  // their ugens will be NULL
  Ugen control_ugen;

  dsp_fn_type fn_type;
  union {
    dsp_mono_fn mono;
    dsp_stereo_fn stereo;
  } fn_u;

  dsp_state state;
} *DSP_callback;

DSP_callback dsp_init();

DSP_callback dsp_init_default();
DSP_callback dsp_init_bitcrusher();
DSP_callback dsp_init_stereo_pan();

void dsp_cleanup();

// prepend a new cb chain structure to the current chain
DSP_callback dsp_add_to_chain(DSP_callback head, DSP_callback new_head);

// entry
void stereo_fx_chain(DSP_callback cb, FTYPE *L, FTYPE *R);

#endif
