#include "thread_pool.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>

typedef struct thread_task thread_task;
typedef struct thread_pool thread_pool;
typedef struct thread_args thread_args;
typedef struct queue queue;

enum
{
    TASK_STATUS_FREE = 0,
    TASK_STATUS_PUSHED = 1,
    TASK_STATUS_FINISHED = 2,
    TASK_STATUS_RUNNING = 3,
};

struct thread_task
{
    thread_task_f function;
    void *arg;

    thread_task *next;

    atomic_int status;
    void *result;

    pthread_mutex_t mutex;
    pthread_cond_t is_finished;

    atomic_bool is_detached;
};

struct thread_pool
{
    pthread_t *threads;

    int threads_count;
    int max_threads_count;

    queue *tasks;

    atomic_bool is_deleting;
    atomic_int running_tasks_count;
};

struct queue
{
    thread_task *head, *tail;
    atomic_int size;
    pthread_mutex_t mutex;
    pthread_cond_t empty_lock;
};

static queue
*queue_new(void)
{
    queue *q = calloc(1, sizeof(queue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->empty_lock, NULL);
    return q;
}

static void
queue_destroy(queue *q)
{
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->empty_lock);
    free(q);
}

static void
queue_push_back(queue *q, thread_task *task)
{
    pthread_mutex_lock(&q->mutex);
    pthread_cond_broadcast(&q->empty_lock);

    if (!atomic_load(&q->size))
    {
        q->head = task;
    } else
        q->tail->next = task;

    q->tail = task;
    atomic_fetch_add(&q->size, 1);

    pthread_mutex_unlock(&q->mutex);
}

static thread_task
*queue_pop_front(queue *q)
{
    pthread_mutex_lock(&q->mutex);
    if (!q->size)
    {
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    }

    thread_task *to_return = q->head;
    q->head = to_return->next;
    --q->size;

    pthread_mutex_unlock(&q->mutex);
    return to_return;
}

static void
*worker_function(void *context)
{
    thread_pool *pool = (thread_pool *) context;

    for (;;)
    {
        thread_task *task = queue_pop_front(pool->tasks);

        if (!task)
        {
            pthread_mutex_lock(&pool->tasks->mutex);
            if (atomic_load(&pool->is_deleting))
            {
                pthread_mutex_unlock(&pool->tasks->mutex);
                return NULL;
            }

            pthread_cond_wait(&pool->tasks->empty_lock, &pool->tasks->mutex);
            pthread_mutex_unlock(&pool->tasks->mutex);
            continue;
        }

        atomic_store(&task->status, TASK_STATUS_RUNNING);
        atomic_fetch_add(&pool->running_tasks_count, 1);

        task->result = task->function(task->arg);

        pthread_mutex_lock(&task->mutex);
        if (atomic_load(&task->is_detached))
        {
            atomic_store(&task->status, TASK_STATUS_FREE);
            pthread_mutex_unlock(&task->mutex);
            thread_task_delete(task);
        } else
        {
            atomic_store(&task->status, TASK_STATUS_FINISHED);
            pthread_cond_broadcast(&task->is_finished);
            pthread_mutex_unlock(&task->mutex);
        }

        atomic_fetch_sub(&pool->running_tasks_count, 1);
    }
}

int
thread_pool_new(int max_thread_count, thread_pool **pool)
{
    if (max_thread_count <= 0 || max_thread_count > TPOOL_MAX_THREADS)
        return TPOOL_ERR_INVALID_ARGUMENT;

    *pool = calloc(1, sizeof(thread_pool));

    (*pool)->max_threads_count = max_thread_count;
    (*pool)->tasks = queue_new();
    (*pool)->threads = calloc(max_thread_count, sizeof(pthread_t));

    atomic_init(&(*pool)->running_tasks_count, 0);
    return EXIT_SUCCESS;
}

int
thread_pool_thread_count(const thread_pool *pool)
{
    return pool->threads_count;
}

int
thread_pool_delete(thread_pool *pool)
{
    if (atomic_load(&pool->tasks->size) || atomic_load(&pool->running_tasks_count))
        return TPOOL_ERR_HAS_TASKS;

    atomic_store(&pool->is_deleting, true);
    pthread_cond_broadcast(&pool->tasks->empty_lock);

    for (int i = 0; i < pool->threads_count; ++i)
        pthread_join(pool->threads[i], NULL);

    free(pool->threads);
    queue_destroy(pool->tasks);

    free(pool);
    return EXIT_SUCCESS;
}

int
thread_pool_push_task(thread_pool *pool, thread_task *task)
{
    if (atomic_load(&pool->tasks->size) == TPOOL_MAX_TASKS)
        return TPOOL_ERR_TOO_MANY_TASKS;

    atomic_store(&task->status, TASK_STATUS_PUSHED);
    queue_push_back(pool->tasks, task);

    if (atomic_load(&pool->tasks->size) > pool->threads_count &&
        pool->threads_count < pool->max_threads_count)
        pthread_create(&pool->threads[pool->threads_count++], NULL, worker_function, pool);

    return EXIT_SUCCESS;
}

int
thread_task_new(thread_task **task, thread_task_f function, void *arg)
{
    (*task) = malloc(sizeof(struct thread_task));
    (*task)->function = function;
    (*task)->arg = arg;

    atomic_init(&(*task)->status, TASK_STATUS_FREE);
    atomic_init(&(*task)->is_detached, false);

    pthread_mutex_init(&(*task)->mutex, NULL);
    pthread_cond_init(&(*task)->is_finished, NULL);

    return EXIT_SUCCESS;
}

bool
thread_task_is_finished(const thread_task *task)
{
    return atomic_load(&task->status) == TASK_STATUS_FINISHED;
}

bool
thread_task_is_running(const thread_task *task)
{
    return atomic_load(&task->status) == TASK_STATUS_RUNNING;
}

int
thread_task_join(thread_task *task, void **result)
{
    if (atomic_load(&task->status) == TASK_STATUS_FREE)
        return TPOOL_ERR_TASK_NOT_PUSHED;

    pthread_mutex_lock(&task->mutex);

    if (atomic_load(&task->status) != TASK_STATUS_FINISHED)
        pthread_cond_wait(&task->is_finished, &task->mutex);

    pthread_mutex_unlock(&task->mutex);

    atomic_store(&task->status, TASK_STATUS_FINISHED);
    *result = task->result;
    return EXIT_SUCCESS;
}

int
thread_task_delete(thread_task *task)
{
    if (atomic_load(&task->status) != TASK_STATUS_FREE &&
        atomic_load(&task->status) != TASK_STATUS_FINISHED)
        return TPOOL_ERR_TASK_IN_POOL;

    pthread_cond_destroy(&task->is_finished);
    pthread_mutex_destroy(&task->mutex);
    free(task);

    return EXIT_SUCCESS;
}

#ifdef NEED_TIMED_JOIN

int
thread_task_timed_join(thread_task *task, double timeout, void **result)
{
    /* IMPLEMENT THIS FUNCTION */
    (void) task;
    (void) timeout;
    (void) result;
    return TPOOL_ERR_NOT_IMPLEMENTED;
}

#endif

#ifdef NEED_DETACH

int
thread_task_detach(thread_task *task)
{
    if (atomic_load(&task->status) == TASK_STATUS_FREE)
        return TPOOL_ERR_TASK_NOT_PUSHED;

    pthread_mutex_lock(&task->mutex);

    if (atomic_load(&task->status) == TASK_STATUS_FINISHED)
    {
        atomic_store(&task->status, TASK_STATUS_FREE);
        pthread_mutex_unlock(&task->mutex);
        thread_task_delete(task);
    } else
    {
        atomic_store(&task->is_detached, true);
        pthread_mutex_unlock(&task->mutex);
    }

    return EXIT_SUCCESS;
}

#endif