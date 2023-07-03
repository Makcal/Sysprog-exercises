#include <stdbool.h>
#include "utils.h"

typedef struct
{
    char **argv;

    bool is_appending_output;
    char *output_file_name;
} command;

void
command_exec(command *cmd);

void
command_free(command *cmd);
