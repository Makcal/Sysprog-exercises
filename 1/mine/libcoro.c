#include "libcoro.h"
#include <err.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <ucontext.h>

typedef struct coro coro_t;
typedef struct coro_list_node coro_list_node_t;

static const size_t STACK_SIZE = 8 * 1024 * 1024 * sizeof(char); // 8 MB
static loop_t *current_loop = NULL;

void loop_init(loop_t *loop) {
    coro_list_node_t *sentinel = malloc(sizeof(coro_list_node_t));
    if (!sentinel) {
        err(EXIT_FAILURE, "loop_init");
    }
    sentinel->next = sentinel->prev = sentinel;
    loop->coros.sentinel = sentinel;

    getcontext(&loop->main_context);
}

void use_loop(loop_t *loop) {
    current_loop = loop;
}

void coro_create(loop_t *loop, void (*func)(void *), void *args) {
    coro_list_node_t *new_node = malloc(sizeof(coro_list_node_t) + STACK_SIZE);
    if (!new_node) {
        err(EXIT_FAILURE, "coro_create");
    }

    getcontext(&new_node->coro.context);
    new_node->coro.context.uc_stack.ss_sp = new_node->coro.stack;
    new_node->coro.context.uc_stack.ss_size = STACK_SIZE;
    new_node->coro.context.uc_link = &loop->main_context;
    makecontext(&new_node->coro.context, (void *)func, 1, args);

    new_node->next = loop->coros.sentinel;
    new_node->prev = loop->coros.sentinel->prev;
    loop->coros.sentinel->prev->next = new_node;
    loop->coros.sentinel->prev = new_node;
}

void loop_start(void) {
    current_loop->current_coro_node = current_loop->coros.sentinel->next;
    while (current_loop->coros.sentinel->next != current_loop->coros.sentinel) {
        swapcontext(&current_loop->main_context, &current_loop->current_coro_node->coro.context);

        coro_list_node_t *old_node = current_loop->current_coro_node;
        current_loop->current_coro_node = current_loop->current_coro_node->next;
        if (current_loop->current_coro_node == current_loop->coros.sentinel)
            current_loop->current_coro_node = current_loop->current_coro_node->next;

        old_node->prev->next = old_node->next;
        old_node->next->prev = old_node->prev;
        free(old_node);
    }
    free(current_loop->coros.sentinel);
}

void coro_yield(void) {
    if (current_loop->coros.sentinel == current_loop->coros.sentinel->next->next)
        return;

    coro_t *old_coro = &current_loop->current_coro_node->coro;
    current_loop->current_coro_node = current_loop->current_coro_node->next;
    if (current_loop->current_coro_node == current_loop->coros.sentinel)
        current_loop->current_coro_node = current_loop->current_coro_node->next;
    swapcontext(&old_coro->context, &current_loop->current_coro_node->coro.context);
}

