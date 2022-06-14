#include <stdlib.h>

#include "../lib/macros.h"

#include "tunable.h"

static FTYPE *tunable_table[NUM_TUNABLES] = {0};

size_t
tunable_register(FTYPE *addr, size_t ind)
{
  if (ind < NUM_TUNABLES && !tunable_table[ind]) {
    tunable_table[ind] = addr;
    return ind;
  }

  int i;
  for (i = 0; i < NUM_TUNABLES; i++) {
    if (!tunable_table[i]) {
      tunable_table[i] = addr;
      return i;
    }
  }

  return 255;
}

void
tunable_unregister(size_t ind)
{
  if (ind < NUM_TUNABLES) {
    tunable_table[ind] = NULL;
  }
}

size_t
tunable_update_table(FTYPE *addr, size_t ind)
{
  tunable_unregister(ind);
  return tunable_register(addr, ind);
}

FTYPE *
tunable_get(size_t ind)
{
  if (ind < NUM_TUNABLES) {
    return tunable_table[ind];
  }
  return NULL;
}

void
tunable_set(FTYPE val, size_t ind)
{
  if (ind < NUM_TUNABLES) {
    (*tunable_table[ind]) = val;
  }
}
