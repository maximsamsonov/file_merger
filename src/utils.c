/**
 * @file utils.c
 * @brief Реализация функций парсинга и форматирования номеров.
 */
#include "utils.h"
#include <string.h>
#include <stdio.h>

phone_t parse_phone(const char *str, size_t len) {
    phone_t val = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = str[i];
        int digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else continue;
        val = (val << 4) | digit;
    }
    return val;
}

void phone_to_hex(phone_t p, char *buf, size_t bufsize) {
    if (bufsize == 0) return;
    if (p == 0) {
        snprintf(buf, bufsize, "0");
        return;
    }
    char tmp[64];
    int idx = 0;
    while (p > 0 && idx < (int)(sizeof(tmp) - 1)) {
        int digit = p & 0xF;
        tmp[idx++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        p >>= 4;
    }
    if (idx >= (int)bufsize) idx = (int)bufsize - 1;
    for (int i = 0; i < idx; ++i)
        buf[i] = tmp[idx - 1 - i];
    buf[idx] = '\0';
}

uint64_t hash_phone(phone_t p, size_t size) {
    uint64_t lo = (uint64_t)p;
    uint64_t hi = (uint64_t)(p >> 64);
    return (lo ^ hi) % size;
}

