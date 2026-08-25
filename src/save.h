/**
 * @file save.h
 * @brief Сохранение объединённых данных (основной массив + изменения) в CSV-файл.
 * 
 * Выполняет слияние записей из основного массива и изменений, применяет
 * удаления, обновления и добавления, сортирует по номеру и записывает результат.
 */
#ifndef SAVE_H
#define SAVE_H

#include "types.h"
#include <stddef.h>

int save_merged(const char *filename, Record *records, size_t count,
                ChangeHash *changes, char *pool, char *change_pool);

#endif

