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
#include "comb_filter.h"
#include "control_joiner.h"
#include "envelope_follower.h"
#include "modulated_delay.h"
#include "pan.h"
#include "passthru.h"
#include "phase_shifter.h"
#include "reverb_tank.h"
#include "signal_source.h"
#include "stereo2mono.h"
#include "sum.h"
#include "two_band_shelving_filter.h"
#include "waveshaper.h"

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

#define NAMES C(FX_UNIT_UNUSED)\
C(FX_UNIT_AUDIO_DELAY)\
C(FX_UNIT_AUDIO_DETECTOR)\
C(FX_UNIT_AUDIO_FILTER)\
C(FX_UNIT_BITCRUSHER)\
C(FX_UNIT_BUFFER)\
C(FX_UNIT_COMB_FILTER)\
C(FX_UNIT_CONTROL_JOINER)\
C(FX_UNIT_ENVELOPE_FOLLOWER)\
C(FX_UNIT_MODULATED_DELAY)\
C(FX_UNIT_PAN)\
C(FX_UNIT_PASSTHRU)\
C(FX_UNIT_PHASE_SHIFTER)\
C(FX_UNIT_REVERB_TANK)\
C(FX_UNIT_SIGNAL_SOURCE)\
C(FX_UNIT_STEREO_2_MONO)\
C(FX_UNIT_SUM)\
C(FX_UNIT_TWO_BAND_SHELVING_FILTER)\
C(FX_UNIT_WAVESHAPER)
#define C(x) x,
typedef enum fx_unit_type_e {
  NAMES FX_UNIT_TYPE_NUM
} fx_unit_type;
#undef C
#define C(x) #x,

typedef struct fx_unit_params_t {
  FTYPE sample_rate;
  fx_unit_type t;
  union {
    fx_unit_audio_delay_params audio_delay;
    fx_unit_audio_detector_params audio_detector;
    fx_unit_audio_filter_params audio_filter;
    fx_unit_bitcrusher_params bitcrusher;
    fx_unit_buffer_params buffer;
    fx_unit_comb_filter_params comb_filter;
    fx_unit_control_joiner_params control_joiner;
    fx_unit_envelope_follower_params envelope_follower;
    fx_unit_modulated_delay_params modulated_delay;
    fx_unit_pan_params pan;
    fx_unit_passthru_params passthru;
    fx_unit_phase_shifter_params phase_shifter;
    fx_unit_reverb_tank_params reverb_tank;
    fx_unit_signal_source_params signal_source;
    fx_unit_sum_params sum;
    fx_unit_s2m_params s2m;
    fx_unit_two_band_shelving_filter_params two_band_shelving_filter;
    fx_unit_waveshaper_params waveshaper;

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
    fx_unit_comb_filter_state comb_filter;
    fx_unit_control_joiner_state control_joiner;
    fx_unit_envelope_follower_state envelope_follower;
    fx_unit_modulated_delay_state modulated_delay;
    fx_unit_pan_state pan;
    fx_unit_passthru_state passthru;
    fx_unit_phase_shifter_state phase_shifter;
    fx_unit_reverb_tank_state reverb_tank;
    fx_unit_signal_source_state signal_source;
    fx_unit_sum_state sum;
    fx_unit_s2m_state s2m;
    fx_unit_two_band_shelving_filter_state two_band_shelving_filter;
    fx_unit_waveshaper_state waveshaper;

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
} fx_unit;

typedef fx_unit *FX_unit;

typedef struct fx_compound_unit_t {
  size_t num_units;
  fx_unit_idx *units;
  size_t num_heads;
  fx_unit_idx *heads;
  fx_unit_idx tail;
} fx_compound_unit;

typedef fx_compound_unit *FX_compound_unit;

void fx_compound_unit_cleanup(FX_compound_unit unit);
FX_compound_unit fx_compound_unit_init(size_t num_units, size_t num_heads);
void fx_compound_unit_insert_as_parent(FX_compound_unit unit, FX_compound_unit parent);
void fx_compound_unit_parent_ref_add(FX_compound_unit unit, fx_unit_idx parent_idx);

void fx_unit_library_cleanup();

fx_unit_idx fx_unit_init();
void fx_unit_cleanup(fx_unit_idx);

void fx_unit_entry_point(FTYPE rv[2], fx_unit_idx head);
void fx_unit_process_frame(fx_unit_idx head); // only used by audio driver
void fx_unit_reset_output_buffers(); // only used by audio driver

void fx_unit_insert_as_parent(fx_unit_idx idx, FX_compound_unit parent);
void fx_unit_parent_ref_add(fx_unit_idx idx, fx_unit_idx parent_idx);

#endif
