#include <stdbool.h>
#include <stdlib.h>

#define LIST_CAPACITY 4

void assert_true(bool condition, const char *error_message);

void *checked_realloc(void *ptr, size_t new_capacity);

#define list_new()                                                              \
{                                                                               \
    .size = 0,                                                                  \
    .capacity = LIST_CAPACITY,                                                  \
    .items = malloc(LIST_CAPACITY * sizeof(void *))                             \
}

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
