#ifndef AUDIO_DETECTOR_H
#define AUDIO_DETECTOR_H

#include <math.h>
#include <stdbool.h>

#include "../lib/macros.h"

#include "../fx/audio_detector.h"

/*
// ln(36.8%)
#define AUDIO_DETECTOR_ENVELOPE_ANALOG_TC -0.9996723408132061

typedef enum {
  AUDIO_DETECTOR_MODE_PEAK,
  AUDIO_DETECTOR_MODE_MS,
  AUDIO_DETECTOR_MODE_RMS
} detect_mode_e;
*/
typedef struct {
  FTYPE sample_rate;
  FTYPE attack_time;
  FTYPE release_time;
  detect_mode_e detect_mode;
  bool detect_db;
  bool clamp_to_unity_max;
  FTYPE previous_envelope;
} audio_detector_params;

// forward declaration of type from dsp.h
typedef union dsp_state_u dsp_state;

void dsp_audio_detector_set_params(dsp_state *state, audio_detector_params params);

// forward declaration of type from dsp.h
typedef struct dsp_callback_t *DSP_callback;

void dsp_audio_detector_reset(DSP_callback cb);

#endif
