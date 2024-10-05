#include "lzw.h"

#include <stdlib.h>
#include <stdbool.h>
#include "util.h"

/*****************************************************************************/

/**
 * @brief Initializes a new dictionary for LZW compressing. Codes 0-255 are
 *        added for keys (LZW_NULL, 0-255). In addition, LZW_EOF=256 is reserved.
 * @return The dictionary. Caller must free the memory using lzw_dict_destroy.
 */
lzw_dict lzw_dict_init() {
    lzw_dict dict = {
        .size = 0,
        .code_length = 9,
        .arr = malloc(LZW_DICT_SIZE*sizeof(lzw_dict_node))
    };
    for (size_t i = 0; i < LZW_DICT_SIZE; i++) {
        dict.arr[i].value = LZW_NULL;
    }

    for (int i = 0; i <= 255; i++) {
        lzw_dict_insert(&dict, LZW_NULL, i, LZW_NULL);
    }
    dict.size += 1; // LZW_EOF=256
    return dict;
}

void lzw_dict_destroy(lzw_dict *dict) {
    free(dict->arr);
}

/**
 * @brief Hash function. Simply the best for our use case.
 */
uint32_t lzw_dict_hash(uint16_t prefix, uint8_t token) {
    return  prefix*256 + token;
}

/**
 * @brief Finds a code from the hash table using (prefix, token)-key, applying
 *        lzw_dict_hash function and linear probing.
 * @param dict Pointer to the dictionary.
 * @param prefix Prefix part of the key. For example key "abcd" -> code of "abc"
 * @param token Token part of the key. For example key "abcd" -> char 'd'
 * @param free_pos Pointer to where the potential position for the key will be
 *                 stored in case the key is not already in the dictionary.
 * @returns The code or LZW_NULL if the key is not in the dictionary.
 */
uint16_t lzw_dict_get(lzw_dict *dict, uint16_t prefix, uint8_t token, uint32_t *free_pos) {
    uint32_t pos = lzw_dict_hash(prefix, token) % LZW_DICT_SIZE;
    while (true) {
        if (dict->arr[pos].value == LZW_NULL) {
            *free_pos = pos;
            return LZW_NULL;
        }
        if (dict->arr[pos].prefix == prefix && dict->arr[pos].token == token) {
            return dict->arr[pos].value;
        } 
        pos = (pos + 1) % LZW_DICT_SIZE;
    }
}

/**
 * @brief Inserts the next code for given (prefix, token) key. The key MUST BE
 *        unique! If no codes left (LZW_CODEWORD_COUNT), does nothing.
 * @param dict Pointer to the dictionary.
 * @param prefix Prefix part of the key. For example key "abcd" -> code of "abc"
 * @param token Token part of the key. For example key "abcd" -> char 'd'
 * @param free_pos The free_pos from lzw_dict_get or LZW_NULL if not available.
 */
void lzw_dict_insert(lzw_dict *dict, uint16_t prefix, uint8_t token, uint32_t free_pos) {
    if (dict->size >= LZW_CODEWORD_COUNT) {
        return;
    }

    if (free_pos == LZW_NULL) {
        lzw_dict_get(dict, prefix, token, &free_pos);
    }

    dict->arr[free_pos].value = dict->size++;
    dict->arr[free_pos].prefix = prefix;
    dict->arr[free_pos].token = token;

    if (dict->size >= (1 << dict->code_length)) {
        dict->code_length++;
    }
}

/*****************************************************************************/

/**
 * @brief Initializes a new lookup table for LZW decompressing. Codes
 *        (LZW_NULL, 255) are added for keys 0-255. Code LZW_EOF=256 is reversed.
 * @return The lookup table. Caller must free the memory using lzw_table_destroy.
 */
lzw_table lzw_table_init() {
    lzw_table table = {
        .size = 257, 
        .code_length = 9,
        .arr = calloc(1, sizeof(lzw_table_node)*LZW_CODEWORD_COUNT)
    };

    for (int i = 0; i <= 256; i++) {
        table.arr[i].prefix = LZW_NULL;
        table.arr[i].token = i;
    }

    return table;
}

void lzw_table_destroy(lzw_table *table) {
    free(table->arr);
}

/**
 * @brief Prints a string from lookup table recursively.
 * @param lzw_table Pointer to the lookup table.
 * @param writer Bytewriter handle to write the bytes to.
 * @param entry The lookup table entry (=code) to print.
 * @return The first byte of the code for later use.
 */
uint8_t lzw_table_print(lzw_table *table, bytewriter *writer, uint16_t entry) {
    uint8_t res = entry;
    if (entry > 255) {
        res = lzw_table_print(table, writer, table->arr[entry].prefix);
    }
    bytewriter_write(writer, table->arr[entry].token);
    return res;
}

/**
 * @brief Inserts the given string to lookup table for next code
 * @param lzw_table Pointer to the lookup table.
 * @param prefix Prefix part of the string. For example key "abcd" -> code of "abc"
 * @param token Token part of the string. For example key "abcd" -> char 'd'
 */
void lzw_table_insert(lzw_table *table, uint16_t prefix, uint8_t token) {
    if (table->size >= LZW_CODEWORD_COUNT) {
        return;
    }

    table->arr[table->size].prefix = prefix;
    table->arr[table->size].token = token;
    table->size++;

    if (table->size+1 >= (1 << table->code_length)) {
        table->code_length++;
    }
}

/*****************************************************************************/

/**
 * @brief Compresseses a file using LZW algorithm
 * @param in_file Source file
 * @param out_file Destination file
 * @return returns original and compressed size
 */
compress_result lzw_compress(FILE *in_file, FILE *out_file) {
    bytereader reader = bytereader_init(in_file, 0);
    bitwriter writer = bitwriter_init(out_file, 0);

    if (!reader.bytes_left) {
        bitwriter_write16r(&writer, LZW_EOF, 9);
        bitwriter_finish(&writer);
        bytereader_finish(&reader);
        return (compress_result) { 0, 0 };
    }

    lzw_dict dict = lzw_dict_init();
    uint16_t last_code = bytereader_read(&reader);

    size_t original_size = 1;
    while (reader.bytes_left) {
        original_size++;
        uint8_t token = bytereader_read(&reader);

        uint32_t free_pos = 0;
        uint16_t code = lzw_dict_get(&dict, last_code, token, &free_pos);
        if (code == LZW_NULL) {
            bitwriter_write16r(&writer, last_code, dict.code_length);
            lzw_dict_insert(&dict, last_code, token, free_pos);
            last_code = token;
        } else {
            last_code = code;
        }
    }

    bitwriter_write16r(&writer, last_code, dict.code_length);
    bitwriter_write16r(&writer, LZW_EOF, dict.code_length);
    size_t compressed_size = bitwriter_finish(&writer);

    lzw_dict_destroy(&dict);
    bytereader_finish(&reader);

    return (compress_result) {
        .size_before = original_size,
        .size_after = compressed_size
    };
}

/**
 * @brief Deompresses a file using LZW algorithm
 * @param in_file Source file
 * @param out_file Destination file
 */
void lzw_decompress(FILE *in_file, FILE *out_file) {
    bitreader reader = bitreader_init(in_file, 0);
    bytewriter writer = bytewriter_init(out_file, 0);
    
    lzw_table table = lzw_table_init();
    
    uint16_t last_code = bitreader_read16r(&reader, table.code_length);
    if (last_code == LZW_EOF) {
        bytewriter_finish(&writer);
        bitreader_finish(&reader);
        return;
    }
    uint8_t last_first_char = lzw_table_print(&table, &writer, last_code);

    while (true) {
        uint16_t code = bitreader_read16r(&reader, table.code_length);
        if (code == LZW_EOF) {
            break;
        }

        uint8_t first_char;
        if (table.size <= code) {
            first_char = lzw_table_print(&table, &writer, last_code);
            lzw_table_print(&table, &writer, last_first_char);
        } else {
            first_char = lzw_table_print(&table, &writer, code);
        }

        lzw_table_insert(&table, last_code, first_char);

        last_code = code;
        last_first_char = first_char;
    }

    bytewriter_finish(&writer);
    bitreader_finish(&reader);
}