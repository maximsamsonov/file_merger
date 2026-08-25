/**
 * @file search.h
 * @brief Бинарный поиск записи по номеру телефона в отсортированном массиве Record.
 * 
 * Возвращает индекс найденной записи или -1, если номер не найден.
 */
#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include <stddef.h>

int binary_search(Record *arr, size_t n, phone_t phone);

#endif

