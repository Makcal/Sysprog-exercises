#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include "command.h"

#define READ_END            0
#define WRITE_END           1

#define RUN_AND_CHAIN       000
#define AND_CHAIN_FAILED    001
#define CAUGHT_OR_ONLY      010
#define EXEC_AFTER_OR       (CAUGHT_OR_ONLY | AND_CHAIN_FAILED)

static void
do_command_exec(command *cmd)
{
    if (cmd->output_file_name != NULL)
    {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->is_appending_output ? O_APPEND : O_TRUNC;

        int file = open(cmd->output_file_name, flags, S_IRUSR | S_IWUSR);
        assert_true(file != -1, "Error opening file in do_command_exec()");

        assert_true(dup2(file, STDOUT_FILENO) >= 0, "Error during dup2() in do_command_exec()");
        assert_true(close(file) >= 0, "Error closing file in do_command_exec()");
    }

    execvp(cmd->argv[0], cmd->argv);
    fputs("execvp() failed", stderr);
    exit(EXIT_FAILURE);
}

int
pipe_list_exec(pipe_list *list)
{
    int previous_pipe[2], current_pipe[2];
    pid_t children[list->size];
    int code = EXIT_FAILURE;
    size_t i;

    for (i = 0; i < list->size; ++i)
    {
        if (i < list->size - 1)
            assert_true(pipe(current_pipe) != -1, "Error making pipe in pipe_list_exec()");

        pid_t pid = fork();
        assert_true(pid != -1, "Error forking in pipe_list_exec()");

        if (pid == 0)
        {
            if (i < list->size - 1)
                dup2(current_pipe[WRITE_END], STDOUT_FILENO);
            if (i > 0)
                dup2(previous_pipe[READ_END], STDIN_FILENO);

            close(current_pipe[READ_END]);
            do_command_exec(list->items[i]);
        } else
        {
            children[i] = pid;

            if (i > 0)
                close(previous_pipe[READ_END]);

            close(current_pipe[WRITE_END]);
            previous_pipe[READ_END] = current_pipe[READ_END];
        }
    }

    for (i = 0; i < list->size; ++i)
    {
        assert_true(waitpid(children[i], &code, 0) >= 0, "Error in waitpid() in pipe_list_exec()");

        if (WIFEXITED(code))
            code = WEXITSTATUS(code);
        else if (WIFSIGNALED(code))
            code = EXIT_FAILURE;
    }

    return code;
}

int entry_list_exec(entry_list *list)
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
