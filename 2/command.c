#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include "command.h"

void
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
    if (cmd->argv != NULL)
    {
        for (size_t i = 0; cmd->argv[i] != NULL; ++i)
            free(cmd->argv[i]);

        free(cmd->argv);
    }

    if (cmd->output_file_name != NULL)
        free(cmd->output_file_name);

    free(cmd);
}
