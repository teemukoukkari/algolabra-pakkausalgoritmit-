#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "unity/unity.h"

#include "tests.h"
#include "../src/util.h"
#include "../src/lzw.h"

/**
 * @brief Checks that the dictionary is intialized correctly, that is size,
 *        code_length and keys for codes 0-255 are correct.
 */
void test_lzw_dict_init() {
    lzw_dict dict = lzw_dict_init();

    TEST_ASSERT_EQUAL(dict.size, 257);
    TEST_ASSERT_EQUAL(dict.code_length, 9);

    for (int i = 0; i < 256; i++) {
        bool found = false;
        for (int j = 0; j < LZW_DICT_SIZE; j++) {
            if (dict.arr[j].value == i 
                && dict.arr[j].prefix == LZW_NULL
                && dict.arr[j].token == i) {
                
                found = true;
                break;
            }
        }
        TEST_ASSERT_TRUE(found);
    }

    lzw_dict_destroy(&dict);
}

/**
 * @brief Checks that the distribution of hash function is perect, when tested
 *        using every possible key.
 */
void test_lzw_dict_hash() {
    uint32_t distribution[1024] = {};
    for (int prefix = 0; prefix < 65536; prefix++) {
        for (int token = 0; token < 256; token++) {
            distribution[lzw_dict_hash(prefix,token)%1024]++;
        }
    }

    for (int i = 0; i < 1024; i++) {
        TEST_ASSERT_EQUAL((65536*256)/1024, distribution[i]);
    }
}

/**
 * @brief Checks that the function works correctly for (LZW_NULL, 0-255) keys.
 */
void test_lzw_dict_get_single_tokens() {
    lzw_dict dict = lzw_dict_init();
    
    for (int i = 0; i < 256; i++) {
        uint32_t free_pos;
        uint16_t res = lzw_dict_get(&dict, LZW_NULL, i, &free_pos);
        TEST_ASSERT_EQUAL_UINT16(res, i);
    }

    lzw_dict_destroy(&dict);
}

/**
 * @brief Checks that the function works correctly for (543, 210) key.
 */
void test_lzw_dict_get_key_with_prefix() {
    lzw_dict dict = lzw_dict_init();
    
    uint16_t pos = lzw_dict_hash(543, 210) % LZW_DICT_SIZE;
    dict.arr[pos].value = 7777;
    dict.arr[pos].prefix = 543;
    dict.arr[pos].token = 210;

    uint32_t free_pos;
    uint16_t res = lzw_dict_get(&dict, 543, 210, &free_pos);

    TEST_ASSERT_EQUAL(7777, res);

    lzw_dict_destroy(&dict);
}

/**
 * @brief Checks that the function works correctly in case of collission.
 */
void test_lzw_dict_get_collission() {
    lzw_dict dict = lzw_dict_init();
    
    uint16_t pos = lzw_dict_hash(543, 210) % LZW_DICT_SIZE;
    dict.arr[pos].value = 3333;
    dict.arr[pos].prefix = 333;
    dict.arr[pos].token = 33;

    dict.arr[pos+1].value = 7777;
    dict.arr[pos+1].prefix = 543;
    dict.arr[pos+1].token = 210;

    uint32_t free_pos;
    uint16_t res = lzw_dict_get(&dict, 543, 210, &free_pos);

    TEST_ASSERT_EQUAL(7777, res);

    lzw_dict_destroy(&dict);
}

/**
 * @brief Checks that LZW_NULL and free_pos are returned when key not in dict.
 */
void test_lzw_dict_get_invalid_key() {
    lzw_dict dict = lzw_dict_init();
    
    uint32_t free_pos;
    uint16_t res = lzw_dict_get(&dict, 543, 210, &free_pos);

    uint32_t correct_pos = lzw_dict_hash(543, 210) % LZW_DICT_SIZE;
    TEST_ASSERT_EQUAL_UINT16(res, LZW_NULL);
    TEST_ASSERT_EQUAL(free_pos, correct_pos);

    lzw_dict_destroy(&dict);
}

/**
 * @brief Checks that the function works correctly when inserting one key.
 */
void test_lzw_dict_insert_one() {
    lzw_dict dict = lzw_dict_init();

    lzw_dict_insert(&dict, 543, 210, LZW_NULL);
    TEST_ASSERT_EQUAL_UINT16(258, dict.size);

    for (int i = 0; i < LZW_DICT_SIZE; i++) {
        if (dict.arr[i].value == 257
            && dict.arr[i].prefix == 543
            && dict.arr[i].token == 210) {
            
            lzw_dict_destroy(&dict);
            return;
        }
    }

    lzw_dict_destroy(&dict);
    TEST_FAIL();
}

/**
 * @brief Checks that the function inserts LZW_CODEWORD_COUNT entries successfully.
 */
void test_lzw_dict_insert_max_amount() {
    lzw_dict dict = lzw_dict_init();

    for (int i = 257; i < LZW_CODEWORD_COUNT; i++) {
        lzw_dict_insert(&dict, i/256, i%256, LZW_NULL);
    }

    bool *found = calloc(LZW_CODEWORD_COUNT, sizeof(bool));
    for (int i = 257; i < LZW_CODEWORD_COUNT; i++) {
        if (dict.arr[i].prefix == LZW_NULL) continue;
        if (dict.arr[i].value == LZW_NULL) continue;
        found[dict.arr[i].prefix*256+dict.arr[i].token] = true;
    }

    TEST_ASSERT_EQUAL_UINT32(dict.size, LZW_CODEWORD_COUNT);
    TEST_ASSERT_EQUAL_UINT8(LZW_CODEWORD_LENGTH_LIMIT, dict.code_length);

    for (int i = 257; i < LZW_CODEWORD_COUNT; i++) {
        TEST_ASSERT_TRUE(found[i]);
    }

    free(found);
    lzw_dict_destroy(&dict);
}

/**
 * @brief Checks that no new codes are added after LZW_CODEWORD_LIMIT.
 */
void test_lzw_dict_insert_when_full() {
    lzw_dict dict = lzw_dict_init();
    for (int i = 257; i < LZW_CODEWORD_COUNT+1; i++) {
        lzw_dict_insert(&dict, i/256, i%256, LZW_NULL);
    }
    TEST_ASSERT_EQUAL(dict.size, LZW_CODEWORD_COUNT);
    lzw_dict_destroy(&dict);
}

/**
 * @brief Checks that the lookup table is intiliazed correctly with codes 0-255
*/
void test_lzw_table_init() {
    lzw_table table = lzw_table_init();

    TEST_ASSERT_EQUAL_UINT16(257, table.size);
    TEST_ASSERT_EQUAL_UINT16(9, table.code_length);

    for (int i = 0; i <= 256; i++) {
        TEST_ASSERT_EQUAL_UINT16(LZW_NULL, table.arr[i].prefix);
        TEST_ASSERT_EQUAL_UINT8(i, table.arr[i].token);
    }

    lzw_table_destroy(&table);
}

/**
 * @brief Checks that a table entry is printed correctly.
*/
void test_lzw_table_print() {
    lzw_table table = lzw_table_init();
    table.arr[300] = (lzw_table_node) { .prefix = 22, .token = 77 };
    table.arr[301] = (lzw_table_node) { .prefix = 300, .token = 111 };
    table.arr[302] = (lzw_table_node) { .prefix = 301, .token = 88 };
    table.arr[400] = (lzw_table_node) { .prefix = 302, .token = 55 };
    table.arr[401] = (lzw_table_node) { .prefix = 400, .token = 222 };
    table.arr[402] = (lzw_table_node) { .prefix = 401, .token = 33 };

    FILE *file = tmpfile();
    bytewriter writer = bytewriter_init(file, 0);
    uint8_t first_char = lzw_table_print(&table, &writer, 402);
    bytewriter_finish(&writer);

    fseek(file, 0, SEEK_SET);
    uint8_t bytes[7+1];
    size_t byte_count = fread(bytes, 1, 7+1, file);

    TEST_ASSERT_EQUAL_UINT8(22, first_char);
    TEST_ASSERT_EQUAL_size_t(7, byte_count);

    TEST_ASSERT_EQUAL_UINT8(22, bytes[0]);
    TEST_ASSERT_EQUAL_UINT8(77, bytes[1]);
    TEST_ASSERT_EQUAL_UINT8(111, bytes[2]);
    TEST_ASSERT_EQUAL_UINT8(88, bytes[3]);
    TEST_ASSERT_EQUAL_UINT8(55, bytes[4]);
    TEST_ASSERT_EQUAL_UINT8(222, bytes[5]);
    TEST_ASSERT_EQUAL_UINT8(33, bytes[6]);

    lzw_table_destroy(&table);
}

/**
 * @brief Checks that the function works correctly when inserting one entry.
*/
void test_lzw_table_insert_one() {
    lzw_table table = lzw_table_init();

    lzw_table_insert(&table, 60, 50);

    TEST_ASSERT_EQUAL_UINT16(258, table.size);
    TEST_ASSERT_EQUAL_UINT8(9, table.code_length);

    TEST_ASSERT_EQUAL_UINT16(60, table.arr[257].prefix);
    TEST_ASSERT_EQUAL_UINT16(50, table.arr[257].token);
    
    lzw_table_destroy(&table);
}

/**
 * @brief Checks that the function inserts LZW_CODEWORD_COUNT entries successfully.
*/
void test_lzw_table_insert_max_amount() {
    lzw_table table = lzw_table_init();

    for (int i = 257; i < LZW_CODEWORD_COUNT; i++) {
        lzw_table_insert(&table, i/256, i%256);
    }

    for (int i = 257; i < LZW_CODEWORD_COUNT; i++) {
        TEST_ASSERT_EQUAL_UINT16(i/256, table.arr[i].prefix);
        TEST_ASSERT_EQUAL_UINT16(i%256, table.arr[i].token);
    }
    TEST_ASSERT_EQUAL_UINT16(LZW_CODEWORD_COUNT, table.size);
    TEST_ASSERT_EQUAL_UINT8(LZW_CODEWORD_LENGTH_LIMIT, table.code_length);
    
    lzw_table_destroy(&table);
}

/**
 * @brief Checks that no new codes are added after LZW_CODEWORD_LIMIT.
 */
void test_lzw_table_insert_when_full() {
    lzw_table table = lzw_table_init();

    for (int i = 257; i < LZW_CODEWORD_COUNT+1; i++) {
        lzw_table_insert(&table, i/256, i%256);
    }

    TEST_ASSERT_EQUAL(LZW_CODEWORD_COUNT, table.size);
    TEST_ASSERT_EQUAL(LZW_CODEWORD_LENGTH_LIMIT, table.code_length);
    
    lzw_table_destroy(&table);
}

/**
 * @brief A helper function. Compresses a file, decompresses it and then makes
 *        sure that the result is identical to the original. In addition can
 *        check that the ratio was good enough.
 * @param in_file The file to test with
 * @param size Size of the file in bytes
 * @param target_ratio target compressed/uncompressed ratio.
 */
static void _test_lzw_full(FILE *in_file, size_t size, double target_ratio) {
    FILE* packed_file = tmpfile();
    FILE* out_file = tmpfile();

    compress_result result = lzw_compress(in_file, packed_file);
    lzw_decompress(packed_file, out_file);

    fseek(in_file, 0, SEEK_SET);
    uint8_t *in_bytes = malloc(size);
    size_t in_bytes_read = fread(in_bytes, 1, size, in_file);
    TEST_ASSERT_EQUAL_size_t(size, in_bytes_read);

    fseek(out_file, 0, SEEK_SET);
    uint8_t *out_bytes = malloc(size+1);
    size_t out_bytes_read = fread(out_bytes, 1, size+1, out_file);

    if (result.size_before) {
        double ratio = (double)result.size_after / result.size_before;
        TEST_ASSERT_EQUAL_size_t(size, out_bytes_read);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(in_bytes, out_bytes, size);
        TEST_ASSERT_LESS_OR_EQUAL_DOUBLE(target_ratio, ratio);
    } else {
        TEST_ASSERT_EQUAL(0, out_bytes_read);
    }

    free(out_bytes);
    free(in_bytes);
    fclose(out_file);
    fclose(packed_file);
}

void test_lzw_full_correctness_lorem_256B() {
    _test_lzw_full(file_lorem_256B, 256, 100);
}

void test_lzw_full_correctness_random_10K() {
    _test_lzw_full(file_random_10K, FILE_10K_SIZE, 100);
}

void test_lzw_full_correctness_random_5M() {
    _test_lzw_full(file_random_5M, FILE_5M_SIZE, 100);
}

void test_lzw_full_correctness_empty_file() {
    _test_lzw_full(file_empty, 0, 100);
}

void test_lzw_full_correctness_only_ones_10K() {
    _test_lzw_full(file_only_ones_10K, FILE_10K_SIZE, 100);
}

void test_lzw_full_correctness_and_ratio_ebook() {
    _test_lzw_full(
        file_pride_and_prejudice,
        FILE_PRIDE_AND_PREJUDICE_SIZE,
        0.5
    );
}