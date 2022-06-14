#ifndef AUDIO_UNIT_H
#define AUDIO_UNIT_H

#include <AudioUnit/AudioUnit.h>

AudioComponentInstance audio_unit_init();
void audio_unit_go(AudioComponentInstance audioUnit);

#endif
