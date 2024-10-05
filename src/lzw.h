#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "util.h"

#define LZW_NULL (uint16_t)(-1)
#define LZW_EOF 256

#define LZW_CODEWORD_LENGTH_LIMIT 14
#define LZW_CODEWORD_COUNT (1 << LZW_CODEWORD_LENGTH_LIMIT)-2
#define LZW_DICT_SIZE (1<<16) // max 2^16

/*****************************************************************************/

typedef struct lzw_dict_node {
    uint16_t value;
    uint16_t prefix;
    uint8_t token;
} lzw_dict_node;

typedef struct lzw_dict {
    uint16_t size;
    uint8_t code_length;
    lzw_dict_node *arr;
} lzw_dict;

lzw_dict lzw_dict_init();
void lzw_dict_destroy(lzw_dict *dict);

uint32_t lzw_dict_hash(uint16_t prefix, uint8_t token);
uint16_t lzw_dict_get(lzw_dict *dict, uint16_t prefix, uint8_t token, uint32_t *free_pos);
void lzw_dict_insert(lzw_dict *dict, uint16_t prefix, uint8_t token, uint32_t free_pos);

/*****************************************************************************/

typedef struct lzw_table_node {
    uint16_t prefix;
    uint8_t token;
} lzw_table_node;

typedef struct lzw_table {
    uint16_t size;
    uint8_t code_length;
    lzw_table_node *arr;
} lzw_table;

lzw_table lzw_table_init();
void lzw_table_destroy(lzw_table *table);

void lzw_table_insert(lzw_table *table, uint16_t index, uint8_t token);
uint8_t lzw_table_print(lzw_table *table, bytewriter *writer, uint16_t entry);

/*****************************************************************************/

compress_result lzw_compress(FILE *in_file, FILE *out_file);
void lzw_decompress(FILE *in_file, FILE *out_file);