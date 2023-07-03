#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include "command.h"

#define READ_END  0
#define WRITE_END 1

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
single_command_exec(command *cmd)
{
    pid_t pid = fork();
    assert_true(pid >= 0, "Error forking in single_command_exec()");

    if (pid == 0)
        do_command_exec(cmd);
    else
    {
        int code;
        assert_true(waitpid(pid, &code, 0) != -1, "Error in waitpid() in single_command_exec()");
        return WEXITSTATUS(code);
    }

    return EXIT_SUCCESS;
}

int
pipe_list_exec(pipe_list *list)
{
    int previous_pipe[2], current_pipe[2];
    pid_t children[list->size];
    int final_exit_code = EXIT_FAILURE, code;
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
            final_exit_code &= WEXITSTATUS(code);
        else if (WIFSIGNALED(code))
            final_exit_code = EXIT_FAILURE;
    }

    return final_exit_code;
}
