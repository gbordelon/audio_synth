#ifndef FX_H
#define FX_H

#include <stdbool.h>

#include "../lib/macros.h"

#include "audio_delay.h"
#include "audio_detector.h"
#include "audio_filter.h"
#include "biquad.h"
#include "bitcrusher.h"
#include "buffer.h"
#include "control_joiner.h"
#include "pan.h"
#include "signal_source.h"
#include "stereo2mono.h"

#include "../ugen/ugen.h"

typedef int16_t fx_unit_idx;
#define FX_UNIT_IDX_NONE (-1)

typedef enum {
  FX_L,
  FX_R,
  FX_C,
  FX_LRC_SIZE
} lrc_e;

/*
 * Buffer should hold a single frame
 */
typedef struct lrc_buffer_t {
  bool dirty;
  FTYPE lrc[FX_LRC_SIZE];
} lrc_buf;

typedef enum fx_unit_type_e {
  FX_UNIT_UNUSED,
  FX_UNIT_AUDIO_DELAY,
  FX_UNIT_AUDIO_DETECTOR,
  FX_UNIT_AUDIO_FILTER,
  FX_UNIT_BITCRUSHER,
  FX_UNIT_BUFFER,
  FX_UNIT_CONTROL_JOINER,
  FX_UNIT_PAN,
  FX_UNIT_SIGNAL_SOURCE,
  FX_UNIT_STEREO_2_MONO,
} fx_unit_type;

typedef struct fx_unit_params_t {
  FTYPE sample_rate;
  fx_unit_type t;
  union {
    fx_unit_audio_delay_params audio_delay;
    fx_unit_audio_detector_params audio_detector;
    fx_unit_audio_filter_params audio_filter;
    fx_unit_bitcrusher_params bitcrusher;
    fx_unit_buffer_params buffer;
    fx_unit_control_joiner_params control_joiner;
    fx_unit_pan_params pan;
    fx_unit_signal_source_params signal_source;
    fx_unit_s2m_params s2m;

  } u;
} fx_unit_params;

typedef fx_unit_params *FX_unit_params;

/*
 * FX units should be a complete effect like delay or enveloper follower etc
 */
typedef struct fx_unit_state_t {
  FTYPE sample_rate;
  fx_unit_type t; // type for debugging
  struct {
    void (*cleanup)(struct fx_unit_state_t *); // takes pointer to state
    //void (*init)(FX_unit_params); // takes pointer to params
    void (*process_frame)(fx_unit_idx); // takes reference to the fx_unit to access parents' buffers and output buffer
    void (*reset)(struct fx_unit_state_t *, FX_unit_params); // takes pointer to state and pointer to params
  } f; // function pointers for fx_unit interface
  union {
    fx_unit_audio_delay_state audio_delay;
    fx_unit_audio_detector_state audio_detector;
    fx_unit_audio_filter_state audio_filter;
    fx_unit_bitcrusher_state bitcrusher;
    fx_unit_buffer_state buffer;
    fx_unit_control_joiner_state control_joiner;
    fx_unit_pan_state pan;
    fx_unit_signal_source_state signal_source;
    fx_unit_s2m_state s2m;

  } u; // actual state
} fx_unit_state;

typedef fx_unit_state *FX_unit_state;

typedef struct fx_unit_t {
  size_t num_parents;
  fx_unit_idx *parents;

  // buffer // for output
  lrc_buf output_buffer;

  // effect-specific state data (should include function pointer for populating the buffer)
  fx_unit_state state;
  // state can include a ugen for generating audio data, or control data, or a constant for control data
} *FX_unit;

/*
 * FX subunits should be objects which help out FX units like the comb filter or delay_apf, etc
 */
typedef union fx_subunit_state_u {
} fx_subunit_state;

typedef struct fx_subunit_t {
  fx_subunit_state state;
} *FX_subunit;

void fx_unit_library_cleanup();

fx_unit_idx fx_unit_init();
fx_unit_idx fx_unit_default_init();
void fx_unit_cleanup(fx_unit_idx);

lrc_buf fx_unit_idx_to_buf(fx_unit_idx);
void  fx_unit_add_parent_ref(fx_unit_idx idx, fx_unit_idx parent_idx);
fx_unit_idx fx_unit_replace_parent_ref(fx_unit_idx idx, fx_unit_idx parent_idx); // returns old parent idx

void fx_unit_entry_point(FTYPE rv[2], fx_unit_idx head);
void fx_unit_process_frame(fx_unit_idx head); // only used by audio driver
void fx_unit_reset_output_buffers(); // only used by audio driver

FX_subunit fx_subunit_init();
FX_subunit fx_subunit_default_init();

#endif
