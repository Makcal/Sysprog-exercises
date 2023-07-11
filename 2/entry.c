#include <stdio.h>
#include "entry.h"
#include "command.h"

void
entry_free(entry *entry)
{
    switch (entry->item_type)
    {
        case ENTRY_AND:
        case ENTRY_OR:
            break;  // entry->item for them is NULL
        case ENTRY_PIPE_LIST:
            list_free((list *) entry->item, command_free);
    }

    free(entry);
}

int entry_list_exec(list *list)
{
    int code = EXIT_SUCCESS;
    int execution_mode = RUN_AND_CHAIN;
    entry *ent;

    for (size_t i = 0; i < list->size; ++i)
    {
        ent = list->items[i];
        switch (ent->item_type)
        {
            case ENTRY_PIPE_LIST:
                if (execution_mode == RUN_AND_CHAIN)
                    code = pipe_list_exec(ent->item);
                else if (execution_mode == EXEC_AFTER_OR)
                {
                    code = pipe_list_exec(ent->item);
                    execution_mode = RUN_AND_CHAIN;
                }

                if (code != EXIT_SUCCESS)
                    execution_mode = AND_CHAIN_FAILED;

                break;
            case ENTRY_AND:
                break;
            case ENTRY_OR:
                execution_mode |= CAUGHT_OR_ONLY;
                break;

            default:
                perror("Invalid entry type");
                exit(EXIT_FAILURE);
        }
    }

    return code;
}
