#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../lib/macros.h"
#include "mmap.h"

Mmap_t
mmap_alloc()
{
  return calloc(1, sizeof(struct mmap_t));
}

void
mmap_free(Mmap_t map)
{
  free(map);
}

Mmap_t
mmap_init() {
  Mmap_t map = mmap_alloc();
  if (map == NULL) {
    perror("mmap_alloc() failed");
    return NULL;
  }

  map->fd = open(MMAP_FILE, O_RDWR);
  if (map->fd < 0) {
    perror("open() failed");
    mmap_free(map);
    return NULL;
  }

  ftruncate(map->fd, MMAP_SIZE * sizeof(float) + MMAP_HEADER_SIZE);

  map->mmap_addr = mmap(NULL, MMAP_SIZE * sizeof(float) + MMAP_HEADER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, map->fd, 0);
  if (map->mmap_addr == MAP_FAILED) {
    perror("mmap() failed");
    close(map->fd);
    mmap_free(map);
    return NULL;
  }

  close(map->fd);
  return map;
}

void
mmap_clean(Mmap_t map)
{
  if (munmap(map->mmap_addr, MMAP_SIZE)){
    perror("munmap() failed");
  }

  mmap_free(map);
}

size_t
mmap_write(Mmap_t map, BYTE *msg, size_t len)
{
  if (memcmp(map->mmap_addr, HEAD2, MMAP_HEADER_SIZE) == 0) {
    memmove(map->mmap_addr + MMAP_HEADER_SIZE, msg, len);
    memset(map->mmap_addr, HEAD1, MMAP_HEADER_SIZE);
    return len;
  }
  return 0;
}
