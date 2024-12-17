#include <err.h>
#include <stddef.h>
#include <stdio.h>

#include "libcoro.h"

void coroutine(void *data) {
    int n = *(int *)data;
    printf("a: %d\n", n);
    coro_yield();
    printf("b: %d\n", n);
}

int main(int argc, char **) {
    if (argc <= 2) {
        fprintf(stderr, "Not enough arguments!\n");
        return 1;
    }

    loop_t loop;
    loop_init(&loop);
    set_loop(&loop);

    int data[] = {3, 2, 1};
    for (int i = 0; i < 3; i++)
        coro_create(&loop, coroutine, &data[i]);

    loop_start();
}

