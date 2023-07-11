#include <stdio.h>
#include <string.h>
#include "pipe_list.h"
#include "entry.h"

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
    strcpy(fl[0], "false");

    command *cmd1 = calloc(1, sizeof(command));
    cmd1->argv = tr;

    command *cmd2 = calloc(1, sizeof(command));
    cmd2->argv = fl;

    pipe_list *lst1;
    list_init(lst1, pipe_list);

    list_add(lst1, cmd1);
    list_trim(lst1);

    pipe_list *lst2;
    list_init(lst2, pipe_list);

    list_add(lst2, cmd2);
    list_trim(lst2);

    int res = pipe_list_exec(lst1);
    printf("pipe_list: %d\n", res);

    entry *and = malloc(sizeof(entry));
    and->item = NULL;
    and->item_type = ENTRY_AND;

    entry *or = malloc(sizeof(entry));
    and->item = NULL;
    and->item_type = ENTRY_OR;

    entry *elist1 = malloc(sizeof(entry));
    elist1->item = lst1;
    elist1->item_type = ENTRY_PIPE_LIST;

    entry *elist2 = malloc(sizeof(entry));
    elist1->item = lst2;
    elist1->item_type = ENTRY_PIPE_LIST;

    entry_list *final_list;
    list_init(final_list, entry_list);
    list_add(final_list, elist2);
    list_add(final_list, and);
//    list_add(final_list, elist2);
//    list_add(final_list, and);
//    list_add(final_list, elist2);
//    list_add(final_list, or);
//    list_add(final_list, elist2);
//    list_add(final_list, and);
    list_add(final_list, elist1);

    list_trim(final_list);

    int code = entry_list_exec(final_list);
    printf("%d\n", code);

    list_free(final_list, entry_free);
    return code;
}
