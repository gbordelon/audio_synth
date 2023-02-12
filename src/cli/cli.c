#include <errno.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "cli.h"

#include "../tunable/tunable.h"

static Cli_menu current_menu = NULL;

Cli_menu
cli_menu_alloc()
{
  return calloc(1, sizeof(struct cli_menu_t));
}

void
cli_menu_free(Cli_menu menu)
{
  free(menu);
}

void
cli_menu_add_menu(Cli_menu head, Cli_menu menu)
{
  Cli_entry entry = calloc(1, sizeof(cli_menu));
  entry->type = CLI_MENU;
  entry->u.menu = menu;

  head->num_entries++;
  head->entries = realloc(head->entries, head->num_entries * sizeof(Cli_entry));
  head->entries[head->num_entries - 1] = entry;
  menu->previous = head;
}

void
cli_menu_cleanup(Cli_menu entry)
{
  int i;
  Cli_menu tmp_menu;
  Cli_entry tmp_entry;

  // hang child menus off of parent
  for (i = 0; i < entry->num_entries; i++) {
    tmp_entry = entry->entries[i];
    if (tmp_entry != NULL && tmp_entry->type == CLI_MENU) {
      tmp_menu = tmp_entry->u.menu;
      cli_menu_add_menu(entry->previous, tmp_menu);
    }
  }

  // eliminate parent's references to this menu
  tmp_menu = entry->previous;
  if (tmp_menu != NULL) {
    for (i = 0; i < tmp_menu->num_entries; i++) {
      tmp_entry = tmp_menu->entries[i];
      if (tmp_entry->type == CLI_MENU && tmp_entry->u.menu == entry) {
        tmp_entry->u.menu = NULL;
        tmp_entry->type = CLI_NONE;
      }
    }
  }

  // free this menu's resources
  for (i = 0; i < entry->num_entries; i++) {
    tmp_entry = entry->entries[i];
    if (tmp_entry != NULL) {
      free(tmp_entry);
    }
  }

  free(entry->name);
  free(entry->prompt_header);
  free(entry->entries);

  cli_menu_free(entry);
}

Cli_menu
cli_menu_init(cli_menu_type type, const char *name, const char *prompt)
{
  Cli_menu rv = cli_menu_alloc();
  rv->type = type;
  rv->name = calloc(strlen(name) + 1, sizeof(char));
  strcpy(rv->name, name);
  rv->prompt_header = calloc(strlen(prompt) + 1, sizeof(char));
  strcpy(rv->prompt_header, prompt);

  return rv;
}

Cli_menu
cli_menu_init_menu_system()
{
  Cli_menu rv, tmp1, tmp2;
  rv = cli_menu_init(CLI_MENU, "Main Menu", "Edit synthesizer or FX");
  tmp1 = cli_menu_init(CLI_MENU, "Synth Menu", "Edit instruments");
  cli_menu_add_menu(rv, tmp1);

  tmp2 = cli_menu_init(CLI_MENU, "Instrument 0 Menu", "Enable/Disable or configure");
  cli_menu_add_menu(tmp1, tmp2);
  tmp2 = cli_menu_init(CLI_MENU, "Instrument 1 Menu", "Enable/Disable or configure");
  cli_menu_add_menu(tmp1, tmp2);

  tmp1 = cli_menu_init(CLI_MENU, "FX Menu", "Choose an effect to configure");
  cli_menu_add_menu(rv, tmp1);

  current_menu = rv;
  return rv;
}

#define INPUT_BUF_LEN (2 * (3 + DBL_MANT_DIG - DBL_MIN_EXP))
#define OUTPUT_BUF_LEN INPUT_BUF_LEN
static char input_buf_s[INPUT_BUF_LEN] = {0};

size_t
cli_input_poll(char *input_buf, size_t buf_len)
{
  static struct timeval tv;
  static fd_set readfds;

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);

  // don't care about writefds and exceptfds:
  select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

  if (FD_ISSET(STDIN_FILENO, &readfds)) {
    fgets(input_buf, buf_len, stdin);
    input_buf[strcspn(input_buf, "\r\n")] = '\0';
    return strlen(input_buf);
  }

  return 0;
}

static bool printed_entry_prompt = false;
static bool menu_or_entry = true; // true for menu
static Tunable current_entry = NULL;

void
cli_print_entry(Tunable entry)
{
  static char output_buf[OUTPUT_BUF_LEN];
  size_t len;

  if (!printed_entry_prompt) {
    printed_entry_prompt = true;
    // print prompt
    len = snprintf(output_buf, strlen(entry->name) + 9, "Adjust %s:\n", entry->name);

    // entry's current value
    switch (entry->param_type) {
    case TUNABLE_INT32:
      len += snprintf(output_buf + len, 10 + 12, "  current: %d\n", *(int32_t *)(entry->param));
      break;
    case TUNABLE_UINT32:
      len += snprintf(output_buf + len, 10 + 12, "  current: %u\n", *(uint32_t *)(entry->param));
      break;
    case TUNABLE_FLOAT:
      len += snprintf(output_buf + len, 3 + DBL_MANT_DIG - DBL_MIN_EXP + 12, "  current: %f\n", *(float *)(entry->param));
      break;
    case TUNABLE_DOUBLE:
      len += snprintf(output_buf + len, 3 + DBL_MANT_DIG - DBL_MIN_EXP + 12, "  current: %f\n", *(double *)(entry->param));
      break;
    default:
      break;
    }

    // entry's expected input range
    switch (entry->range_type) {
    case TUNABLE_RANGE_NONE:
      break;
    case TUNABLE_RANGE_0_127:
      len += snprintf(output_buf + len, 19, "  integer [0-127]: ");
      break;
    case TUNABLE_RANGE_0_1:
      len += snprintf(output_buf + len, 15, "  float [0-1]: ");
      break;
    case TUNABLE_RANGE_0_24000:
      len += snprintf(output_buf + len, 17, "  float [0-24000]: ");
      break;
    default:
      break;
    }

    printf("%s", output_buf);
  }
}

size_t
cli_entry_prompt_and_response(Tunable entry)
{
  cli_print_entry(entry);

  return cli_input_poll(input_buf_s, sizeof(INPUT_BUF_LEN));
}

void
cli_entry_entry_point()
{
  char *str_end;
  int32_t i_val = 0;
  uint32_t u_val = 0;
  float f_val = 0;
  double d_val = 0;
  if (cli_entry_prompt_and_response(current_entry) > 0) {
    printed_entry_prompt = false;
    errno = 0;
    switch (current_entry->param_type) {
    case TUNABLE_INT32:
      i_val = strtol(input_buf_s, &str_end, 10);
      break;
    case TUNABLE_UINT32:
      u_val = strtoul(input_buf_s, &str_end, 10);
      break;
    case TUNABLE_FLOAT:
      f_val = strtof(input_buf_s, &str_end);
      break;
    case TUNABLE_DOUBLE:
      d_val = strtod(input_buf_s, &str_end);
      break;
    default:
      break;
    }
    if (errno == EINVAL || errno == ERANGE) {

    }

    tunable_arg arg;
    switch (current_entry->param_type) {
    case TUNABLE_INT32:
      arg.i = i_val;
      break;
    case TUNABLE_UINT32:
      arg.u = u_val;
      break;
    case TUNABLE_FLOAT:
      arg.f = f_val;
      break;
    case TUNABLE_DOUBLE:
      arg.d = d_val;
      break;
    default:
      break;
    }
    tunable_param_write(current_entry, &arg);
    tunable_callback_invoke(current_entry);

    current_entry = NULL;
    menu_or_entry = true;
  }
}

void
cli_print_menu(Cli_menu menu)
{
  int i;
  Cli_entry entry;
  if (!menu->printed_prompt) {
    menu->printed_prompt = true;
    printf("%s\n%s\n%s", menu->name, menu->prompt_header, menu->previous == NULL ? "" : "  -1: previous menu\n");
    for (i = 0; i < menu->num_entries; i++) {
      printf("  %d: ", i); 
      entry = menu->entries[i];
      if (entry->type == CLI_MENU) {
        printf("menu %s\n", entry->u.menu->name);
      } else if (entry->type == CLI_ENTRY) {
        printf("adjust %s\n", entry->u.entry->name);
      }
    }
    fflush(stdout);
  }
}

void
cli_menu_prompt_and_response(Cli_menu menu)
{
  cli_print_menu(menu);

  if (cli_input_poll(input_buf_s, sizeof(INPUT_BUF_LEN)) > 0) {
    menu->printed_prompt = false;
  }
}

int32_t
cli_menu_prompt_and_response_int(Cli_menu menu)
{
  cli_menu_prompt_and_response(menu);
  char *str_end;
  return strtol(input_buf_s, &str_end, 10);
}

void
cli_menu_entry_point()
{
  errno = 0;
  int i, option = cli_menu_prompt_and_response_int(current_menu);
  if (errno == EINVAL) {
    return;
  }

  if (option == -1 && current_menu->previous != NULL) {
    current_menu = current_menu->previous;
  } else {
    for (i = 0; i < current_menu->num_entries; i++) {
      if (i == option) {
        if (current_menu->entries[i]->type == CLI_MENU) {
          current_menu = current_menu->entries[i]->u.menu;
        } else if (current_menu->entries[i]->type == CLI_ENTRY) {
          menu_or_entry = false;
          current_entry = current_menu->entries[i]->u.entry; 
        }
        break;
      }
    }
  }
}

void
cli_main_menu()
{
  cli_menu_entry_point();
}

void
cli_entry_point()
{
  if (menu_or_entry) {
    cli_menu_entry_point();
  } else {
    cli_entry_entry_point();
  }
}
