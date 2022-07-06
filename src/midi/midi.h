#ifndef MIDI_H
#define MIDI_H

#include <portmidi.h>
#include <pmutil.h>

/*
 * RIP Dave Smith, who led the development of the MIDI spec, and who died today.
 */

#define MIDI_CODE_MASK  0xf0
#define MIDI_CHN_MASK   0x0f
#define MIDI_OFF_NOTE   0x80
#define MIDI_ON_NOTE    0x90
#define MIDI_POLY_TOUCH 0xa0
#define MIDI_CTRL       0xb0
#define MIDI_CH_PROGRAM 0xc0
#define MIDI_TOUCH      0xd0
#define MIDI_BEND       0xe0

#define MIDI_SYSEX      0xf0
#define MIDI_Q_FRAME	0xf1
#define MIDI_SONG_POINTER 0xf2
#define MIDI_SONG_SELECT 0xf3
#define MIDI_TUNE_REQ	0xf6
#define MIDI_EOX        0xf7
#define MIDI_TIME_CLOCK 0xf8
#define MIDI_START      0xfa
#define MIDI_CONTINUE	0xfb
#define MIDI_STOP       0xfc
#define MIDI_ACTIVE_SENSING 0xfe
#define MIDI_SYS_RESET  0xff

#define MIDI_ALL_SOUND_OFF 0x78
#define MIDI_RESET_CONTROLLERS 0x79
#define MIDI_LOCAL	0x7a
#define MIDI_ALL_OFF	0x7b
#define MIDI_OMNI_OFF	0x7c
#define MIDI_OMNI_ON	0x7d
#define MIDI_MONO_ON	0x7e
#define MIDI_POLY_ON	0x7f

static const FTYPE midi_note_to_freq_table[128] = {
     8.18,
     8.66,
     9.18,
     9.72,
     10.30,
     10.91,
     11.56,
     12.25,
     12.98,
     13.75, /* A-1 9 */
     14.57,
     15.43,
     16.35,
     17.32,
     18.35,
     19.45,
     20.60,
     21.83,
     23.12,
     24.50,
     25.96,
     27.50, /* A0 21 */
     29.14,
     30.87,
     32.70,
     34.65,
     36.71,
     38.89,
     41.20,
     43.65,
     46.25,
     49.00,
     51.91,
     55.00, /* A1 33 */
     58.27,
     61.74,
     65.41,
     69.30,
     73.42,
     77.78,
     82.41,
     87.31,
     92.50,
     98.00,
    103.83,
    110.00, /* A2 45 */
    116.54,
    123.47,
    130.81,
    138.59,
    146.83,
    155.56,
    164.81,
    174.61,
    185.00,
    196.00,
    207.65,
    220.00, /* A3 57 */
    233.08,
    246.94,
    261.63, /* middle C 60 */
    277.18,
    293.66,
    311.13,
    329.63,
    349.23,
    369.99,
    392.00,
    415.30,
    440.00, /* A4 */
    466.16,
    493.88,
    523.25,
    554.37,
    587.33,
    622.25,
    659.26,
    698.46,
    739.99,
    783.99,
    830.61,
    880.00, /* A5 */
    932.33,
    987.77,
   1046.50,
   1108.73,
   1174.66,
   1244.51,
   1318.51,
   1396.91,
   1479.98,
   1567.98,
   1661.22,
   1760.00, /* A6 */
   1864.66,
   1975.53,
   2093.00,
   2217.46,
   2349.32,
   2489.02,
   2637.02,
   2793.83,
   2959.96,
   3135.96,
   3322.44,
   3520.00, /* A7 */
   3729.31,
   3951.07,
   4186.01,
   4434.92,
   4698.64,
   4978.03,
   5274.04,
   5587.65,
   5919.91,
   6271.93,
   6644.88,
   7040.00, /* A8 */
   7458.62,
   7902.13,
   8372.02,
   8869.84,
   9397.27,
   9956.06,
  10548.08,
  11175.30,
  11839.82,
  12543.85,
//  13289.75
};

PmQueue *midi_listener_init();
void midi_cleanup();

void midi_start();
void midi_stop();

#endif
