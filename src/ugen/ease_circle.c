#include <math.h>
#include <stdlib.h>

#include "../lib/macros.h"

#include "ugen.h"
#include "ease_circle.h"

FTYPE
ugen_sample_ease_in_circle(Ugen ugen, size_t phase_ind)
{
  return ease_in_circle[phase_ind];
}

FTYPE
ugen_sample_ease_out_circle(Ugen ugen, size_t phase_ind)
{
  return ease_out_circle[phase_ind];
}

void
ugen_generate_table_ease_in_circle()
{
  int i;
  FTYPE incr = 1.0 / UGEN_TABLE_SIZE;
  for (i = 0; i < UGEN_TABLE_SIZE; i++) {
    ease_in_circle[i] = 1.0 - sqrt(1.0 - ((FTYPE)i)*incr * ((FTYPE)i)*incr);
  }
}

void
ugen_generate_table_ease_out_circle()
{
  int i;
  FTYPE incr = 1.0 / UGEN_TABLE_SIZE;
  for (i = 0; i < UGEN_TABLE_SIZE; i++) {
    ease_out_circle[i] = sqrt(1.0 - ((FTYPE)i-1)*incr * ((FTYPE)i-1)*incr);
  }
}
