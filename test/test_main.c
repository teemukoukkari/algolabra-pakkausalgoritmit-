#include <stdio.h>
#include <stdlib.h>
#include "unity/unity.h"
#include "tests.h"

#define RUN_TEST_INDENT(x) printf("   ");RUN_TEST(x)

FILE *file_up_and_down_512B;
FILE *file_lorem_256B;
FILE *file_random_10K;
FILE *file_random_5M;
FILE *file_empty;
FILE *file_only_ones_10K;
FILE *file_pride_and_prejudice;

int main() {
    file_up_and_down_512B = fopen("test/files/up_and_down_512B.bin", "rb");
    file_lorem_256B = fopen("test/files/lorem_256B.txt", "rb");
    file_random_10K = fopen("test/files/random_10K.bin", "rb");
    file_random_5M = fopen("test/files/random_5M.bin", "rb");
    file_empty = fopen("test/files/empty.bin", "rb");
    file_only_ones_10K = fopen("test/files/only_ones_10K.bin", "rb");
    file_pride_and_prejudice = fopen("test/files/pride_and_prejudice.txt", "rb");

    if (!file_up_and_down_512B
        || !file_lorem_256B
        || !file_random_10K
        || !file_random_5M
        || !file_empty
        || !file_only_ones_10K
        || !file_pride_and_prejudice) {
        
        printf("ERROR: Failed to open an input file for tests\n");
        exit(0);
    }

    UNITY_BEGIN();
    printf("**************************************************************************\n\n");
    printf("Huffman nodes and heap\n");
    RUN_TEST_INDENT(test_huffman_node_create);
    RUN_TEST_INDENT(test_huffman_heap_insert_256_nodes);
    RUN_TEST_INDENT(test_huffman_heap_pop_256_nodes);
    printf("\n");

    printf("Reading frequencies, creating a Huffman tree and finding codes:\n");
    RUN_TEST_INDENT(test_huffman_read_frequencies_file_random_10K);
    RUN_TEST_INDENT(test_huffman_create_tree_uniform_freqs);
    RUN_TEST_INDENT(test_huffman_create_tree_biased_freqs);
    RUN_TEST_INDENT(test_huffman_find_codes_inverted_coding);
    RUN_TEST_INDENT(test_huffman_find_codes_all_lengths_coding);
    printf("\n");

    printf("Serializing and deserializing a Huffman tree to/from a file\n");
    RUN_TEST_INDENT(test_huffman_write_tree_inverted_coding);
    RUN_TEST_INDENT(test_huffman_write_tree_all_lengths_coding);
    RUN_TEST_INDENT(test_huffman_read_tree_inverted_coding);
    RUN_TEST_INDENT(test_huffman_read_tree_all_lengths_coding);
    printf("\n");

    printf("Encoding/decoding data using a Huffman tree\n");
    RUN_TEST_INDENT(test_huffman_encode_data_inverted_coding);
    RUN_TEST_INDENT(test_huffman_decode_data_inverted_coding);
    printf("\n");

    printf("Full Huffman compression and decompression for files\n");
    RUN_TEST_INDENT(test_huffman_full_correctness_lorem_256B);
    RUN_TEST_INDENT(test_huffman_full_correctness_random_10K);
    RUN_TEST_INDENT(test_huffman_full_correctness_random_5M);
    RUN_TEST_INDENT(test_huffman_full_correctness_empty_file);
    RUN_TEST_INDENT(test_huffman_full_correctness_only_ones_10K);
    RUN_TEST_INDENT(test_huffman_full_correctness_and_ratio_ebook);
    printf("\n");

    printf("**************************************************************************\n\n");

    printf("The dictionary used for LZW compression\n");
    RUN_TEST_INDENT(test_lzw_dict_init);
    RUN_TEST_INDENT(test_lzw_dict_hash);
    RUN_TEST_INDENT(test_lzw_dict_get_single_tokens);
    RUN_TEST_INDENT(test_lzw_dict_get_key_with_prefix);
    RUN_TEST_INDENT(test_lzw_dict_get_collission);
    RUN_TEST_INDENT(test_lzw_dict_get_invalid_key);
    RUN_TEST_INDENT(test_lzw_dict_insert_one);
    RUN_TEST_INDENT(test_lzw_dict_insert_max_amount);
    RUN_TEST_INDENT(test_lzw_dict_insert_when_full);
    printf("\n");

    printf("The code table used for LZW decompression\n");
    RUN_TEST_INDENT(test_lzw_table_init);
    RUN_TEST_INDENT(test_lzw_table_print);
    RUN_TEST_INDENT(test_lzw_table_insert_one);
    RUN_TEST_INDENT(test_lzw_table_insert_max_amount);
    RUN_TEST_INDENT(test_lzw_table_insert_when_full);
    printf("\n");

    printf("Full LZW compression and decompression for files\n");
    RUN_TEST_INDENT(test_lzw_full_correctness_lorem_256B);
    RUN_TEST_INDENT(test_lzw_full_correctness_random_10K);
    RUN_TEST_INDENT(test_lzw_full_correctness_random_5M);
    RUN_TEST_INDENT(test_lzw_full_correctness_empty_file);
    RUN_TEST_INDENT(test_lzw_full_correctness_only_ones_10K);
    RUN_TEST_INDENT(test_lzw_full_correctness_and_ratio_ebook);
    UNITY_END();

    fclose(file_up_and_down_512B);
    fclose(file_lorem_256B);
    fclose(file_random_10K);
    fclose(file_random_5M);
    fclose(file_empty);
    fclose(file_only_ones_10K);
    fclose(file_pride_and_prejudice);

    return 0;
}