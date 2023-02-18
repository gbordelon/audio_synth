#include <stdlib.h>
#include <string.h>

#include "tunable.h"

Tunable
tunable_alloc()
{
  return calloc(1, sizeof(struct tunable_t));
}

void
tunable_free(Tunable t)
{
  free(t);
}

void
tunable_cleanup(Tunable t)
{
  if (t != NULL) {
    Tunable n = t->Next;
    if (t->Prev != NULL) {
      t->Prev->Next = n;
    }
    if (n != NULL) {
      n->Prev = t->Prev;
    }

    if (t->name != NULL) {
      free(t->name);
    }
    tunable_free(t);
  }
}

void
tunable_param_set(Tunable t, tunable_type param_type, tunable_range_type range, void *param)
{
  t->param_type = param_type;
  t->range_type = range;
  t->param = param;
}

void
tunable_args_set(Tunable t, tunable_arg args[2])
{
  memcpy(t->args_u, args, 2 * sizeof(tunable_arg));
}

void
tunable_fn_set(Tunable t, tunable_fn_type fn_type, Tunable_fn fn)
{
  t->fn_type = fn_type;
  t->fn_u = *fn;
}

#define NAME_LEN_MAX 256
void
tunable_name_set(Tunable t, const char *description)
{
  if (description != NULL) {
    size_t len = strlen(description);
    len = NAME_LEN_MAX < len ? NAME_LEN_MAX : len;
    t->name = malloc(len * sizeof(char));
    strncpy(t->name, description, len);
  }
}
#undef NAME_LEN_MAX

Tunable
tunable_init(
    tunable_type param_type, tunable_range_type range, void *param,
    tunable_arg args[2], tunable_fn_type fn_type, Tunable_fn fn,
    const char *description
)
{
  Tunable rv = tunable_alloc();
  tunable_param_set(rv, param_type, range, param);
  tunable_args_set(rv, args);
  tunable_fn_set(rv, fn_type, fn);
  tunable_name_set(rv, description);

  return rv;
}

void
arity0_default()
{
}

Tunable
tunable_default_init()
{
  tunable_arg args[2] = {0};
  tunable_fn fn;
  fn.f0 = arity0_default;

  return tunable_init(
    TUNABLE_NONE,
    TUNABLE_RANGE_NONE,
    NULL,
    args,
    ARITY_0,
    &fn,
    NULL
  );
}

void
tunable_param_write(Tunable t, Tunable_arg val)
{
  size_t len = 0;
  switch (t->param_type) {
  case TUNABLE_INT32:
    len = sizeof(int32_t);
    break;
  case TUNABLE_UINT32:
    len = sizeof(uint32_t);
    break;
  case TUNABLE_FLOAT:
    len = sizeof(float);
    break;
  case TUNABLE_DOUBLE:
    len = sizeof(double);
    break;
  default:
    break;
  }
  memcpy(t->param, val, len);
}

void
tunable_callback_invoke(Tunable t)
{
  switch (t->fn_type) {
  case ARITY_0:
    t->fn_u.f0();
    break;

  case ARITY_1_P:
    t->fn_u.f1p(t->args_u[0].v);
    break;
  case ARITY_1_I:
    t->fn_u.f1i(t->args_u[0].i);
    break;
  case ARITY_1_U:
    t->fn_u.f1u(t->args_u[0].u);
    break;
  case ARITY_1_F:
    t->fn_u.f1f(t->args_u[0].f);
    break;
  case ARITY_1_D:
    t->fn_u.f1d(t->args_u[0].d);
    break;

  case ARITY_2_PP:
    t->fn_u.f2pp(t->args_u[0].v, t->args_u[1].v);
    break;
  case ARITY_2_PI:
    t->fn_u.f2pi(t->args_u[0].v, t->args_u[1].i);
    break;
  case ARITY_2_PU:
    t->fn_u.f2pu(t->args_u[0].v, t->args_u[1].u);
    break;
  case ARITY_2_PF:
    t->fn_u.f2pf(t->args_u[0].v, t->args_u[1].f);
    break;
  case ARITY_2_PD:
    t->fn_u.f2pd(t->args_u[0].v, t->args_u[1].d);
    break;

  case ARITY_2_II:
    t->fn_u.f2ii(t->args_u[0].i, t->args_u[1].i);
    break;
  case ARITY_2_IU:
    t->fn_u.f2iu(t->args_u[0].i, t->args_u[1].u);
    break;
  case ARITY_2_IF:
    t->fn_u.f2if(t->args_u[0].i, t->args_u[1].f);
    break;
  case ARITY_2_ID:
    t->fn_u.f2ii(t->args_u[0].i, t->args_u[1].i);
    break;

  case ARITY_2_UU:
    t->fn_u.f2uu(t->args_u[0].u, t->args_u[1].u);
    break;
  case ARITY_2_UF:
    t->fn_u.f2uf(t->args_u[0].u, t->args_u[1].f);
    break;
  case ARITY_2_UD:
    t->fn_u.f2ud(t->args_u[0].u, t->args_u[1].d);
    break;

  case ARITY_2_FF:
    t->fn_u.f2ff(t->args_u[0].f, t->args_u[1].f);
    break;
  case ARITY_2_FD:
    t->fn_u.f2fd(t->args_u[0].f, t->args_u[1].d);
    break;

  case ARITY_2_DD:
    t->fn_u.f2dd(t->args_u[0].d, t->args_u[1].d);
    break;
  default:
    break;
  }
}
