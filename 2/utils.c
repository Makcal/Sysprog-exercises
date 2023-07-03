#include "utils.h"
#include <stdio.h>

void
assert_true(bool condition, const char *error_message)
{
    if (!condition)
    {
        perror(error_message);
        exit(EXIT_FAILURE);
    }
}

void
*checked_realloc(void *ptr, size_t new_capacity)
{
    void *updated_ptr = realloc(ptr, new_capacity * sizeof(void *));
    assert_true(updated_ptr != NULL, "Error reallocating ptr");
    return updated_ptr;
}

list
list_new(void)
{
    list list = {
            .size = 0,
            .capacity = LIST_CAPACITY,
            .items = malloc(LIST_CAPACITY * sizeof(void *))
    };

    return list;
}

void
list_add(list *list, void *item)
{
    if (list->size == list->capacity)
    {
        list->capacity += LIST_CAPACITY;
        list->items = checked_realloc(list->items, list->capacity);
    }

    list->items[list->size++] = item;
}