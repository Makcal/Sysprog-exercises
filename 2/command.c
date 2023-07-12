#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include "command.h"

static void
command_exec(command *cmd)
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

void
command_free(command *cmd)
{
    if (cmd->argv)
    {
        for (size_t i = 0; cmd->argv[i]; ++i)
            free(cmd->argv[i]);

        free(cmd->argv);
    }

    if (cmd->output_file_name)
        free(cmd->output_file_name);

    free(cmd);
}

int
pipe_list_exec(list *list)
{
    int previous_pipe[2], current_pipe[2];
    pid_t children[list->size];
    int code = EXIT_SUCCESS;
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
            command_exec(list->items[i]);
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
        code = WEXITSTATUS(code);
    }

    return code;
}
