#include "new_entry.h"

int
entry_list_exec(list *entry_list)
{
    int code = EXIT_SUCCESS;
    int execution_mode = RUN_AND_CHAIN;
    new_entry *ent;

    for (size_t i = 0; i < entry_list->size; ++i)
    {
        ent = entry_list->items[i];

        if (execution_mode == RUN_AND_CHAIN)
            code = pipe_list_exec(ent->pipe_list);
        else if (execution_mode == EXEC_AFTER_OR)
        {
            code = pipe_list_exec(ent->pipe_list);
            execution_mode = RUN_AND_CHAIN;
        }

        if (code != EXIT_SUCCESS)
            execution_mode = AND_CHAIN_FAILED;

        switch (ent->next_operator_type)
        {
            case ENTRY_OR:
                execution_mode |= CAUGHT_OR_ONLY;
                break;
            case ENTRY_NULL:
                goto out;
        }
    }

    out:
    return code;
}

void
new_entry_free(new_entry *ent)
{
    if (ent->pipe_list)
        list_free(ent->pipe_list, command_free);

    free(ent);
}