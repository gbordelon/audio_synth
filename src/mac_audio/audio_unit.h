#ifndef AUDIO_UNIT_H
#define AUDIO_UNIT_H

#include <AudioUnit/AudioUnit.h>

AudioComponentInstance audio_unit_input_init();
AudioComponentInstance audio_unit_io_init();
AudioComponentInstance audio_unit_output_init();

void audio_unit_go(AudioComponentInstance audioUnit);

#endif
