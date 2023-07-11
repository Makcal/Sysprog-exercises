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

list *
list_init() {
    list *list_ptr = malloc(sizeof(list));
    list_ptr->size = 0;
    list_ptr->capacity = LIST_CAPACITY;
    list_ptr->items = malloc(LIST_CAPACITY * sizeof(void *));
    return list_ptr;
}

void
list_add(list *list_ptr, void *item) {
    if (list_ptr->size == list_ptr->capacity)
    {
        list_ptr->capacity += LIST_CAPACITY;
        list_ptr->items= checked_realloc(list_ptr->items, list_ptr->capacity);
    }

    list_ptr->items[list_ptr->size++] = item;
}

void
list_trim(list *list_ptr)
{
    list_ptr->items = checked_realloc(list_ptr->items, list_ptr->size);
    list_ptr->capacity = list_ptr->size;
}
