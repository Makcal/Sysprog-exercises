#ifndef MYPROJECT_COMMAND_H
#define MYPROJECT_COMMAND_H

#include <stdbool.h>
#include "utils.h"

#define ENTRY_AND       0
#define ENTRY_OR        1
#define ENTRY_PIPE_LIST 2

typedef struct command command;
typedef struct entry entry;
typedef struct list pipe_list;
typedef struct list entry_list;

struct command
{
    char **argv;

    bool is_appending_output;
    char *output_file_name;
};

struct entry
{
    void *item;
    int item_type;
};

int
pipe_list_exec(pipe_list *list);

int entry_list_exec(entry_list *list);

#endif //MYPROJECT_COMMAND_H
