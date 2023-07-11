#include "command.h"

#define READ_END    0
#define WRITE_END   1

typedef struct
{
    command **items;
    size_t size, capacity;
} pipe_list;

int
pipe_list_exec(pipe_list *list);
