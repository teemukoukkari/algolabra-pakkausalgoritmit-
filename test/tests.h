#pragma once

#include <stdio.h>
#include <sys/types.h>

extern FILE *file_up_and_down_512B;
extern FILE *file_lorem_256B;
extern FILE *file_random_10K;
extern FILE *file_random_5M;
extern FILE *file_empty;
extern FILE *file_only_ones_10K;
extern FILE *file_pride_and_prejudice;

#define FILE_10K_SIZE 10240
#define FILE_5M_SIZE 5242880
#define FILE_PRIDE_AND_PREJUDICE_SIZE 757507

/*****************************************************************************/

void test_huffman_node_create();
void test_huffman_heap_insert_256_nodes();
void test_huffman_heap_pop_256_nodes();

void test_huffman_read_frequencies_file_random_10K();
void test_huffman_create_tree_uniform_freqs();
void test_huffman_create_tree_biased_freqs();
void test_huffman_find_codes_inverted_coding();
void test_huffman_find_codes_all_lengths_coding();

void test_huffman_write_tree_inverted_coding();
void test_huffman_write_tree_all_lengths_coding();
void test_huffman_read_tree_inverted_coding();
void test_huffman_read_tree_all_lengths_coding();

void test_huffman_encode_data_inverted_coding();
void test_huffman_decode_data_inverted_coding();

void test_huffman_full_correctness_lorem_256B();
void test_huffman_full_correctness_random_10K();
void test_huffman_full_correctness_random_5M();
void test_huffman_full_correctness_empty_file();
void test_huffman_full_correctness_only_ones_10K();
void test_huffman_full_correctness_and_ratio_ebook();

/*****************************************************************************/

void test_lzw_dict_init();
void test_lzw_dict_hash();
void test_lzw_dict_get_single_tokens();
void test_lzw_dict_get_key_with_prefix();
void test_lzw_dict_get_collission();
void test_lzw_dict_get_invalid_key();
void test_lzw_dict_insert_one();
void test_lzw_dict_insert_max_amount();
void test_lzw_dict_insert_when_full();

void test_lzw_table_init();
void test_lzw_table_print();
void test_lzw_table_insert_one();
void test_lzw_table_insert_max_amount();
void test_lzw_table_insert_when_full();

void test_lzw_full_correctness_lorem_256B();
void test_lzw_full_correctness_random_10K();
void test_lzw_full_correctness_random_5M();
void test_lzw_full_correctness_empty_file();
void test_lzw_full_correctness_only_ones_10K();
void test_lzw_full_correctness_and_ratio_ebook();