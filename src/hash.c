/**
 * @file hash.c
 * @brief Реализация хеш-таблицы изменений с цепочками для разрешения коллизий.
 * 
 * Строки комментариев и дат хранятся в отдельном пуле, чтобы минимизировать
 * накладные расходы на выделение памяти.
 */
#include "hash.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

ChangeHash* changes_create(size_t size) {
    ChangeHash *h = calloc(1, sizeof(ChangeHash));
    h->size = size;
    h->buckets = calloc(size, sizeof(Change*));
    return h;
}

void changes_put(ChangeHash *h, phone_t phone, uint8_t type,
                 const char *comment, size_t comment_len,
                 const char *date, size_t date_len,
                 char *pool, size_t *pool_pos) {
    uint64_t idx = hash_phone(phone, h->size);
    Change *ch = h->buckets[idx];
    while (ch) {
        if (ch->phone == phone) {
            ch->type = type;
            if (comment_len > 0) {
                ch->comment_offset = *pool_pos;
                memcpy(pool + *pool_pos, comment, comment_len);
                pool[*pool_pos + comment_len] = '\0';
                ch->comment_len = comment_len;
                *pool_pos += comment_len + 1;
            } else {
                ch->comment_offset = 0;
                ch->comment_len = 0;
            }
            if (date_len > 0) {
                ch->date_offset = *pool_pos;
                memcpy(pool + *pool_pos, date, date_len);
                pool[*pool_pos + date_len] = '\0';
                ch->date_len = date_len;
                *pool_pos += date_len + 1;
            } else {
                ch->date_offset = 0;
                ch->date_len = 0;
            }
            return;
        }
        ch = ch->next;
    }
    Change *new_ch = malloc(sizeof(Change));
    new_ch->phone = phone;
    new_ch->type = type;
    if (comment_len > 0) {
        new_ch->comment_offset = *pool_pos;
        memcpy(pool + *pool_pos, comment, comment_len);
        pool[*pool_pos + comment_len] = '\0';
        new_ch->comment_len = comment_len;
        *pool_pos += comment_len + 1;
    } else {
        new_ch->comment_offset = 0;
        new_ch->comment_len = 0;
    }
    if (date_len > 0) {
        new_ch->date_offset = *pool_pos;
        memcpy(pool + *pool_pos, date, date_len);
        pool[*pool_pos + date_len] = '\0';
        new_ch->date_len = date_len;
        *pool_pos += date_len + 1;
    } else {
        new_ch->date_offset = 0;
        new_ch->date_len = 0;
    }
    new_ch->next = h->buckets[idx];
    h->buckets[idx] = new_ch;
}

Change* changes_get(ChangeHash *h, phone_t phone) {
    uint64_t idx = hash_phone(phone, h->size);
    Change *ch = h->buckets[idx];
    while (ch) {
        if (ch->phone == phone) return ch;
        ch = ch->next;
    }
    return NULL;
}

void changes_free(ChangeHash *h) {
    if (!h) return;
    for (size_t i = 0; i < h->size; ++i) {
        Change *ch = h->buckets[i];
        while (ch) {
            Change *next = ch->next;
            free(ch);
            ch = next;
        }
    }
    free(h->buckets);
    free(h);
}

