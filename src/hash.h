/**
 * @file hash.h
 * @brief Интерфейс хеш-таблицы для хранения изменений (инкрементов).
 * 
 * Обеспечивает:
 * - создание/уничтожение хеш-таблицы;
 * - вставку/обновление изменения по номеру;
 * - поиск изменения по номеру.
 */
#ifndef HASH_H
#define HASH_H

#include "types.h"

ChangeHash* changes_create(size_t size);
void changes_put(ChangeHash *h, phone_t phone, uint8_t type,
                 const char *comment, size_t comment_len,
                 const char *date, size_t date_len,
                 char *pool, size_t *pool_pos);
Change* changes_get(ChangeHash *h, phone_t phone);
void changes_free(ChangeHash *h);

#endif

