#include <unistd.h>
#include "evaluator.h"
#include "parser.h"

static char
**init_argv(list *tokens, size_t left_bound, size_t right_bound)
{
    char **argv = malloc((2 + right_bound - left_bound) * sizeof(char *));
    argv[right_bound + 1] = NULL;

    for (size_t i = left_bound; i <= right_bound; ++i)
        argv[i - left_bound] = strdup(tokens->items[i]);

    return argv;
}

static program
*init_program(list *tokens, size_t left_bound, size_t right_bound)
{
    program *prg = malloc(sizeof(program));

    for (size_t i = left_bound; i <= right_bound; ++i)
    {
        char *token = tokens->items[i];
        if (is_redirection(token))
        {
            prg->argv = init_argv(tokens, left_bound, i - 1);
            prg->is_appending_output = is_appending(token);
            prg->output_file_name = strdup(tokens->items[i + 1]);

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
        char *token = tokens->items[i];
        if (is_pipeline(token))
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
        char *operator = tokens->items[right_bound];
        entry->next_operator_type = str_equal(operator, AND_OPERATOR) ? ENTRY_AND : ENTRY_OR;
    }

    return entry;
}

int
evaluate(list *tokens)
{
    if (!tokens->size)
        return EXIT_SUCCESS;

    list *command = list_init();
    bool is_background_execution = false;
    size_t left_bound = 0;
    int return_code = EXIT_SUCCESS;

    for (size_t i = 0; i < tokens->size; ++i)
    {
        char *token = tokens->items[i];

        if (is_logical(token))
        {
            list_add(command, init_command_entry(tokens, left_bound, i, false));
            left_bound = i + 1;
        } else if (i == tokens->size - 1)
        {
            if (is_background(token))
                is_background_execution = true;
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
            return_code = command_exec(command);
    } else
        return_code = command_exec(command);

    command_free(command);
    return return_code;
}
