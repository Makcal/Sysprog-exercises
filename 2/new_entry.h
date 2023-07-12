#include "command.h"

#define RUN_AND_CHAIN       000
#define AND_CHAIN_FAILED    001
#define CAUGHT_OR_ONLY      010
#define EXEC_AFTER_OR       (CAUGHT_OR_ONLY | AND_CHAIN_FAILED)

#define ENTRY_NULL          0
#define ENTRY_AND           1
#define ENTRY_OR            2

typedef struct
{
    list *pipe_list;
    int next_operator_type;
} new_entry;

int
entry_list_exec(list *entry_list);

void
new_entry_free(new_entry *ent);