#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "command.h"

static void
print_prompt(void)
{
    // Get username
    const long USERNAME_LENGTH = sysconf(_SC_LOGIN_NAME_MAX);
    char username[USERNAME_LENGTH];
    getlogin_r(username, USERNAME_LENGTH);

    // Get hostname
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);

    // Get current working directory
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);

    // Get $HOME path
    const char *home_path = getenv("HOME");
    size_t home_path_length = strlen(home_path);

    // Get privileges
    char privilege = geteuid() ? '$' : '#';

    // If a path is inside home directory, then replace home path with "~"
    if (strncmp(home_path, cwd, home_path_length) == 0 && cwd[home_path_length] == '/')
        printf("[%s@%s ~%s]%c ", username, hostname, cwd + home_path_length, privilege);
    else
        printf("[%s@%s %s]%c ", username, hostname, cwd, privilege);
}

int
main(void)
{
    print_prompt();

    char *argv1[3] = {"cat", "d", NULL};
    char *argv2[4] = {"ls", "-a", "/", NULL};
    char *argv3[3] = {"grep", "lib", NULL};

    command cmd1;
    memset(&cmd1, 0, sizeof(cmd1));

    command cmd2;
    memset(&cmd2, 0, sizeof(cmd2));

    command cmd3;
    memset(&cmd3, 0, sizeof(cmd3));

    cmd1.argv = argv1;

    cmd2.argv = argv2;

    cmd3.argv = argv3;
    cmd3.output_file_name = "test.txt";


    pipe_list list = list_new();
    list_add(&list, &cmd1);
    list_add(&list, &cmd2);
    list_add(&list, &cmd3);

    pipe_list_exec(&list);

    return 0;
}