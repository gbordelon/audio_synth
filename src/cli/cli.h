#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stdlib.h>

#include "../lib/macros.h"

/*
 * TODO perhaps this should have two items, menu and entry. That might help with deciding how to
 * navigate the menu system and how to print the appropriate things.
 * e.g. The main menu is a menu, the synth menu is a menu, each instrument menu is a menu, but each instrument group of settings (envelope, lfo, etc) can have a menu of entries which can be tuneables.
 */
typedef enum {
  CLI_NONE,
  CLI_MENU,
  CLI_ENTRY,
  CLI_MENU_NUM
} cli_menu_type;

// forward decl
typedef struct cli_menu_t *Cli_menu;
typedef struct tunable_t *Tunable;

typedef struct cli_entry_t {
  cli_menu_type type;
  union {
    Cli_menu menu;
    Tunable entry;
  } u;
} cli_entry;

typedef cli_entry *Cli_entry;

typedef struct cli_menu_t {
  cli_menu_type type;
  char *name;

  bool printed_prompt;
  char *prompt_header;

  size_t num_entries;
  Cli_entry *entries;

  Cli_menu previous;
} cli_menu;

Cli_menu cli_menu_init_menu_system(bool instruments, bool fx, Cli_menu inst1, Cli_menu inst2, Cli_menu fxm);

Cli_menu cli_menu_init(cli_menu_type type, const char *name, const char *prompt);
void cli_menu_cleanup(Cli_menu menu);

void cli_menu_add_menu(Cli_menu head, Cli_menu menu);
void cli_menu_add_tunable(Cli_menu menu, Tunable entry);

/*
 * TODO
 * top menu should provide access to synth and fx chain
 *  synth menu should provide access to change/enable/disable instrument 1 or instrument 2
 *    instrument menu should provide access to individial tunable parameters
 *  fx chain menu should provide access to add or remove effects from graph
 *  fx chain menu should show fx graph and provide access to select an effect
 *    effect menu should provide access to individual tunable parameters
 */

size_t cli_input_alpha(const char *input_buf, size_t buf_len);
FTYPE cli_input_float(const char *input_buf, size_t buf_len);
int32_t cli_input_integer(const char *input_buf, size_t buf_len);

void cli_entry_point();

#endif
