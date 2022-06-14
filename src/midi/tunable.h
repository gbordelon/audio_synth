#ifndef TUNABLE_H
#define TUNABLE_H

#include <stdlib.h>

#include "../lib/macros.h"

#define NUM_TUNABLES 128

size_t tunable_register(FTYPE *addr, size_t ind);
size_t tunable_update_table(FTYPE *addr, size_t ind);
void tunable_unregister(size_t ind);

FTYPE *tunable_get(size_t ind);
void tunable_set(FTYPE val, size_t ind);

#endif
