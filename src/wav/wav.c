#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "wav.h"

Riff_chunk
riff_alloc(const uint16_t format_code, const uint16_t number_of_channels)
{
  Riff_chunk rc = (Riff_chunk) calloc(1, sizeof(struct riff_chunk));
  // null check rc

  rc->header.chunk_id = RIFF_CHUNK_ID;

  // TODO probably need to rethink this
  rc->header.chunk_body_size = 4 +
    8 + 2 + 2 + 4 + 4 + 2 + 2 + 2 +
    8 + 4 +
    8;

  rc->riff_format_code = RIFF_FORMAT_CODE;

  rc->format_chunk = (Format_chunk) calloc(1, sizeof(struct format_chunk));
  // null check rc->format_chunk

  rc->format_chunk->header.chunk_id = FORMAT_CHUNK_ID;
  rc->format_chunk->header.chunk_body_size = sizeof(struct format_chunk) - sizeof(struct chunk) - 2 /* WTF? */;

  rc->format_chunk->format_code = format_code;
  rc->format_chunk->number_of_channels = number_of_channels;
  rc->format_chunk->samples_per_second = DEFAULT_SAMPLE_FREQUENCY;
  rc->format_chunk->bytes_per_second = rc->format_chunk->samples_per_second * number_of_channels * (format_code == 3 ? 4 : 2);
  rc->format_chunk->bytes_per_sample_frame = number_of_channels * (format_code == 3 ? 4 : 2);
  rc->format_chunk->bits_per_sample = CHAR_BIT * (format_code == 3 ? 4 : 2);
  rc->format_chunk->extension_size = 0;


  rc->fact_chunk = (Fact_chunk) calloc(1, sizeof(struct fact_chunk));
  // null check rc->fact_chunk

  rc->fact_chunk->header.chunk_id = FACT_CHUNK_ID;
  rc->fact_chunk->header.chunk_body_size = sizeof(struct fact_chunk) - sizeof(struct chunk);

  rc->fact_chunk->number_of_sample_frames = 0; // set later


  rc->data_chunk = (Data_chunk) calloc(1, sizeof(struct data_chunk));
  // null check rc->data_chunk

  rc->data_chunk->header.chunk_id = DATA_CHUNK_ID;
  rc->data_chunk->header.chunk_body_size = 0; // set later

  rc->data_chunk->frames = NULL; // set later

  return rc;
}

void
riff_free(const Riff_chunk riff_chunk)
{
  free((void *)riff_chunk->data_chunk->frames);
  free(riff_chunk->data_chunk);
  free(riff_chunk->fact_chunk);
  free(riff_chunk->format_chunk);
  free(riff_chunk);
}

size_t
riff_append_frames(const Riff_chunk riff_chunk, const BYTE *frames, const size_t num_frames)
{
  // appropriate null chunks

  BYTE *new_frames = realloc((void *)riff_chunk->data_chunk->frames, (riff_chunk->fact_chunk->number_of_sample_frames + num_frames) * riff_chunk->format_chunk->bytes_per_sample_frame);
  // null check

  memcpy(new_frames + riff_chunk->fact_chunk->number_of_sample_frames * riff_chunk->format_chunk->bytes_per_sample_frame, frames, num_frames * riff_chunk->format_chunk->bytes_per_sample_frame);
  riff_chunk->data_chunk->frames = new_frames;
  riff_chunk->fact_chunk->number_of_sample_frames += num_frames;

  riff_chunk->data_chunk->header.chunk_body_size = riff_chunk->fact_chunk->number_of_sample_frames * riff_chunk->format_chunk->bytes_per_sample_frame;
  return num_frames;
}

size_t
riff_get_frame_size(const Riff_chunk riff_chunk)
{
  return riff_chunk->format_chunk->bytes_per_sample_frame;
}

void
write_word_be(FILE *f, uint32_t word, size_t size)
{
  if (size == 2) {
    fputc((word & 0x0000ff00)>>CHAR_BIT, f);
    fputc((word & 0x000000ff), f);
  } else if (size == 4) {
    fputc((word & 0xff000000)>>(CHAR_BIT*3), f);
    fputc((word & 0x00ff0000)>>(CHAR_BIT*2), f);
    fputc((word & 0x0000ff00)>>CHAR_BIT, f);
    fputc((word & 0x000000ff), f);
  } else {
    for (; size; --size, word >>= CHAR_BIT) {
      fputc(word & 0xff, f);
    }
  }
}

void
write_word(FILE *f, uint32_t word, size_t size)
{
  for (; size; --size, word >>= CHAR_BIT) {
    fputc(word & 0xff, f);
  }
}

bool
riff_write_wav_file(const Riff_chunk riff_chunk, const char *path)
{
  riff_chunk->header.chunk_body_size += riff_chunk->data_chunk->header.chunk_body_size;
  FILE *f = fopen(path, "wb");
  if (f != NULL) {
    // write riff header
    write_word_be(f, riff_chunk->header.chunk_id, 4);
    write_word(f, riff_chunk->header.chunk_body_size, 4);
    write_word_be(f, riff_chunk->riff_format_code, 4);

    write_word_be(f, riff_chunk->format_chunk->header.chunk_id, 4);
    write_word(f, riff_chunk->format_chunk->header.chunk_body_size, 4);
    write_word(f, riff_chunk->format_chunk->format_code, 2);
    write_word(f, riff_chunk->format_chunk->number_of_channels, 2);
    write_word(f, riff_chunk->format_chunk->samples_per_second, 4);
    write_word(f, riff_chunk->format_chunk->bytes_per_second, 4);
    write_word(f, riff_chunk->format_chunk->bytes_per_sample_frame, 2);
    write_word(f, riff_chunk->format_chunk->bits_per_sample, 2);
    if (riff_chunk->format_chunk->format_code == 3) {
      write_word(f, riff_chunk->format_chunk->extension_size, 2);
    }

    write_word_be(f, riff_chunk->fact_chunk->header.chunk_id, 4);
    write_word(f, riff_chunk->fact_chunk->header.chunk_body_size, 4);
    write_word(f, riff_chunk->fact_chunk->number_of_sample_frames, 4);

    write_word_be(f, riff_chunk->data_chunk->header.chunk_id, 4);
    write_word(f, riff_chunk->data_chunk->header.chunk_body_size, 4);

    size_t word_length = riff_chunk->format_chunk->bytes_per_sample_frame / riff_chunk->format_chunk->number_of_channels;
    size_t sample_count = riff_chunk->fact_chunk->number_of_sample_frames * riff_chunk->format_chunk->number_of_channels;
    const BYTE *current = riff_chunk->data_chunk->frames;
    for (; sample_count; --sample_count, current += word_length) {
      write_word(f, *(uint32_t *)current, word_length);
    }

    fclose(f);
    return true;
  }
  return false;
}

