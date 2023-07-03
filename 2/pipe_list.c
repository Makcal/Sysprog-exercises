#include <unistd.h>
#include <sys/wait.h>
#include "pipe_list.h"

int
pipe_list_exec(pipe_list *list)
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

void
pipe_list_free(pipe_list *list)
{
    if (list->size > 0)
    {
        for (size_t i = 0; list->items[i] != NULL; ++i)
            command_free(list->items[i]);
    }

    free(list);
}
