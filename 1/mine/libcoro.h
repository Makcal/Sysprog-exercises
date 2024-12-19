#ifndef H_LIBCORO
#define H_LIBCORO

#include <ucontext.h>

struct coro {
    ucontext_t context;
    char stack[];
};

struct coro_list_node {
    struct coro_list_node *next, *prev;
    struct coro coro;
};

struct coro_list {
    struct coro_list_node *sentinel;
};

typedef struct {
    struct coro_list coros;
    struct coro_list_node *current_coro_node;
    ucontext_t main_context;
} loop_t;

/*
 * Initializes a loop structure.
 */
void loop_init(loop_t *);

/*
 * Save a loop as the global to be run later.
 */
void use_loop(loop_t *);

/*
 * Adds a task to a loop.
 */
void coro_create(loop_t *, void (*)(void *), void *);

/*
 * The loop's state is invalid after it finishes.
 */
void loop_start(void);

/*
 * Pauses execution to pass it to another task.
 */
void coro_yield(void);

#endif // H_LIBCORO

