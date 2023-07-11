#include <stdlib.h>
#include "utils.h"

#define RUN_AND_CHAIN       000
#define AND_CHAIN_FAILED    001
#define CAUGHT_OR_ONLY      010
#define EXEC_AFTER_OR       (CAUGHT_OR_ONLY | AND_CHAIN_FAILED)

#define ENTRY_AND       0
#define ENTRY_OR        1
#define ENTRY_PIPE_LIST 2

typedef struct
{
    void *item;
    int item_type;
} entry;

void
entry_free(entry *entry);

int
entry_list_exec(list *list);
