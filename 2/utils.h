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

#define list_init(list_ptr)                                                     \
do {                                                                            \
    (list_ptr) = malloc(sizeof(list));                                          \
    (list_ptr)->size = 0;                                                       \
    (list_ptr)->capacity = LIST_CAPACITY;                                       \
    (list_ptr)->items = malloc(LIST_CAPACITY * sizeof(void *));                 \
} while (false)

#define list_add(list, item)                                                    \
do {                                                                            \
    if ((list)->size == (list)->capacity)                                       \
    {                                                                           \
        (list)->capacity += LIST_CAPACITY;                                      \
        (list)->items = checked_realloc((list)->items, (list)->capacity);       \
    }                                                                           \
                                                                                \
    (list)->items[(list)->size++] = (item);                                     \
} while (false)

#define list_trim(list)                                                         \
do {                                                                            \
    (list)->items = checked_realloc((list)->items, (list)->size);               \
    (list)->capacity = (list)->size;                                            \
} while (false)

#define list_free(list, item_cleaner)                                           \
do {                                                                            \
    for (size_t i = 0; i < (list)->size; ++i)                                   \
        (item_cleaner)((list)->items[i]);                                       \
    free((list));                                                               \
} while (false)
