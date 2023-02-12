#ifndef TUNABLE_H
#define TUNABLE_H

#include <stdlib.h>

#include "../lib/macros.h"

/*
 * parameter type
 */
typedef enum {
  TUNABLE_NONE,
  TUNABLE_INT32,
  TUNABLE_UINT32,
  TUNABLE_FLOAT,
  TUNABLE_DOUBLE,
  TUNABLE_TYPE_NUM
} tunable_type;

/*
 * Specify how to convert the MIDI value to the desired range
 */
// TODO remember pitchbend has 14 bits. [0, 16383]
typedef enum {
  TUNABLE_RANGE_NONE, // don't care about output type.
  TUNABLE_RANGE_0_127, // integer between 0 and 127
  TUNABLE_RANGE_0_1, // float between 0 and 1
  TUNABLE_RANGE_0_24000, // float between 0 and 24000
  TUNABLE_RANGE_NUM
} tunable_range_type;

/*
 * callback function type
 */
typedef enum {
  ARITY_0,

  ARITY_1_P,
  ARITY_1_I,
  ARITY_1_U,
  ARITY_1_F,
  ARITY_1_D,

  ARITY_2_PP,
  ARITY_2_PI,
  ARITY_2_PU,
  ARITY_2_PF,
  ARITY_2_PD,

  ARITY_2_II,
  ARITY_2_IU,
  ARITY_2_IF,
  ARITY_2_ID,

  ARITY_2_UU,
  ARITY_2_UF,
  ARITY_2_UD,

  ARITY_2_FF,
  ARITY_2_FD,

  ARITY_2_DD,

  ARITY_NUM
} tunable_fn_type;

/*
 * for arguments to callback functions
 */
typedef union {
    void *v;
    int32_t i;
    uint32_t u;
    float f;
    double d;
} tunable_arg;

typedef tunable_arg *Tunable_arg;

/*
 * callback function types
 */
typedef void (*arity0)();

typedef void (*arity1_p)(void *);
typedef void (*arity1_i)(int32_t);
typedef void (*arity1_u)(uint32_t);
typedef void (*arity1_f)(float);
typedef void (*arity1_d)(double);

typedef void (*arity2_pp)(void *, void *);
typedef void (*arity2_pi)(void *, int32_t);
typedef void (*arity2_pu)(void *, uint32_t);
typedef void (*arity2_pf)(void *, float);
typedef void (*arity2_pd)(void *, double);

typedef void (*arity2_ii)(int32_t, int32_t);
typedef void (*arity2_iu)(int32_t, uint32_t);
typedef void (*arity2_if)(int32_t, float);
typedef void (*arity2_id)(int32_t, double);

typedef void (*arity2_uu)(uint32_t, uint32_t);
typedef void (*arity2_uf)(uint32_t, float);
typedef void (*arity2_ud)(uint32_t, double);

typedef void (*arity2_ff)(float, float);
typedef void (*arity2_fd)(float, double);

typedef void (*arity2_dd)(double, double);

/*
 * callback function union
 */
typedef union {
    arity0 f0;

    arity1_p f1p;
    arity1_i f1i;
    arity1_u f1u;
    arity1_f f1f;
    arity1_d f1d;

    arity2_pp f2pp;
    arity2_pi f2pi;
    arity2_pu f2pu;
    arity2_pf f2pf;
    arity2_pd f2pd;

    arity2_ii f2ii;
    arity2_iu f2iu;
    arity2_if f2if;
    arity2_id f2id;

    arity2_uu f2uu;
    arity2_uf f2uf;
    arity2_ud f2ud;

    arity2_ff f2ff;
    arity2_fd f2fd;

    arity2_dd f2dd;
} tunable_fn;

typedef tunable_fn *Tunable_fn;

/*
 * contains the address and type of the tunable parameter to be updated.
 * also contains a function and its arguments to be called after updating the tunable.
 */
typedef struct tunable_t {
  struct tunable_t *Next, *Prev;

  char *name;

  tunable_type param_type;
  void *param;

  tunable_range_type range_type;

  tunable_fn_type args_type[2]; // TODO redundant information to fn_type?
  tunable_arg args_u[2];

  tunable_fn_type fn_type;
  tunable_fn fn_u;

} tunable;

typedef tunable *Tunable;

void tunable_cleanup(Tunable t);

Tunable tunable_default_init();
Tunable tunable_init(
    tunable_type param_type, tunable_range_type range, void *param,
    tunable_fn_type args_type[2], tunable_arg args[2],
    tunable_fn_type fn_type, Tunable_fn fn, const char *description
);

void tunable_param_set(Tunable t, tunable_type param_type, tunable_range_type range, void *param);
void tunable_args_set(Tunable t, tunable_fn_type args_type[2], tunable_arg args[2]);
void tunable_fn_set(Tunable t, tunable_fn_type fn_type, Tunable_fn fn);
void tunable_name_set(Tunable t, const char *description);

void tunable_param_write(Tunable t, Tunable_arg val);
void tunable_callback_invoke(Tunable t);

/*
 * TODO main loop needs to index Tunables and map them to midi messages
 * probably just an array of 128 elements for CC messages, but what about pitch bend,
 * polytouch and touch?
 * How about an array of 131 elements, with CC mapping 0-127 then pb, poly, and touch mapping
 * to the rest?
 * TODO I should be able to control multiple parameters with one signal.
 * TODO I should be able to flatten consecutive signals which might overwrite previous unread values.
 *
 * polytouch maps a key,val pair to a monovoice parameter
 * touch maps a val to a voice-wide parameter
 * pitchbend maps a val to a voice-wide parameter (probably fc for all operators or all carriers)
 */

/*
 * TODO how do I remap a tunable to a different CC? Add to existing list for that signal.
 */

/*
 * TODO How do I initiate a swap? There needs to be a user interaction...
 * A menu and keyboard listener (use select)? a MIDI message? 
 */

#endif
