#ifndef MMAP_H
#define MMAP_H

#include <stdio.h>

#include "../lib/macros.h"

#define PAGE_SIZE 1024
#define MMAP_SIZE (NUM_CHANNELS * PAGE_SIZE)
#define MMAP_FILE "./DEADBEEF"
#define MMAP_HEADER_SIZE 4
#define HEAD1 'A'
#define HEAD2 "0000"

typedef struct mmap_t {
  int fd;
  BYTE *mmap_addr;
} *Mmap_t;

Mmap_t mmap_init();
void mmap_clean(Mmap_t map);
size_t mmap_write(Mmap_t map, BYTE *msg, size_t len);

#endif
