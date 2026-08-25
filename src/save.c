/**
 * @file save.c
 * @brief Реализация слияния основного набора и изменений с записью в CSV.
 * 
 * Собирает ключи изменений, сортирует их, затем выполняет объединённый проход
 * по массиву и ключам, применяя операции и записывая результат.
 */
#include "save.h"
#include "hash.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compare_phone(const void *a, const void *b) {
    phone_t pa = *(const phone_t*)a;
    phone_t pb = *(const phone_t*)b;
    if (pa < pb) return -1;
    if (pa > pb) return 1;
    return 0;
}

int save_merged(const char *filename, Record *records, size_t count,
                ChangeHash *changes, char *pool, char *change_pool) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;

    size_t change_keys_count = 0;
    phone_t *change_keys = NULL;
    for (size_t i = 0; i < changes->size; ++i) {
        Change *ch = changes->buckets[i];
        while (ch) {
            if (ch->type != 0) {
                change_keys = realloc(change_keys, (change_keys_count + 1) * sizeof(phone_t));
                change_keys[change_keys_count++] = ch->phone;
            }
            ch = ch->next;
        }
    }
    qsort(change_keys, change_keys_count, sizeof(phone_t), compare_phone);

    size_t i = 0, j = 0;
    char phone_buf[64];
    while (i < count || j < change_keys_count) {
        Change *ch = NULL;
        Record *rec = NULL;
        int use_change = 0;

        if (j >= change_keys_count || (i < count && records[i].phone < change_keys[j])) {
            ch = changes_get(changes, records[i].phone);
            if (ch) {
                if (ch->type == 0) {
                    i++;
                    continue;
                } else if (ch->type == 2) {
                    use_change = 1;
                    if (j < change_keys_count && change_keys[j] == records[i].phone) {
                        j++;
                    }
                    i++;
                } else {
                    rec = &records[i];
                    i++;
                }
            } else {
                rec = &records[i];
                i++;
            }
        } else if (i >= count || records[i].phone > change_keys[j]) {
            ch = changes_get(changes, change_keys[j]);
            if (ch && ch->type != 0) {
                use_change = 1;
                j++;
            } else {
                j++;
                continue;
            }
        } else {
            ch = changes_get(changes, records[i].phone);
            if (ch) {
                if (ch->type == 0) {
                    i++;
                    j++;
                    continue;
                } else if (ch->type == 2) {
                    use_change = 1;
                    i++;
                    j++;
                } else {
                    rec = &records[i];
                    i++;
                    j++;
                }
            } else {
                rec = &records[i];
                i++;
                j++;
            }
        }

        if (use_change) {
            phone_to_hex(ch->phone, phone_buf, sizeof(phone_buf));
            fprintf(f, "%s;", phone_buf);
            if (ch->comment_len > 0)
                fprintf(f, "%.*s", (int)ch->comment_len, change_pool + ch->comment_offset);
            fprintf(f, ";");
            if (ch->date_len > 0)
                fprintf(f, "%.*s", (int)ch->date_len, change_pool + ch->date_offset);
            fprintf(f, "\n");
        } else if (rec) {
            phone_to_hex(rec->phone, phone_buf, sizeof(phone_buf));
            fprintf(f, "%s;", phone_buf);
            if (rec->comment_len > 0)
                fprintf(f, "%.*s", (int)rec->comment_len, pool + rec->comment_offset);
            fprintf(f, ";");
            if (rec->date_len > 0)
                fprintf(f, "%.*s", (int)rec->date_len, pool + rec->date_offset);
            fprintf(f, "\n");
        }
    }

    free(change_keys);
    fclose(f);
    return 0;
}

