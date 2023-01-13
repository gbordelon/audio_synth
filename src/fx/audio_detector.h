#ifndef FX_AUDIO_DETECTOR_H
#define FX_AUDIO_DETECTOR_H

#include <stdbool.h>

#include "../lib/macros.h"

// ln(36.8%)
#define AUDIO_DETECTOR_ENVELOPE_ANALOG_TC -0.9996723408132061

typedef enum {
  AUDIO_DETECTOR_MODE_PEAK,
  AUDIO_DETECTOR_MODE_MS,
  AUDIO_DETECTOR_MODE_RMS
} detect_mode_e;

typedef struct fx_unit_audio_detector_state_t {
  FTYPE sample_rate;
  FTYPE attack_time;
  FTYPE release_time;
  detect_mode_e detect_mode;
  bool detect_db;
  bool clamp_to_unity_max;
  FTYPE previous_envelope;
} fx_unit_audio_detector_state;

typedef fx_unit_audio_detector_state *FX_unit_audio_detector_state;

typedef struct fx_unit_audio_detector_params_t {
  FTYPE attack_time;
  FTYPE release_time;
  detect_mode_e detect_mode;
  bool detect_db;
  bool clamp_to_unity_max;
} fx_unit_audio_detector_params;

typedef fx_unit_audio_detector_params *FX_unit_audio_detector_params;

// forward decl
typedef struct fx_unit_params_t *FX_unit_params;
typedef int16_t fx_unit_idx;

fx_unit_idx fx_unit_audio_detector_init(FX_unit_params params);

#endif
