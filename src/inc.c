/**
 * @file inc.c
 * @brief Реализация загрузки инкрементов из CSV-файлов.
 * 
 * Последовательно обрабатывает каждый файл, парсит строки и добавляет
 * изменения в хеш-таблицу.
 */
#include "inc.h"
#include "hash.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int load_increments_from_files(const char **filenames, size_t num_files,
                               ChangeHash *changes, char *change_pool, size_t *change_pool_pos) {
    for (size_t f = 0; f < num_files; ++f) {
        FILE *file = fopen(filenames[f], "r");
        if (!file) {
            fprintf(stderr, "Не удалось открыть файл инкрементов: %s\n", filenames[f]);
            continue;
        }
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        while ((read = getline(&line, &len, file)) != -1) {
            if (read > 0 && line[read-1] == '\n') line[--read] = '\0';
            if (read > 0 && line[read-1] == '\r') line[--read] = '\0';
            if (read == 0) continue;
            char *p1 = strchr(line, ';');
            if (!p1) continue;
            uint8_t type;
            switch (line[0]) {
                case 'A': type = 1; break;
                case 'D': type = 0; break;
                case 'U': type = 2; break;
                default: continue;
            }
            char *phone_start = p1 + 1;
            char *p2 = strchr(phone_start, ';');
            if (!p2) continue;
            size_t phone_len = p2 - phone_start;
            phone_t phone = parse_phone(phone_start, phone_len);
            char *comment_start = p2 + 1;
            char *p3 = strchr(comment_start, ';');
            if (!p3) {
                size_t comment_len = read - (comment_start - line);
                changes_put(changes, phone, type,
                            comment_start, comment_len,
                            NULL, 0,
                            change_pool, change_pool_pos);
            } else {
                size_t comment_len = p3 - comment_start;
                char *date_start = p3 + 1;
                size_t date_len = read - (date_start - line);
                changes_put(changes, phone, type,
                            comment_start, comment_len,
                            date_start, date_len,
                            change_pool, change_pool_pos);
            }
        }
        free(line);
        fclose(file);
    }
    return 0;
}

