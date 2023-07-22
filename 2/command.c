#include "command.h"

static void
command_entry_free(command_entry *entry)
{
    pipe_list_free(entry->pipe_list);
    free(entry);
}

exit_context
command_exec(list *command)
{
    exit_context context;
    memset(&context, 0, sizeof(exit_context));

    if (!command->size)
        return context;

    int execution_mode = RUN_AND_CHAIN;
    command_entry *entry;

    for (size_t i = 0; i < command->size; ++i)
    {
        entry = command->items[i];

        if (execution_mode == RUN_AND_CHAIN)
        {
            context = pipe_list_exec(entry->pipe_list);

            if (context.is_terminate)
                goto out;
        }
        else if (execution_mode == EXEC_AFTER_OR)
        {
            context = pipe_list_exec(entry->pipe_list);

            if (context.is_terminate)
                goto out;

            execution_mode = RUN_AND_CHAIN;
        }

        if (context.exit_code != EXIT_SUCCESS)
            execution_mode = AND_CHAIN_FAILED;

        switch (entry->next_operator_type)
        {
            case ENTRY_AND:
                if (execution_mode == CAUGHT_OR_ONLY)
                    execution_mode = RUN_AND_CHAIN;
                break;
            case ENTRY_OR:
                execution_mode |= CAUGHT_OR_ONLY;
                break;
            case ENTRY_NULL:
                goto out;
        }
    }

    out:
    return context;
}

void
command_free(list *command)
{
    list_free(command, command_entry_free);
}
