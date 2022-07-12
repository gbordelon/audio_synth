#ifndef DSP_H
#define DSP_H

#include "../lib/macros.h"

#include "../ugen/ugen.h"

#include "audio_delay.h"
#include "audio_detector.h"
#include "audio_filter.h"
#include "biquad.h"
#include "envelope_follower.h"
#include "modulated_delay.h"
#include "phase_shifter.h"

typedef union dsp_state_u {
  struct {
    FTYPE quantized_bit_depth;
  } bitcrusher;
  audio_filter_params audio_filter;
  audio_delay_params audio_delay;
  audio_detector_params audio_detector;
  envelope_follower_params envelope_follower;
  modulated_delay_params modulated_delay;
  phase_shifter_params phase_shifter;
} dsp_state;

typedef enum {
  DSP_MONO_L,
  DSP_MONO_R,
  DSP_STEREO
} dsp_fn_type;

// sample pointer, control
typedef FTYPE (*dsp_mono_fn)(FTYPE *, dsp_state *, FTYPE);

// sample pointer, sample pointer, control
typedef FTYPE (*dsp_stereo_fn)(FTYPE *, FTYPE *, dsp_state *, FTYPE);

typedef enum {
  DSP_CONTROL_NONE,
  DSP_CONTROL_CALLBACK,
  DSP_CONTROL_CONSTANT,
  DSP_CONTROL_UGEN
} dsp_control_e;

/*
 * start with panning
 * register another callback will place it before the first e.g.
 * register a callback will cause the function to be called, then call panning
 */
typedef struct dsp_callback_t {
  struct dsp_callback_t *next;

  // TODO what about functions of multiple control paramaters?
  dsp_control_e control;
  union {
    struct dsp_callback_t *dsp;
    Ugen ugen;
    FTYPE constant;
  } ctrl_u;

  dsp_fn_type fn_type;
  union {
    dsp_mono_fn mono;
    dsp_stereo_fn stereo;
  } fn_u;

  dsp_state state;
} *DSP_callback;

DSP_callback dsp_init();
DSP_callback dsp_init_default();

DSP_callback dsp_init_audio_delay(audio_delay_params params);
DSP_callback dsp_init_audio_delay_default();

DSP_callback dsp_init_audio_detector(audio_detector_params params);
DSP_callback dsp_init_audio_detector_default();

DSP_callback dsp_init_audio_filter(audio_filter_params params);
DSP_callback dsp_init_audio_filter_default();

DSP_callback dsp_init_bitcrusher();

DSP_callback dsp_init_envelope_follower(envelope_follower_params params);
DSP_callback dsp_init_envelope_follower_default();

DSP_callback dsp_init_modulated_delay(modulated_delay_params params);
DSP_callback dsp_init_modulated_delay_chorus_default();
DSP_callback dsp_init_modulated_delay_flanger_default();
DSP_callback dsp_init_modulated_delay_vibrato_default();

DSP_callback dsp_init_phase_shifter(phase_shifter_params params);
DSP_callback dsp_init_phase_shifter_default();

DSP_callback dsp_init_stereo_pan();

void dsp_cleanup();


void dsp_set_mono_left(DSP_callback cb, dsp_mono_fn fn);
void dsp_set_mono_right(DSP_callback cb, dsp_mono_fn fn);
void dsp_set_stereo(DSP_callback cb, dsp_stereo_fn fn);

void dsp_set_control_ugen(DSP_callback cb, Ugen ugen);
void dsp_set_control_dsp(DSP_callback cb, DSP_callback ctrl);
void dsp_set_control_constant(DSP_callback cb, FTYPE ctrl);

// prepend a new cb chain structure to the current chain
DSP_callback dsp_add_to_chain(DSP_callback head, DSP_callback new_head);

// entry
FTYPE stereo_fx_chain(DSP_callback cb, FTYPE *L, FTYPE *R);

#endif
