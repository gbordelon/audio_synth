#ifndef SIMPLE_LPF_H
#define SIMPLE_LPF_H

#include "../lib/macros.h"

/*
 * Based on the description from Designing Audio Effect Plugins in C++ by Pirkle, chapter 17
 */

typedef struct simple_lpf_t {
  FTYPE g;
  FTYPE z;
} *Simple_lpf;

Simple_lpf simple_lpf_init(FTYPE g);
void simple_lpf_cleanup(Simple_lpf sl);

FTYPE simple_lpf_process(Simple_lpf sl, FTYPE xn);

#endif
