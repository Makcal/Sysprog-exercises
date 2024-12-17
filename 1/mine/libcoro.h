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

void loop_init(loop_t *);

void set_loop(loop_t *);

void coro_create(loop_t *, void (*)(void *), void *);

void loop_start(void);

void coro_yield(void);

#endif // H_LIBCORO

