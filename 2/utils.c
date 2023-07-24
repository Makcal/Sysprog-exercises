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
list_init()
{
    list *list_ptr = malloc(sizeof(list));
    list_ptr->size = 0;
    list_ptr->capacity = LIST_CAPACITY;
    list_ptr->items = calloc(1, LIST_CAPACITY * sizeof(void *));
    return list_ptr;
}

void
list_add(list *list_ptr, void *item)
{
    if (list_ptr->size == list_ptr->capacity)
    {
        list_ptr->capacity += LIST_CAPACITY;
        list_ptr->items = checked_realloc(list_ptr->items, list_ptr->capacity);
    }

    list_ptr->items[list_ptr->size++] = item;
}

void
list_trim(list *list_ptr)
{
    if (!list_ptr || !list_ptr->size)
        return;  // Ignore trim in case list_ptr->size == 0

    list_ptr->items = checked_realloc(list_ptr->items, list_ptr->size);
    list_ptr->capacity = list_ptr->size;
}

string_builder
*string_builder_init()
{
    return calloc(1, sizeof(string_builder));
}

void
string_builder_append(string_builder *builder, char c)
{
    if (!builder->content)
        builder->content = malloc(sizeof(char));
    else
    {
        char *tmp = realloc(builder->content, (builder->size + 1) * sizeof(char));
        assert_true(tmp != NULL, "Error reallocating string_builder during append");
        builder->content = tmp;
    }

    builder->content[builder->size++] = c;
}

char
*to_string(string_builder *builder)
{
    char *tmp = realloc(builder->content, (builder->size + 1) * sizeof(char));
    assert_true(tmp != NULL, "Error reallocating string_builder during to_string");
    tmp[builder->size] = '\0';

    return strdup(builder->content);
}

void
string_builder_free(string_builder *builder)
{
    if (builder->content)
        free(builder->content);

    free(builder);
}
