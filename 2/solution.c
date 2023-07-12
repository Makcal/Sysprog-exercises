#include <stdio.h>
#include <string.h>
#include "new_entry.h"
//#include "../utils/heap_help/heap_help.h"

int
main(void)
{
    char **tr = calloc(3, sizeof(char *));
    tr[0] = calloc(3, sizeof(char));
    strcpy(tr[0], "ls");
    tr[1] = calloc(4, sizeof(char));
    strcpy(tr[1], "/400");

    char **fl = calloc(2, sizeof(char *));
    fl[0] = calloc(6, sizeof(char));
    strcpy(fl[0], "true");

    char **fl2 = calloc(2, sizeof(char *));
    fl2[0] = calloc(6, sizeof(char));
    strcpy(fl2[0], "false");

    command *cmd1 = calloc(1, sizeof(command));
    cmd1->argv = tr;

    command *cmd2 = calloc(1, sizeof(command));
    cmd2->argv = fl;

    command *cmd3 = calloc(1, sizeof(command));
    cmd3->argv = fl2;

    list *lst1 = list_init();

    list_add(lst1, cmd1);
    list_trim(lst1);

    list *lst2 = list_init();

    list_add(lst2, cmd2);
    list_trim(lst2);

    list *lst3 = list_init();

    list_add(lst3, cmd3);
    list_trim(lst3);

    new_entry *elist1 = malloc(sizeof(new_entry));
    elist1->pipe_list = lst1;
    elist1->next_operator_type = ENTRY_NULL;

    new_entry *elist2 = malloc(sizeof(new_entry));
    elist2->pipe_list = lst2;
    elist2->next_operator_type = ENTRY_AND;

    new_entry *elist3 = malloc(sizeof(new_entry));
    elist3->pipe_list = lst3;
    elist3->next_operator_type = ENTRY_OR;

    list *final_list = list_init();
    list_add(final_list, elist2);
    list_add(final_list, elist3);
    list_add(final_list, elist1);
    list_trim(final_list);

    int code = entry_list_exec(final_list);
    printf("%d\n", code);

    list_free(final_list, new_entry_free);

//    heaph_get_alloc_count();
    return code;
}
