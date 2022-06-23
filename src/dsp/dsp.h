#ifndef DSP_H
#define DSP_H

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#include "audio_filter.h"
#include "biquad.h"

typedef union dsp_state_u {
  struct {
    FTYPE quantized_bit_depth;
  } bitcrusher;
  audio_filter_params audio_filter;
} dsp_state;

typedef enum {
  DSP_MONO_L,
  DSP_MONO_R,
  DSP_STEREO
} dsp_fn_type;

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
  // TODO what about functions of multiple paramters? How about an array of ugens?
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

DSP_callback dsp_init_audio_filter();
DSP_callback dsp_init_audio_filter_default();

DSP_callback dsp_init_bitcrusher();

DSP_callback dsp_init_stereo_pan();

void dsp_cleanup();

void dsp_audio_filter_set_mono_left(DSP_callback cb);
void dsp_audio_filter_set_mono_right(DSP_callback cb);

// prepend a new cb chain structure to the current chain
DSP_callback dsp_add_to_chain(DSP_callback head, DSP_callback new_head);

// entry
void stereo_fx_chain(DSP_callback cb, FTYPE *L, FTYPE *R);

#endif
