#ifndef MACROS_H
#define MACROS_H

#define DEFAULT_SAMPLE_RATE 48000
// UGEN_TABLE_SIZE determines the minimum speed of oscillators
// 200 * DEFAULT_SAMPLE_RATE means the minimum speed is 1/200 Hz or 0.005 Hz
#define UGEN_TABLE_SIZE 9600000
#define CHUNK_SIZE 1024
#define NUM_CHANNELS 2
#define FTYPE double
#define BYTE unsigned char

#endif
