/**
 * @file sort.h
 * @brief Параллельная сортировка массива Record по полю phone.
 * 
 * Реализует многопоточное слияние (merge sort), используя до 24 потоков.
 */
#ifndef SORT_H
#define SORT_H

#include "types.h"
#include <stddef.h>

void parallel_sort(Record *arr, size_t n);

#endif

