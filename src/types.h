/**
 * @file types.h
 * @brief Определение основных структур данных, используемых во всей программе.
 * 
 * Содержит:
 * - phone_t – 128-битный тип для хранения телефонного номера (до 19 hex-цифр).
 * - Record – структура записи из основного набора данных.
 * - Change – структура для хранения изменения (добавление/удаление/обновление).
 * - ChangeHash – хеш-таблица для хранения изменений.
 */
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef unsigned __int128 phone_t;

typedef struct {
    phone_t phone;
    uint32_t comment_offset;
    uint32_t comment_len;
    uint32_t date_offset;
    uint32_t date_len;
} Record;

typedef struct Change {
    phone_t phone;
    uint8_t type;              // 0 – delete, 1 – add, 2 – update
    uint32_t comment_offset;
    uint32_t comment_len;
    uint32_t date_offset;
    uint32_t date_len;
    struct Change *next;
} Change;

typedef struct {
    size_t size;
    Change **buckets;
} ChangeHash;

#endif

