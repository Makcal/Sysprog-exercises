#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include "pipe_list.h"

static void
program_exec(program *prg)
{
    if (prg->output_file_name != NULL)
    {
        int flags = O_WRONLY | O_CREAT;
        flags |= prg->is_appending_output ? O_APPEND : O_TRUNC;

        int file = open(prg->output_file_name, flags, S_IRUSR | S_IWUSR);
        assert_true(file != -1, "Error opening file in do_command_exec()");

        assert_true(dup2(file, STDOUT_FILENO) >= 0, "Error during dup2() in do_command_exec()");
        assert_true(close(file) >= 0, "Error closing file in do_command_exec()");
    }

    execvp(prg->argv[0], prg->argv);
    fputs("execvp() failed", stderr);
    exit(EXIT_FAILURE);
}

static void
program_free(program *prg)
{
    if (prg->argv)
    {
        for (size_t i = 0; prg->argv[i]; ++i)
            free(prg->argv[i]);

        free(prg->argv);
    }

    if (prg->output_file_name)
        free(prg->output_file_name);

    free(prg);
}

int
pipe_list_exec(list *pipe_list)
{
    int previous_pipe[2], current_pipe[2];
    pid_t children[pipe_list->size];
    int code = EXIT_SUCCESS;
    size_t i;

    for (i = 0; i < pipe_list->size; ++i)
    {
        if (i < pipe_list->size - 1)
            assert_true(pipe(current_pipe) != -1, "Error making pipe in pipe_list_exec()");

        pid_t pid = fork();
        assert_true(pid != -1, "Error forking in pipe_list_exec()");

        if (pid == 0)
        {
            if (i < pipe_list->size - 1)
                dup2(current_pipe[WRITE_END], STDOUT_FILENO);
            if (i > 0)
                dup2(previous_pipe[READ_END], STDIN_FILENO);

            close(current_pipe[READ_END]);
            program_exec(pipe_list->items[i]);
        } else
        {
            children[i] = pid;

            if (i > 0)
                close(previous_pipe[READ_END]);

            close(current_pipe[WRITE_END]);
            previous_pipe[READ_END] = current_pipe[READ_END];
        }
    }

    for (i = 0; i < pipe_list->size; ++i)
    {
        assert_true(waitpid(children[i], &code, 0) >= 0, "Error in waitpid() in pipe_list_exec()");
        code = WEXITSTATUS(code);
    }

    return code;
}

void
pipe_list_free(list *pipe_list)
{
    list_free(pipe_list, program_free);
}
