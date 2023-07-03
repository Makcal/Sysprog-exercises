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
