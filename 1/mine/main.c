#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libcoro.h"

typedef int (*int_arr_p)[];

struct file_result {
    size_t count;
    int_arr_p array_p;
};

struct file_data {
    char status;
    const char *filename;
    struct file_result result;
};

struct common_data {
    size_t n_files;
    double quantum;
    struct file_data (*file_data_p)[];
};

struct coro_data {
    double running_time;
    size_t switches;
    const struct common_data *common_data_p;
};

static double get_time_ms(void) {
    return (double)clock() / CLOCKS_PER_SEC * 1000;
}

static inline void coro_yield_with_metrics(double *const start_time_p, struct coro_data *const coro_data_p) {
    double end_time = get_time_ms();
    double passed_time = end_time - *start_time_p;
    if (passed_time < coro_data_p->common_data_p->quantum)
        return;
    coro_data_p->running_time += passed_time;
    coro_data_p->switches++;
    coro_yield();
    *start_time_p = get_time_ms();
}

static void input_file(const char *const filename, struct file_result *const out) {
    FILE *file = fopen(filename, "r");
    if (!file)
        err(EXIT_FAILURE, "fopen");
    int buf_size = 16, i = 0;
    int_arr_p buf_p = malloc(buf_size * sizeof(int));
    for (int n; fscanf(file, "%d", &n) == 1; i++) {
        if (i == buf_size) {

            buf_size *= 2;
            buf_p = realloc(buf_p, buf_size * sizeof(int));
        }
        (*buf_p)[i] = n;
    }
    out->count = i;
    out->array_p = buf_p;
    fclose(file);
}

static void merge_sort(int *const l, int *const r, double *const start_time_p, struct coro_data *const coro_data_p) {
    size_t len = (size_t)(r - l);
    if (len <= 1) {
        return;
    } else if (len == 2) {
        if (*l > *(l + 1)) {
            int t = *l;
            *l = *(l + 1);
            *(l + 1) = t;
        }
        return;
    }

    int *m = l + len / 2;
    merge_sort(l, m, start_time_p, coro_data_p);
    merge_sort(m, r, start_time_p, coro_data_p);

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
    coro_yield_with_metrics(start_time_p, coro_data_p);
}

static void coroutine(void *const raw_data) {
    double time = get_time_ms();

    struct coro_data *const coro_data_p = raw_data;
    const struct common_data *const common_data_p = coro_data_p->common_data_p;

    size_t file_i = 0;
    while (true) {
        // find a free file
        struct file_data *file_data_p = NULL;
        for (; file_i < common_data_p->n_files; file_i++) {
            file_data_p = &(*common_data_p->file_data_p)[file_i];
            if (file_data_p->status == 0) {
                file_data_p->status = 1;
                break;
            }
        }
        if (file_i == common_data_p->n_files || !file_data_p)
            break;
        coro_yield_with_metrics(&time, coro_data_p);

        // input
        input_file(file_data_p->filename, &file_data_p->result);
        coro_yield_with_metrics(&time, coro_data_p);

        // sorting
        const int_arr_p array_p = file_data_p->result.array_p;
        merge_sort(*array_p, (*array_p) + file_data_p->result.count, &time, coro_data_p);
    }

    double end_time = get_time_ms();
    coro_data_p->running_time += end_time - time;
}

int main(int argc, char *argv[]) {
    if (argc <= 3)
        errx(EXIT_FAILURE, "Not enough arguments!");
    const size_t n_files = (size_t)argc - 3;
    const size_t n_coro = (size_t)atoi(argv[1]);
    if (!n_coro)
        errx(EXIT_FAILURE, "Zero coroutines");
    const double quantum = atof(argv[2]) / n_coro;
    if (!quantum)
        errx(EXIT_FAILURE, "Quantum is zero");

    // Init variables
    loop_t loop;
    loop_init(&loop);
    use_loop(&loop);

    struct file_data file_data[n_files];
    for (size_t i = 0; i < n_files; i++) {
        file_data[i].status = 0;
        file_data[i].filename = argv[3 + i];
    }

    const struct common_data common_data = {.quantum = quantum, .n_files = n_files, .file_data_p = &file_data};
    struct coro_data coro_data[n_coro];
    for (size_t i = 0; i < n_coro; i++) {
        coro_data[i].common_data_p = &common_data;
        coro_data[i].switches = 0;
        coro_data[i].running_time = 0;
        coro_create(&loop, coroutine, &coro_data[i]);
    }

    // Run
    const double time1 = get_time_ms();
    loop_start();

    size_t total_count = 0;
    for (size_t i = 0; i < n_files; i++) {
        total_count += file_data[i].result.count;
    }

    // Merge results
    int merged[total_count];
    size_t cursors[n_files] = {};
    for (size_t t = 0; t < total_count; t++) {
        int min = INT_MAX;
        size_t min_j = -1;
        for (size_t j = 0; j < n_files; j++) {
            if (cursors[j] == file_data[j].result.count)
                continue;
            int value = (*file_data[j].result.array_p)[cursors[j]];
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
    for (size_t i = 0; i < n_coro; i++) {
        fprintf(stderr,
                "Coro #%zu. Work time: %.3lf ms. %zu switches.\n",
                i + 1,
                coro_data[i].running_time,
                coro_data[i].switches);
    }

    // Free
    for (size_t i = 0; i < n_files; i++) {
        free(*file_data[i].result.array_p);
    }
    fclose(stdout);
}

