/**
 * @file utils.h
 * @brief Утилитные функции для парсинга и форматирования телефонных номеров.
 * 
 * Экспортируемые функции:
 * - parse_phone – преобразует hex-строку в 128-битное число.
 * - phone_to_hex – преобразует 128-битное число в hex-строку.
 * - hash_phone – вычисляет хеш для 128-битного номера (используется в хеш-таблице).
 */
#ifndef UTILS_H
#define UTILS_H

#include "types.h"

phone_t parse_phone(const char *str, size_t len);
void phone_to_hex(phone_t p, char *buf, size_t bufsize);
uint64_t hash_phone(phone_t p, size_t size);

#endif

