#include <stdio.h>
#include <string.h>
#include "command.h"
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

    program *prg1 = calloc(1, sizeof(program));
    prg1->argv = tr;

    program *prg2 = calloc(1, sizeof(program));
    prg2->argv = fl;

    program *prg3 = calloc(1, sizeof(program));
    prg3->argv = fl2;

    list *lst1 = list_init();

    list_add(lst1, prg1);
    list_trim(lst1);

    list *lst2 = list_init();

    list_add(lst2, prg2);
    list_trim(lst2);

    list *lst3 = list_init();

    list_add(lst3, prg3);
    list_trim(lst3);

    command_entry *elist1 = malloc(sizeof(command_entry));
    elist1->pipe_list = lst1;
    elist1->next_operator_type = ENTRY_NULL;

    command_entry *elist2 = malloc(sizeof(command_entry));
    elist2->pipe_list = lst2;
    elist2->next_operator_type = ENTRY_AND;

    command_entry *elist3 = malloc(sizeof(command_entry));
    elist3->pipe_list = lst3;
    elist3->next_operator_type = ENTRY_OR;

    list *final_list = list_init();
    list_add(final_list, elist2);
    list_add(final_list, elist3);
    list_add(final_list, elist1);
    list_trim(final_list);

    int code = command_exec(final_list);
    printf("%d\n", code);

    command_free(final_list);

//    heaph_get_alloc_count();
    return code;
}
