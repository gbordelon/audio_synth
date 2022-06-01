#ifndef WAV_H
#define WAV_H

#include <stdint.h>
#include <stdbool.h>

#include "../lib/macros.h"

/*
 * https://wavefilegem.com/how_wave_files_work.html
 */

#define FORMAT_CHUNK_ID 0x666d7420
#define FACT_CHUNK_ID 0x66616374
#define DATA_CHUNK_ID 0x64617461
#define RIFF_CHUNK_ID 0x52494646
#define RIFF_FORMAT_CODE 0x57415645

struct chunk {
  uint32_t chunk_id;
  uint32_t chunk_body_size;
};

typedef struct format_chunk {
  // 0x666d7420 "fmt "
  struct chunk header;
  // integer PCM: 1
  // floating point PCM: 3
  uint16_t format_code;
  // 1 for mono
  // 2 for stereo
  uint16_t number_of_channels;
  // typical 441000
  uint32_t samples_per_second;
  // 4 * 2 * samples_per_second for 32-bit floating point stereo
  uint32_t bytes_per_second;
  // 4 * 2 for 32-bit stereo
  uint16_t bytes_per_sample_frame;
  // probably 32
  uint16_t bits_per_sample;
  // ignore if format code is 1
  // set to 0 if format code is 3
  uint16_t extension_size;
} *Format_chunk;

typedef struct fact_chunk {
  // 0x66616374 "fact"
  struct chunk header;
  uint32_t number_of_sample_frames;
} *Fact_chunk;

typedef struct data_chunk {
  // 0x64617461 "data"
  struct chunk header;
  const BYTE *frames;
} *Data_chunk;

typedef struct riff_chunk {
  // 0x52494646 "RIFF"
  // entire file size - 8 bytes
  struct chunk header;
  uint32_t riff_format_code; // 0x57415645 "WAVE"

  // TODO collection of chunks. usually format chunk followed by data chunks
  Format_chunk format_chunk;
  // optional if format code is 1
  Fact_chunk fact_chunk;
  Data_chunk data_chunk;

} *Riff_chunk;

Riff_chunk riff_alloc(const uint16_t format_code, const uint16_t number_of_channels);
void riff_free(const Riff_chunk riff_chunk);
size_t riff_append_frames(const Riff_chunk, const BYTE *frames, const size_t num_frames);
size_t riff_get_frame_size(const Riff_chunk riff_chunk);
bool riff_write_wav_file(const Riff_chunk riff_chunk, const char *path);


#endif
