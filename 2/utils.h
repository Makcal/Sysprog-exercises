#ifndef MYPROJECT_UTILS_H
#define MYPROJECT_UTILS_H

#include <stdbool.h>
#include <stdlib.h>

#define LIST_CAPACITY 4

typedef struct list list;

struct list
{
    void **items;
    size_t size, capacity;
};

list
list_new(void);

void
list_add(list *list, void *item);

void assert_true(bool condition, const char *error_message);

void *checked_realloc(void *ptr, size_t new_capacity);

#endif //MYPROJECT_UTILS_H
