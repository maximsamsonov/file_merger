/**
 * @file inc.h
 * @brief Загрузка файлов инкрементов (изменений) из CSV.
 * 
 * Читает файлы с операциями A (add), D (delete), U (update) и заполняет
 * хеш-таблицу изменений, используя отдельный пул для строк.
 */
#ifndef INC_H
#define INC_H

#include "types.h"
#include <stddef.h>

int load_increments_from_files(const char **filenames, size_t num_files,
                               ChangeHash *changes, char *change_pool, size_t *change_pool_pos);

#endif

