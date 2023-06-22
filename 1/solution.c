#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <limits.h>
#include "libcoro.h"

#define min(a, b) (a) < (b) ? (a) : (b)

typedef struct files_pool files_pool;
typedef struct files_list files_list;
typedef struct file_content file_content;
typedef struct coro_context coro_context;

struct file_content
{
    int *array;
    size_t size;
};

struct files_list
{
    char *file_name;
    files_list *prev;
};

/**
 * Queue of files to be processed. Bonus task #2
 */
struct files_pool
{
    files_list *head;
    size_t size;
};

/**
 * Essential data needed by coroutine
 */
struct coro_context
{
    char *name;

    /** Start_time is initialized where a coroutine wakes up, end_time - before each yield */
    struct timespec start_time, end_time;

    /** execution_time += end_time - start_time */
    size_t execution_time;

    /** Quantum T / N for bonus task */
    time_t quantum;

    files_pool *pool;
};

static coro_context
*new_coro_context(char *name, time_t quantum, files_pool *pool)
{
    coro_context *context = calloc(1, sizeof(coro_context));

    context->name = name;
    context->quantum = quantum;
    context->pool = pool;

    return context;
}

static void
coro_start_timer(coro_context *context)
{
    clock_gettime(CLOCK_MONOTONIC, &context->start_time);
}

static void
coro_end_timer(coro_context *context)
{
    clock_gettime(CLOCK_MONOTONIC, &context->end_time);
}

/**
 * Calculates difference between end_time and start_time
 *
 * @param context coro_context
 */
static void
coro_append_execution_time(coro_context *context)
{
    context->execution_time += context->end_time.tv_sec * 10e9 - context->start_time.tv_sec * 10e9;
    context->execution_time += context->end_time.tv_nsec - context->start_time.tv_nsec;

    memset(&context->start_time, 0, sizeof(context->start_time));
    memset(&context->end_time, 0, sizeof(context->end_time));
}

/**
 * Take the file and yield (let other coroutines also take the files)
 *
 * @param casted_context current coroutine's context
 * @param this current coroutine
 * @param file_name file to open & sort
 */
static void
coro_handle_file_selection(coro_context *casted_context, char *file_name)
{
    printf("%s: selected file %s\n", casted_context->name, file_name);

    {
        coro_end_timer(casted_context);
        coro_append_execution_time(casted_context);

        // Give an opportunity to other coroutines to take some file
        coro_yield();

        coro_start_timer(casted_context);
    }
}

/**
 * Just for code readability - iterator
 *
 * @param pool files_pool
 * @return head of the pool as an iterator pointer
 */
static files_list
*new_iterator(files_pool *pool)
{
    return pool->head;
}

static files_list
*peek_next_file(files_list *iterator)
{
    if (iterator == NULL) return NULL;
    return iterator->prev;
}

static files_pool
*new_files_pool()
{
    return calloc(1, sizeof(files_pool));
}

static void
del_files_pool(files_pool *pool)
{
    files_list *iterator = new_iterator(pool);

    while (iterator != NULL)
    {
        files_list *to_remove = iterator;
        iterator = peek_next_file(iterator);
        free(to_remove);
    }
}

static void
pool_append_file(char *name, files_pool *pool)
{
    files_list *old_head = pool->head;

    files_list *new_head = calloc(1, sizeof(files_list));
    new_head->file_name = name;
    new_head->prev = old_head;

    pool->head = new_head;
    ++pool->size;
}

static char
*poll_next_file(files_pool *pool)
{
    if (pool->size == 0)
        return NULL;

    files_list *current_head = pool->head;
    files_list *new_head = current_head->prev;

    char *file_name = current_head->file_name;

    free(current_head);
    pool->head = new_head;

    --pool->size;
    return file_name;
}

static void
*realloc_array(void *array, size_t new_capacity)
{
    void *updated_array = realloc(array, new_capacity * sizeof(void *));
    assert(updated_array != NULL);
    return updated_array;
}

static file_content
*read_file(char *name)
{
    FILE *file = fopen(name, "r");

    assert(file != NULL);

    size_t array_capacity = 128, size = 0;
    int *array = malloc(array_capacity * sizeof(int));

    int num;
    while (fscanf(file, "%d", &num) != EOF)
    {

        if (size == array_capacity)
        {
            array_capacity += 10;
            array = realloc_array(array, array_capacity);
        }

        array[size++] = num;
    }

    assert(fclose(file) == 0);

    file_content *content = malloc(sizeof(file_content));
    content->array = size == 0 ? NULL : realloc_array(array, size);
    content->size = size;

    return content;
}

static void
write_file(file_content *content, char *file_name)
{
    FILE *file = fopen(file_name, "w");
    assert(file != NULL);

    for (size_t i = 0; i < content->size; ++i)
        fprintf(file, "%d ", content->array[i]);

    assert(fclose(file) == 0);
}

static void
merge(int *array, int *tmp, size_t size, size_t from, size_t mid, size_t to)
{
    size_t k = from, i = from, j = mid + 1;

    while (i <= mid && j <= to)
        if (array[i] < array[j])
            tmp[k++] = array[i++];
        else
            tmp[k++] = array[j++];

    while (i < size && i <= mid)
        tmp[k++] = array[i++];

    for (i = from; i <= to; i++)
        array[i] = tmp[i];
}

static void
merge_sort(file_content *content, coro_context *casted_context)
{
    int *array = content->array;
    size_t size = content->size;

    int *tmp = malloc(size * sizeof(int));
    memcpy(tmp, array, sizeof(int) * size);

    for (size_t m = 1; m <= size - 1; m = 2 * m)
    {
        for (size_t i = 0; i < size - 1; i += 2 * m)
        {
            merge(array, tmp, size, i, i + m - 1, min(i + 2 * m - 1, size - 1));

            coro_end_timer(casted_context);
            time_t current_execution_time
                    = casted_context->end_time.tv_nsec - casted_context->start_time.tv_nsec;

            if (current_execution_time > casted_context->quantum)
            {
                // Force update timer in order to account if-condition and two prints to the console
                coro_end_timer(casted_context);
                coro_append_execution_time(casted_context);

                coro_yield();
                coro_start_timer(casted_context);
            }
        }
    }
}

static void
merge_all_files(files_pool *pool, char *merged_file_name)
{
    files_list *iterator = new_iterator(pool);

    size_t i, j;

    size_t contents_capacity = pool->size;
    file_content **contents = malloc(contents_capacity * sizeof(file_content *));

    for (i = 0; iterator != NULL; ++i) {
        contents[i] = read_file(iterator->file_name);
        iterator = peek_next_file(iterator);
    }

    size_t total_size = 0;
    for (i = 0; i < contents_capacity; ++i)
        total_size += contents[i]->size;

    int *merged = malloc(total_size * sizeof(int));
    size_t *indices = calloc(contents_capacity, sizeof(size_t));

    for (i = 0; i < total_size; ++i)
    {
        size_t min_index = -1;
        int min_value = INT_MAX;

        for (j = 0; j < contents_capacity; ++j)
        {
            if (indices[j] < contents[j]->size && contents[j]->array[indices[j]] < min_value)
            {

                min_index = j;
                min_value = contents[j]->array[indices[j]];
            }
        }

        merged[i] = min_value;
        ++indices[min_index];
    }

    free(indices);

    for (i = 0; i < contents_capacity; ++i) {
        free(contents[i]->array);
        free(contents[i]);
    }

    free(contents);

    file_content merged_content = {
            .array = merged,
            .size = total_size
    };

    write_file(&merged_content, merged_file_name);
}

/**
 * Coroutine body. This code is executed by all the coroutines. Here you
 * implement your solution, sort each individual file.
 */
static int
coroutine_func_f(void *context)
{
    coro_context *casted_context = (coro_context *) context;

    coro_start_timer(casted_context);

    printf("Started coroutine %s\n", casted_context->name);

    char *file_name;
    while ((file_name = poll_next_file(casted_context->pool)) != NULL)
    {

        coro_handle_file_selection(casted_context, file_name);
        file_content *content = read_file(file_name);
        merge_sort(content, casted_context);

        write_file(content, file_name);

        free(content->array);
        free(content);
    }

    coro_end_timer(casted_context);
    coro_append_execution_time(casted_context);

    printf(
            "Stopped coroutine %s, exec time: %ld nanoseconds\n",
            casted_context->name,
            casted_context->execution_time
    );

    return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{
    files_pool *pool = new_files_pool();
    files_pool *copy_pool = new_files_pool();

    for (int i = 3; i < argc - 1; ++i)
    {
        pool_append_file(argv[i], pool);
        pool_append_file(argv[i], copy_pool);
    }

    coro_sched_init();

    // Bonus task #2
    size_t coro_count = strtoull(argv[1], NULL, 10);

    // Bonus task #1
    time_t quantum = (time_t) ((double) strtoull(argv[2], NULL, 10) / (double) coro_count * 10e3);

    for (size_t i = 0; i < coro_count; ++i)
    {
        char name[16];
        sprintf(name, "coro_%ld", i);

        coro_context *context = new_coro_context(strdup(name), quantum, pool);
        coro_new(coroutine_func_f, context);
    }

    /* Wait for all the coroutines to end. */
    struct coro *c;
    while ((c = coro_sched_wait()) != NULL)
    {
        /*
         * Each 'wait' returns a finished coroutine with which you can
         * do anything you want. Like check its exit status, for
         * example. Don't forget to free the coroutine afterward.
         */
        printf("Finished %d\n", coro_status(c));
        coro_delete(c);
    }

    merge_all_files(copy_pool, "merged.txt");

    del_files_pool(pool);
    del_files_pool(copy_pool);

    return EXIT_SUCCESS;
}
