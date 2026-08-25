/**
 * @file test.c
 * @brief Тестовая программа, демонстрирующая работу всех модулей.
 * 
 * Создаёт тестовые CSV-файлы, загружает основной набор, инкременты,
 * выполняет поиск и сохраняет результат. Использует assert для проверок.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "types.h"
#include "load.h"
#include "sort.h"
#include "hash.h"
#include "inc.h"
#include "search.h"
#include "save.h"
#include "utils.h"

static void create_test_files() {
    const char *data1 =
        "1234567890ABCDEF;комментарий к первому;2025-12-31\n"
        "A1B2C3D4E5F67890;;\n"
        "112233445566778899;без даты;2026-01-01\n";
    const char *data2 =
        "001122334455667788;второй файл;2024-06-15\n"
        "FFEEDDCCBBAA9988;комментарий;2025-07-20\n";
    const char *inc1 =
        "A;999999999999999999;новый номер;\n"
        "D;1234567890ABCDEF;;\n"
        "U;112233445566778899;обновлённый комментарий;2026-12-31\n";
    const char *inc2 =
        "A;ABCDEF0123456789;;2027-01-01\n"
        "U;A1B2C3D4E5F67890;изменён коммент;2025-01-01\n"
        "D;001122334455667788;;\n";

    FILE *f;
    f = fopen("data1.csv", "w"); if (f) { fputs(data1, f); fclose(f); }
    f = fopen("data2.csv", "w"); if (f) { fputs(data2, f); fclose(f); }
    f = fopen("inc1.csv", "w"); if (f) { fputs(inc1, f); fclose(f); }
    f = fopen("inc2.csv", "w"); if (f) { fputs(inc2, f); fclose(f); }
}

void run_tests() {
    create_test_files();

    const char *main_files[] = {"data1.csv", "data2.csv", NULL};

    Record *records = NULL;
    size_t count = 0;
    char *pool = NULL;
    size_t pool_size = 0;

    int ret = load_from_files(main_files, 2, &records, &count, &pool, &pool_size);
    assert(ret == 0);
    printf("Загружено %zu записей из основных файлов\n", count);

    parallel_sort(records, count);
    printf("Сортировка завершена\n");

    ChangeHash *changes = changes_create(1 << 10);
    char *change_pool = malloc(1 << 20);
    size_t change_pool_pos = 0;

    const char *inc_files[] = {"inc1.csv", "inc2.csv", NULL};
    ret = load_increments_from_files(inc_files, 2, changes, change_pool, &change_pool_pos);
    assert(ret == 0);
    printf("Инкременты загружены\n");

    phone_t phone1 = parse_phone("1234567890ABCDEF", 16);
    phone_t phone2 = parse_phone("112233445566778899", 18);
    phone_t phone3 = parse_phone("001122334455667788", 18);
    phone_t phone4 = parse_phone("ABCDEF0123456789", 16);
    phone_t phone5 = parse_phone("FFEEDDCCBBAA9988", 16);

    Change *ch = changes_get(changes, phone1);
    assert(ch && ch->type == 0);

    ch = changes_get(changes, phone2);
    assert(ch && ch->type == 2);
    char comment_buf[100];
    snprintf(comment_buf, sizeof(comment_buf), "%.*s", (int)ch->comment_len, change_pool + ch->comment_offset);
    assert(strcmp(comment_buf, "обновлённый комментарий") == 0);

    ch = changes_get(changes, phone3);
    assert(ch && ch->type == 0);

    ch = changes_get(changes, phone4);
    assert(ch && ch->type == 1);

    int idx = binary_search(records, count, phone5);
    assert(idx >= 0);
    char buf[64];
    phone_to_hex(records[idx].phone, buf, sizeof(buf));
    assert(strcmp(buf, "FFEEDDCCBBAA9988") == 0);
    printf("Все проверки поиска пройдены\n");

    ret = save_merged("result.csv", records, count, changes, pool, change_pool);
    assert(ret == 0);
    printf("Результат сохранён в result.csv\n");

    free(records);
    free(pool);
    changes_free(changes);
    free(change_pool);
}

int main() {
    run_tests();
    return 0;
}

