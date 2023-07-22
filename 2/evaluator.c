#include <unistd.h>
#include "evaluator.h"
#include "parser.h"

static char
**init_argv(list *tokens, size_t left_bound, size_t right_bound)
{
    char **argv = malloc((2 + right_bound - left_bound) * sizeof(char *));
    argv[right_bound - left_bound + 1] = NULL;

    for (size_t i = left_bound; i <= right_bound; ++i)
    {
        token *tok = tokens->items[i];
        argv[i - left_bound] = strdup(tok->value);
    }

    return argv;
}

static program
*init_program(list *tokens, size_t left_bound, size_t right_bound)
{
    program *prg = malloc(sizeof(program));

    for (size_t i = left_bound; i <= right_bound; ++i)
    {
        token *tok = tokens->items[i];
        if (is_redirection(tok->type))
        {
            prg->argv = init_argv(tokens, left_bound, i - 1);
            prg->is_appending_output = is_redirection_append(tok->type);

            token *file_token =  tokens->items[i + 1];
            prg->output_file_name = strdup(file_token->value);

            return prg;
        }
    }

    prg->argv = init_argv(tokens, left_bound, right_bound);
    prg->is_appending_output = false;
    prg->output_file_name = NULL;

    return prg;
}

static list
*init_pipe_list(list *tokens, size_t left_bound, size_t right_bound)
{
    list *pipe_list = list_init();
    size_t program_left_bound = left_bound;

    for (size_t i = left_bound; i <= right_bound; ++i)
    {
        token *tok = tokens->items[i];
        if (is_pipeline(tok->type))
        {
            list_add(pipe_list, init_program(tokens, program_left_bound, i - 1));
            program_left_bound = i + 1;
        } else if (i == right_bound)
            list_add(pipe_list, init_program(tokens, program_left_bound, i));
    }

    list_trim(pipe_list);
    return pipe_list;
}

static command_entry
*init_command_entry(list *tokens, size_t left_bound, size_t right_bound, bool is_terminate)
{
    command_entry *entry = malloc(sizeof(command_entry));

    if (is_terminate)
    {
        entry->pipe_list = init_pipe_list(tokens, left_bound, right_bound);
        entry->next_operator_type = ENTRY_NULL;
    }
    else
    {
        entry->pipe_list = init_pipe_list(tokens, left_bound, right_bound - 1);

        token *tok = tokens->items[right_bound];
        int operator_type = tok->type;

        entry->next_operator_type = is_logical_and(operator_type) ? ENTRY_AND : ENTRY_OR;
    }

    return entry;
}

exit_context
evaluate(list *tokens)
{
    exit_context context;
    context.exit_code = EXIT_NO_CODE;
    context.is_terminate = false;

    if (!tokens->size)
        return context;

    list *command = list_init();
    bool is_background_execution = false;
    size_t left_bound = 0;

    for (size_t i = 0; i < tokens->size; ++i)
    {
        token *tok = tokens->items[i];
        if (is_logical(tok->type))
        {
            list_add(command, init_command_entry(tokens, left_bound, i, false));
            left_bound = i + 1;
        } else if (i == tokens->size - 1)
        {
            if (is_background_execution(tok->type))
            {
                is_background_execution = true;
                list_add(command, init_command_entry(tokens, left_bound, i - 1, true));
            }

            else
                list_add(command, init_command_entry(tokens, left_bound, i, true));
        }

    }

    list_trim(command);

    if (is_background_execution)
    {
        pid_t pid = fork();
        assert_true(pid >= 0, "Error forking in evaluate()");

        if (pid == 0)
            context = command_exec(command);
    } else
        context = command_exec(command);

    command_free(command);
    return context;
}
