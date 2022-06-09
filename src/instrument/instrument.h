#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <stdint.h>
#include <stdbool.h>

#include "../lib/macros.h"

#include "../env/envelope.h"
#include "../osc/osc.h"
#include "../pcm/channel.h"

#define instrument_playing(i) ((i)->cur_dur < (i)->max_dur)

typedef struct instrument_t {
  Channel channels;
  size_t channel_num;
  Osc oscillators;
  size_t osc_num;
  Envelope env;
  size_t max_dur;
  size_t cur_dur;
} *Instrument;

Instrument instrument_init(Channel channels, size_t channel_num);
void instrument_cleanup();
void instrument_play_chunk(Instrument instr);
void instrument_play_config(Instrument instr, uint8_t midi_note, FTYPE dur);

#endif
