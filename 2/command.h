#include <stdbool.h>
#include "utils.h"

#define READ_END    0
#define WRITE_END   1

typedef struct
{
    char **argv;

    bool is_appending_output;
    char *output_file_name;
} command;

void
command_free(command *cmd);

int
pipe_list_exec(list *list);
