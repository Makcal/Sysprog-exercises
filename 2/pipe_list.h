#include <stdbool.h>
#include "utils.h"

#pragma once

#define READ_END    0
#define WRITE_END   1

#define EXIT_NO_CODE (-1)

typedef struct
{
    char **argv;

    bool is_appending_output;
    char *output_file_name;
} program;

typedef struct
{
    int exit_code;
    bool is_terminate;
} exit_context;

exit_context
pipe_list_exec(list *pipe_list);

void
pipe_list_free(list *pipe_list);
