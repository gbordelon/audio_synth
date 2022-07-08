#ifndef MACROS_H
#define MACROS_H

#define DEFAULT_SAMPLE_RATE 48000
// UGEN_TABLE_SIZE determines the minimum speed of oscillators
// 200 * DEFAULT_SAMPLE_RATE means the minimum speed is 1/200 Hz or 0.005 Hz
// normal LFOs range from 0.05 to 20 Hz so use 20 * DEFAULT_SAMPLE_RATE
// use a power of 2 for table size to avoid modulus operator
// 20 * DEFAULT_SAMPLE_RATE -> 960000
// 2^20 is the smallest power of 2 above that
#define UGEN_TABLE_SIZE 1048576
#define CHUNK_SIZE 1024
#define NUM_CHANNELS 2
#define FTYPE double
#define BYTE unsigned char

#endif
