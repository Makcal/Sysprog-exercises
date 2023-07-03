#include <stdio.h>
#include <string.h>
#include "command.h"

int
main(void)
{
    char *tr[3] = {"ls", "/400", NULL};
    char *fl[2] = {"false", NULL};

    command cmd1 = {
            .argv = tr,
            .output_file_name = NULL,
            .is_appending_output = false
    };

    command cmd2 = {
            .argv = fl,
            .output_file_name = NULL,
            .is_appending_output = false
    };

    pipe_list lst1 = list_new();
    list_add(&lst1, &cmd1);

    pipe_list lst2 = list_new();
    list_add(&lst2, &cmd2);

    int res = pipe_list_exec(&lst1);
    printf("pipe_list: %d\n", res);

    entry and = {
            .item = "&&",
            .item_type = ENTRY_AND
    };

    entry or = {
            .item = "||",
            .item_type = ENTRY_OR
    };

    entry elist1 = {
            .item = &lst1,
            .item_type = ENTRY_PIPE_LIST
    };

    entry elist2 = {
            .item = &lst2,
            .item_type = ENTRY_PIPE_LIST
    };

    entry_list final_list = list_new();
    list_add(&final_list, &elist2);
    list_add(&final_list, &and);
    list_add(&final_list, &elist2);
    list_add(&final_list, &and);
    list_add(&final_list, &elist2);
    list_add(&final_list, &or);
    list_add(&final_list, &elist2);
    list_add(&final_list, &and);
    list_add(&final_list, &elist1);

    int code = entry_list_exec(&final_list);
    printf("%d\n", code);

    return code;
}