#ifndef MYPROJECT_COMMAND_H
#define MYPROJECT_COMMAND_H

#include <stdbool.h>
#include "utils.h"

#define ENTRY_COMMAND   0
#define ENTRY_AND       1
#define ENTRY_OR        2
#define ENTRY_EXIT_CODE 3

typedef struct command command;
typedef struct list pipe_list;
typedef struct command_list command_list;

struct command
{
    char **argv;

    bool is_appending_output;
    char *output_file_name;
};


struct command_list
{
    void *entries;
    int *entries_types;
    size_t size;
};

int
single_command_exec(command *cmd);

int
pipe_list_exec(pipe_list *list);

//void
//command_list_add(command_list *list, void *entry, int type);
//
//void
//command_list_exec(command_list *list);

#endif //MYPROJECT_COMMAND_H
