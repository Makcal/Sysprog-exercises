#include <stdbool.h>
#include "utils.h"

#define READ_END    0
#define WRITE_END   1

typedef struct
{
    char **argv;

    bool is_appending_output;
    char *output_file_name;
} program;

int
pipe_list_exec(list *pipe_list);

void
pipe_list_free(list *pipe_list);
