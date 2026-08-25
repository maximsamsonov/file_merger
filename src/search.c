/**
 * @file search.c
 * @brief Реализация бинарного поиска по массиву Record.
 */
#include "search.h"

int binary_search(Record *arr, size_t n, phone_t phone) {
    size_t lo = 0, hi = n;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (arr[mid].phone < phone) lo = mid + 1;
        else if (arr[mid].phone > phone) hi = mid;
        else return (int)mid;
    }
    return -1;
}

