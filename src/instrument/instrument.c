#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "instrument.h"
#include "../env/envelope.h"
#include "../lib/macros.h"
#include "../osc/imp.h"
#include "../osc/osc.h"
#include "../midi/midi.h"

Instrument
instrument_alloc()
{
  return calloc(1, sizeof(struct instrument_t));
}

void
instrument_free(Instrument instr)
{
  free(instr);
}

Instrument
instrument_init(Channel channels, size_t channel_num)
{
  Instrument rv = instrument_alloc();
  rv->channels = channels;
  rv->channel_num = channel_num;
  // TODO for channel in channels: increment refcount
  rv->oscillators = osc_alloc_many(2);
  rv->osc_num = 2;
  osc_set(rv->oscillators, OSC_SIN, midi_note_to_freq_table[45]);
  osc_set(&rv->oscillators[1], OSC_IMP, midi_note_to_freq_table[45] * 7.0 / 2.0);
  imp_set_duty_cycle(&rv->oscillators[1], 0.1);

  rv->env = env_init(DEFAULT_SAMPLE_RATE);
  rv->max_dur = 0;
  rv->cur_dur = 0;

  return rv;
}

void
instrument_cleanup(Instrument instr)
{
  // TODO for channel in channels: reduce refcount
  osc_cleanup(instr->oscillators);
  env_cleanup(instr->env);
  instrument_free(instr);
}

void
instrument_play_resume(Instrument instr)
{
  Channel left = instr->channels;
  Channel right = &instr->channels[1];

  FTYPE t_sample, e_sample;
  FTYPE sample[2];

  Osc car_gen = instr->oscillators;
  Osc mod_gen = &instr->oscillators[1];
  //printf("mod gen\n  type: %d\n  tone_freq: %f\n  p_inc_whole: %u\n  p_inc_frac: %f\n  p_ind: %u\n  duty_cycle: %f\n", mod_gen->type, mod_gen->tone_freq, mod_gen->p_inc_whole, mod_gen->p_inc_frac, mod_gen->p_ind, mod_gen->u.imp.duty_cycle);

  t_sample = osc_sample_phase_osc(car_gen, mod_gen);
  e_sample = env_sample(instr->env);

  sample[0] = t_sample * e_sample;
  sample[1] = t_sample * e_sample;

  channel_write(left, sample[0]);
  channel_write(right, sample[1]);

  instr->cur_dur++;
}

// TODO support other instruments
// currently assumes two oscillators
void
instrument_play_config(Instrument instr, uint8_t midi_note, FTYPE dur /* in seconds */)
{
  instr->cur_dur = 0;
  instr->max_dur = DEFAULT_SAMPLE_RATE * dur;
  env_reset(instr->env);
  env_set_duration(instr->env, instr->max_dur);
  // for osc in oscillators
  //    set freq
  //    reset phase?
  osc_set_freq(instr->oscillators, midi_note_to_freq_table[midi_note]);
  osc_set_freq(&instr->oscillators[1], midi_note_to_freq_table[midi_note] * 7.0 / 2.0);
  osc_reset_phase(instr->oscillators);
  osc_reset_phase(&instr->oscillators[1]);
}
