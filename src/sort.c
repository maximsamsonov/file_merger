/**
 * @file sort.c
 * @brief Параллельная сортировка слиянием с использованием потоков.
 * 
 * Для малых фрагментов используется qsort, для больших – рекурсивное
 * разделение и слияние в отдельных потоках.
 */
#include "sort.h"
#include <stdlib.h>
#include <pthread.h>

static int record_cmp(const void *a, const void *b) {
    const Record *ra = (const Record*)a;
    const Record *rb = (const Record*)b;
    if (ra->phone < rb->phone) return -1;
    if (ra->phone > rb->phone) return 1;
    return 0;
}

static void sequential_sort(Record *arr, size_t n) {
    qsort(arr, n, sizeof(Record), record_cmp);
}

static void merge(Record *arr, size_t left, size_t mid, size_t right, Record *tmp) {
    size_t i = left, j = mid, k = left;
    while (i < mid && j < right) {
        if (record_cmp(&arr[i], &arr[j]) <= 0)
            tmp[k++] = arr[i++];
        else
            tmp[k++] = arr[j++];
    }
    while (i < mid) tmp[k++] = arr[i++];
    while (j < right) tmp[k++] = arr[j++];
    for (i = left; i < right; ++i) arr[i] = tmp[i];
}

typedef struct {
    Record *arr;
    Record *tmp;
    size_t left;
    size_t right;
    int depth;
} SortTask;

static void* sort_thread(void *arg) {
    SortTask *task = (SortTask*)arg;
    size_t n = task->right - task->left;
    if (n <= 1024) {
        sequential_sort(task->arr + task->left, n);
        return NULL;
    }
    size_t mid = task->left + n/2;
    SortTask left_task = {task->arr, task->tmp, task->left, mid, task->depth + 1};
    SortTask right_task = {task->arr, task->tmp, mid, task->right, task->depth + 1};
    pthread_t left_thread, right_thread;
    pthread_create(&left_thread, NULL, sort_thread, &left_task);
    pthread_create(&right_thread, NULL, sort_thread, &right_task);
    pthread_join(left_thread, NULL);
    pthread_join(right_thread, NULL);
    merge(task->arr, task->left, mid, task->right, task->tmp);
    return NULL;
}

void parallel_sort(Record *arr, size_t n) {
    if (n <= 1) return;
    Record *tmp = malloc(n * sizeof(Record));
    if (!tmp) { qsort(arr, n, sizeof(Record), record_cmp); return; }
    SortTask main_task = {arr, tmp, 0, n, 0};
    pthread_t main_thread;
    pthread_create(&main_thread, NULL, sort_thread, &main_task);
    pthread_join(main_thread, NULL);
    free(tmp);
}

