#include "command.h"

static void
command_entry_free(command_entry *entry)
{
    pipe_list_free(entry->pipe_list);
    free(entry);
}

int
command_exec(list *command)
{
    int code = EXIT_SUCCESS;
    int execution_mode = RUN_AND_CHAIN;
    command_entry *entry;

    for (size_t i = 0; i < command->size; ++i)
    {
        entry = command->items[i];

        if (execution_mode == RUN_AND_CHAIN)
            code = pipe_list_exec(entry->pipe_list);
        else if (execution_mode == EXEC_AFTER_OR)
        {
            code = pipe_list_exec(entry->pipe_list);
            execution_mode = RUN_AND_CHAIN;
        }

        if (code != EXIT_SUCCESS)
            execution_mode = AND_CHAIN_FAILED;

        switch (entry->next_operator_type)
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
command_free(list *command)
{
    list_free(command, command_entry_free);
}
