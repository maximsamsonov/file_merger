/**
 * @file load.h
 * @brief Загрузка основного набора данных из нескольких CSV-файлов.
 * 
 * Функция load_from_files принимает список имён файлов, параллельно парсит их
 * с использованием mmap и пула потоков, заполняет массив Record и пул строк.
 */
#ifndef LOAD_H
#define LOAD_H

#include "types.h"
#include <stddef.h>

int load_from_files(const char **filenames, size_t num_files,
                    Record **out_records, size_t *out_count,
                    char **out_pool, size_t *out_pool_size);

#endif

