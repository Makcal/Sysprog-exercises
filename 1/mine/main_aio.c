/*
 * Actually, this try in asynchronous IO made perfomance worse :(
 */

#include <aio.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "libcoro.h"

typedef int (*int_arr_p)[];

struct coro_result {
    size_t count;
    int_arr_p array;
    double running_time;
    size_t switches;
};

struct coro_data {
    const char *filename;
    struct coro_result *result_p;
};

static double get_time_ms(void) {
    return (double)clock() / CLOCKS_PER_SEC * 1000;
}

static inline void coro_yield_with_metrics(double *const start_time_p, struct coro_result *const result) {
    double end_time = get_time_ms();
    result->running_time += end_time - *start_time_p;
    result->switches++;
    coro_yield();
    *start_time_p = get_time_ms();
}

static void input_file(const char *const filename, double *start_time_p, struct coro_result *const result_p) {
    /*FILE *file = fopen(filename, "r");*/
    struct aiocb aiocb;
    memset(&aiocb, 0, sizeof(aiocb));

    const int fd = open(filename, O_RDONLY);
    if (fd == -1)
        err(EXIT_FAILURE, "open");

    aiocb.aio_fildes = fd;

    static const size_t BUF_SIZE = 10 * 1024; // 100 KB
    char io_buf[BUF_SIZE + 1];
    io_buf[BUF_SIZE] = '\0';

    size_t buf_size = 16, i = 0, last_portion_size = 0;
    int_arr_p array = malloc(buf_size * sizeof(int));
    while (true) {
        // Asynchronously read next portion.
        aiocb.aio_buf = io_buf + last_portion_size;
        aiocb.aio_nbytes = BUF_SIZE - last_portion_size;
        aio_read(&aiocb);
        *start_time_p = *start_time_p;
        *result_p = *result_p;
        while (aio_error(&aiocb) == EINPROGRESS)
            coro_yield_with_metrics(start_time_p, result_p);
        ssize_t read_count = aio_return(&aiocb);
        if (read_count == 0) {
            close(fd);
            break;
        }
        aiocb.aio_offset += read_count;

        // Parse numbers from the buffer.
        size_t total_shift = 0;
        for (int n; true; i++) {
            size_t shift;
            int scan_result = sscanf(io_buf + total_shift, "%d %zn", &n, &shift);
            if (scan_result == 0)
                err(EXIT_FAILURE, "scanf (not a number)");
            else if (scan_result == EOF) {
                // We are at the end of the buffer.
                // Move the last (possibly partially read) to the buffer's start
                // and read next portion.
                const size_t prev_start = total_shift - shift;
                last_portion_size = BUF_SIZE - prev_start;
                memmove(io_buf, io_buf + prev_start, last_portion_size);
                // The last number should be re-parsed (to count case it had been read partially).
                i--;
                break;
            }
            total_shift += shift;

            if (i == buf_size) {
                buf_size *= 2;
                array = realloc(array, buf_size * sizeof(int));
            }
            (*array)[i] = n;
        }
    }
    result_p->count = i;
    result_p->array = array;
    /*fclose(file);*/
}

static void merge_sort(int *l, int *r, double *const start_time_p, struct coro_result *const result_p) {
    size_t len = (size_t)(r - l);
    if (len <= 1) {
        return;
    } else if (len == 2) {
        r = r - 1;
        if (*l > *r) {
            int t = *l;
            *l = *r;
            *r = t;
        }
        return;
    }

    int *m = l + len / 2;
    merge_sort(l, m, start_time_p, result_p);
    merge_sort(m, r, start_time_p, result_p);

    int buf[len];
    int *cur1 = l, *cur2 = m;
    for (size_t i = 0; i < len; i++) {
        if (cur1 == m) {
            while (cur2 != r)
                buf[i++] = *cur2++;
            break;
        } else if (cur2 == r) {
            while (cur1 != m)
                buf[i++] = *cur1++;
            break;
        }
        if (*cur1 < *cur2)
            buf[i] = *cur1++;
        else
            buf[i] = *cur2++;
    }
    memcpy(l, buf, len * sizeof(int));
    coro_yield_with_metrics(start_time_p, result_p);
}

static void coroutine(void *const raw_data) {
    double time = get_time_ms();

    // input
    struct coro_data *const data = raw_data;
    input_file(data->filename, &time, data->result_p);
    coro_yield_with_metrics(&time, data->result_p);

    // sorting
    const int_arr_p buf = data->result_p->array;
    merge_sort(*buf, (*buf) + data->result_p->count, &time, data->result_p);
}

int main(int argc, char *argv[]) {
    const int nfiles = argc - 1;
    if (nfiles <= 0) {
        fprintf(stderr, "Not enough arguments!\n");
        return 1;
    }

    loop_t loop;
    loop_init(&loop);
    use_loop(&loop);

    struct coro_data data[nfiles];
    struct coro_result results[nfiles];
    for (int i = 0; i < nfiles; i++) {
        data[i].filename = argv[1 + i];
        results[i].running_time = 0;
        results[i].switches = 0;
        data[i].result_p = &results[i];
        coro_create(&loop, coroutine, &data[i]);
    }

    const double time1 = get_time_ms();
    loop_start();

    size_t total_count = 0;
    for (int i = 0; i < nfiles; i++) {
        total_count += results[i].count;
    }

    int merged[total_count];
    size_t cursors[nfiles] = {};
    for (size_t t = 0; t < total_count; t++) {
        int min = INT_MAX;
        size_t min_j = -1;
        for (int j = 0; j < nfiles; j++) {
            if (cursors[j] == results[j].count)
                continue;
            int value = (*results[j].array)[cursors[j]];
            if (value < min) {
                min = value;
                min_j = j;
            }
        }

        cursors[min_j]++;
        merged[t] = min;
    }
    const double time2 = get_time_ms();

    // Output
    for (size_t i = 0; i < total_count; i++) {
        printf(i == 0 ? "%d" : " %d", merged[i]);
    }
    printf("\n");

    // Perfomance
    fprintf(stderr, "Total work time: %.3lf ms\n", time2 - time1);
    for (int i = 0; i < nfiles; i++) {
        fprintf(stderr,
                "Coro #%d. Work time: %.3lf ms. %lu switches.\n",
                i + 1,
                results[i].running_time,
                (unsigned long)results[i].switches);
    }

    for (int i = 0; i < nfiles; i++) {
        free(results[i].array);
    }
    fclose(stdout);
}

