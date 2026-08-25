/**
 * @file load.c
 * @brief Реализация параллельной загрузки основных файлов.
 * 
 * Использует mmap для быстрого чтения, разбивает файлы на чанки, запускает
 * несколько потоков для парсинга строк. Результат – массив Record и пул строк.
 */
#include "load.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
    const char *data;
    size_t start, end;
} Chunk;

typedef struct {
    Chunk *chunks;
    size_t num_chunks;
    atomic_size_t chunk_idx;
    Record *records;
    atomic_size_t *record_idx;
    char *pool;
    atomic_size_t *pool_pos;
    size_t max_records;
} ParseContext;

static void parse_line(const char *line, size_t len, Record *rec,
                       char *pool, size_t *pool_pos) {
    const char *p1 = memchr(line, ';', len);
    if (!p1) return;
    size_t phone_len = p1 - line;
    rec->phone = parse_phone(line, phone_len);

    const char *p2 = memchr(p1 + 1, ';', len - (p1 + 1 - line));
    if (!p2) {
        size_t comment_len = len - (p1 + 1 - line);
        if (comment_len > 0) {
            rec->comment_offset = *pool_pos;
            memcpy(pool + *pool_pos, p1 + 1, comment_len);
            pool[*pool_pos + comment_len] = '\0';
            rec->comment_len = comment_len;
            *pool_pos += comment_len + 1;
        } else {
            rec->comment_offset = 0;
            rec->comment_len = 0;
        }
        rec->date_offset = 0;
        rec->date_len = 0;
        return;
    }
    size_t comment_len = p2 - (p1 + 1);
    if (comment_len > 0) {
        rec->comment_offset = *pool_pos;
        memcpy(pool + *pool_pos, p1 + 1, comment_len);
        pool[*pool_pos + comment_len] = '\0';
        rec->comment_len = comment_len;
        *pool_pos += comment_len + 1;
    } else {
        rec->comment_offset = 0;
        rec->comment_len = 0;
    }
    size_t date_len = len - (p2 + 1 - line);
    if (date_len > 0) {
        rec->date_offset = *pool_pos;
        memcpy(pool + *pool_pos, p2 + 1, date_len);
        pool[*pool_pos + date_len] = '\0';
        rec->date_len = date_len;
        *pool_pos += date_len + 1;
    } else {
        rec->date_offset = 0;
        rec->date_len = 0;
    }
}

static void* parse_worker(void *arg) {
    ParseContext *ctx = (ParseContext*)arg;
    while (1) {
        size_t idx = atomic_fetch_add(&ctx->chunk_idx, 1);
        if (idx >= ctx->num_chunks) break;
        Chunk *chunk = &ctx->chunks[idx];
        const char *data = chunk->data;
        size_t pos = chunk->start;
        size_t end = chunk->end;

        while (pos < end) {
            size_t line_end = pos;
            while (line_end < end && data[line_end] != '\n') line_end++;
            if (line_end > pos) {
                size_t len = line_end - pos;
                size_t rec_idx = atomic_fetch_add(ctx->record_idx, 1);
                if (rec_idx < ctx->max_records) {
                    Record *rec = &ctx->records[rec_idx];
                    size_t pool_pos = atomic_fetch_add(ctx->pool_pos, 0);
                    size_t local_pos = pool_pos;
                    parse_line(data + pos, len, rec, ctx->pool, &local_pos);
                    atomic_fetch_add(ctx->pool_pos, local_pos - pool_pos);
                }
            }
            pos = line_end + 1;
        }
    }
    return NULL;
}

static size_t count_lines_and_prepare_chunks(const char **filenames, size_t num_files,
                                             Chunk **out_chunks, size_t *out_num_chunks,
                                             void ***out_mmaps, size_t *out_mmap_count) {
    size_t total_lines = 0;
    size_t chunk_capacity = 1024;
    Chunk *chunks = malloc(chunk_capacity * sizeof(Chunk));
    size_t chunk_count = 0;

    void **mmaps = NULL;
    size_t mmap_count = 0, mmap_cap = 0;

    for (size_t f = 0; f < num_files; ++f) {
        int fd = open(filenames[f], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "Не удалось открыть файл: %s\n", filenames[f]);
            continue;
        }
        struct stat st;
        if (fstat(fd, &st) < 0) {
            close(fd);
            fprintf(stderr, "Не удалось получить размер файла: %s\n", filenames[f]);
            continue;
        }
        size_t file_size = st.st_size;
        char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        if (data == MAP_FAILED) {
            fprintf(stderr, "Ошибка mmap для файла: %s\n", filenames[f]);
            continue;
        }
        if (mmap_count >= mmap_cap) {
            mmap_cap = mmap_cap ? mmap_cap*2 : 16;
            mmaps = realloc(mmaps, mmap_cap * sizeof(void*));
        }
        mmaps[mmap_count++] = data;

        size_t lines = 0;
        for (size_t i = 0; i < file_size; ++i)
            if (data[i] == '\n') lines++;
        total_lines += lines;

        const size_t CHUNK_SIZE = 256ULL * 1024 * 1024;
        size_t pos = 0;
        while (pos < file_size) {
            size_t end = pos + CHUNK_SIZE;
            if (end >= file_size) end = file_size;
            else {
                while (end < file_size && data[end] != '\n') end++;
                if (end < file_size) end++;
            }
            if (chunk_count >= chunk_capacity) {
                chunk_capacity *= 2;
                chunks = realloc(chunks, chunk_capacity * sizeof(Chunk));
            }
            chunks[chunk_count].data = data;
            chunks[chunk_count].start = pos;
            chunks[chunk_count].end = end;
            chunk_count++;
            pos = end;
        }
    }
    *out_chunks = chunks;
    *out_num_chunks = chunk_count;
    *out_mmaps = mmaps;
    *out_mmap_count = mmap_count;
    return total_lines;
}

int load_from_files(const char **filenames, size_t num_files,
                    Record **out_records, size_t *out_count,
                    char **out_pool, size_t *out_pool_size) {
    Chunk *chunks = NULL;
    size_t num_chunks = 0;
    void **mmaps = NULL;
    size_t mmap_count = 0;
    size_t total_lines = count_lines_and_prepare_chunks(filenames, num_files,
                                                        &chunks, &num_chunks,
                                                        &mmaps, &mmap_count);
    if (total_lines == 0 || num_chunks == 0) {
        free(chunks);
        free(mmaps);
        return -1;
    }

    Record *records = malloc(total_lines * sizeof(Record));
    if (!records) { free(chunks); free(mmaps); return -1; }
    size_t pool_capacity = 16ULL * 1024 * 1024 * 1024;
    char *pool = malloc(pool_capacity);
    if (!pool) { free(records); free(chunks); free(mmaps); return -1; }

    atomic_size_t record_idx = 0;
    atomic_size_t pool_pos = 0;

    ParseContext ctx = {
        .chunks = chunks,
        .num_chunks = num_chunks,
        .chunk_idx = 0,
        .records = records,
        .record_idx = &record_idx,
        .pool = pool,
        .pool_pos = &pool_pos,
        .max_records = total_lines
    };

    int num_threads = 24;
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; ++i)
        pthread_create(&threads[i], NULL, parse_worker, &ctx);
    for (int i = 0; i < num_threads; ++i)
        pthread_join(threads[i], NULL);

    free(chunks);
    free(mmaps);

    *out_records = records;
    *out_count = atomic_load(&record_idx);
    *out_pool = pool;
    *out_pool_size = atomic_load(&pool_pos);
    return 0;
}

