#include <stdbool.h>
#include <stdlib.h>
#pragma once

#define LIST_CAPACITY 4

void assert_true(bool condition, const char *error_message);

void *checked_realloc(void *ptr, size_t new_capacity);

typedef struct
{
    size_t size, capacity;
    void **items;
} list;

list *
list_init();

void
list_add(list *list_ptr, void *item);

void
list_trim(list *list_ptr);

#define list_free(list, item_cleaner)                                           \
do {                                                                            \
    for (size_t i = 0; i < (list)->size; ++i)                                   \
        (item_cleaner)((list)->items[i]);                                       \
                                                                                \
    free((list)->items);                                                        \
    free((list));                                                               \
} while (false)
