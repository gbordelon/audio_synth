#include <stdlib.h>

#include "../lib/macros.h"

#include "simple_lpf.h"

/*
 * Based on the description from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */
Simple_lpf
simple_lpf_alloc()
{
  return calloc(1, sizeof(struct simple_lpf_t));
}

Simple_lpf
simple_lpf_init(FTYPE g)
{
  Simple_lpf rv = simple_lpf_alloc();
  rv->g = g;

  return rv;
}

void
simple_lpf_free(Simple_lpf sl)
{
  free(sl);
}

void
simple_lpf_cleanup(Simple_lpf sl)
{
  simple_lpf_free(sl);
}

FTYPE
simple_lpf_process(Simple_lpf sl, FTYPE xn)
{
  sl->z = sl->g * sl->z + xn * (1.0 - sl->g);

  return sl->z;
}
