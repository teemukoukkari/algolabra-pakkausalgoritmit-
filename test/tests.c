#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "unity/unity.h"
#include "unity/unity_internals.h"
#include "../src/huffman.h"

uint64_t SAMPLE_FREQS[256] = {
    0, 1, 1, 2, 1, 3, 1, 0, 0, 1, 3, 0, 0, 1, 0, 2, 
    0, 2, 1, 1, 0, 0, 2, 1, 0, 0, 3, 0, 0, 1, 1, 0, 
    2, 2, 1, 0, 0, 1, 2, 1, 3, 2, 0, 2, 1, 0, 2, 0, 
    2, 0, 0, 2, 1, 2, 2, 2, 0, 2, 1, 0, 0, 1, 2, 3, 
    1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 2, 0, 1, 1, 
    2, 0, 0, 1, 1, 2, 1, 1, 2, 2, 1, 2, 0, 1, 2, 2, 
    0, 0, 0, 0, 0, 1, 1, 2, 3, 2, 1, 1, 0, 1, 1, 2, 
    0, 5, 0, 1, 1, 1, 0, 0, 2, 0, 1, 0, 3, 1, 1, 2, 
    0, 3, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 2, 
    0, 1, 1, 2, 1, 0, 1, 2, 0, 0, 0, 1, 2, 0, 1, 0, 
    3, 1, 0, 3, 0, 1, 1, 0, 2, 1, 1, 2, 0, 2, 1, 2, 
    2, 3, 1, 0, 1, 2, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 
    3, 0, 0, 4, 1, 1, 1, 0, 2, 4, 5, 0, 2, 1, 2, 1, 
    1, 2, 1, 0, 0, 1, 3, 0, 0, 1, 0, 3, 0, 1, 0, 0, 
    1, 1, 0, 1, 2, 1, 0, 2, 1, 1, 0, 0, 1, 2, 0, 1, 
    0, 2, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 4, 0, 1
};

FILE* file_random_256B;
FILE* file_random_1M;

void setUp(){};
void tearDown(){};

void test_huffman_node_create() {
    huffman_node *left = malloc(sizeof(huffman_node));
    huffman_node *right = malloc(sizeof(huffman_node));
    huffman_node *node = huffman_node_create(123, 456, left, right);
    TEST_ASSERT_EQUAL_UINT8(node->byte, 123);
    TEST_ASSERT_EQUAL_UINT64(node->freq, 456);
    TEST_ASSERT_EQUAL_PTR(left, node->left);
    TEST_ASSERT_EQUAL_PTR(right, node->right);
    free(node);
    free(right);
    free(left);
}

void test_huffman_heap_insert() {
    huffman_heap heap = {};

    uint64_t correct_sum = 0;
    for (int i = 0; i < 256; i++) {
        huffman_node *node = huffman_node_create(0, (i*123456)%654321, NULL, NULL);
        correct_sum += node->freq;
        huffman_heap_insert(&heap, node);
    }
    TEST_ASSERT_EQUAL_INT(256, heap.size);

    uint64_t sum = heap.arr[1]->freq;
    for (int k = 2; k < 257; k++) {
        TEST_ASSERT_NOT_NULL(heap.arr[k]);
        TEST_ASSERT_NOT_NULL(heap.arr[k/2]);
        TEST_ASSERT_TRUE(heap.arr[k]->freq >= heap.arr[k/2]->freq);
        sum += heap.arr[k]->freq;
    }
    TEST_ASSERT_EQUAL_UINT64(correct_sum, sum);

    for (int k = 1; k < 257; k++) {
        free(heap.arr[k]);
    }
}

void test_huffman_heap_pop() {
    huffman_heap heap ={};

    uint64_t correct_sum = 0;
    for (int i = 0; i < 256; i++) {
        huffman_node *node = huffman_node_create(0, (i*123456)%654321, NULL, NULL);
        correct_sum += node->freq;
        huffman_heap_insert(&heap, node);
    }

    uint64_t sum = 0;
    uint64_t max = 0;
    for (int i = 0; i < 256; i++) {
        huffman_node *node = huffman_heap_pop(&heap);
        TEST_ASSERT_NOT_NULL(node);
        TEST_ASSERT_TRUE(node->freq >= max);

        sum += node->freq;
        max = node->freq;

        free(node);
    }
    
    TEST_ASSERT_EQUAL_UINT64(correct_sum, sum);
    TEST_ASSERT_EQUAL_INT(0, heap.size);
}

void test_huffman_read_frequencies_256B() {
    uint64_t total;
    uint64_t *freqs = huffman_read_frequencies(file_random_256B, &total);
    TEST_ASSERT_EQUAL_UINT64_ARRAY(SAMPLE_FREQS, freqs, 256);
    TEST_ASSERT_EQUAL_UINT64(256, total);
}

void test_huffman_read_frequencies_1M() {
    const uint64_t CORRECT_SUM = 1048576;

    uint64_t total;
    uint64_t *freqs = huffman_read_frequencies(file_random_1M, &total);

    uint64_t sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += freqs[i];
    }

    TEST_ASSERT_EQUAL_UINT64(total, sum);
    TEST_ASSERT_EQUAL_UINT64(CORRECT_SUM, sum);
}

static int _find_leaf_depth(huffman_node *node, uint8_t leaf, int depth) {
    if (node == NULL) return 0;
    if (node->left == NULL && node->right == NULL && node->byte == leaf) return depth;

    int left = _find_leaf_depth(node->left, leaf, depth+1);
    int right = _find_leaf_depth(node->right, leaf, depth+1);
    if (left) return left;
    if (right) return right;
    return 0;
}

void test_huffman_create_tree() {
    uint64_t *freqs = malloc(256*sizeof(uint64_t));
    memcpy(freqs, SAMPLE_FREQS, 256*sizeof(uint64_t));

    freqs[213] = 50;
    freqs[142] = 40;
    freqs[189] = 30;
    freqs[23] = 20;
    freqs[96] = 10;

    huffman_node *tree = huffman_create_tree(freqs);

    int depth1 = _find_leaf_depth(tree, 213, 0);
    int depth2 = _find_leaf_depth(tree, 142, 0);
    int depth3 = _find_leaf_depth(tree, 189, 0);
    int depth4 = _find_leaf_depth(tree, 23, 0);
    int depth5 = _find_leaf_depth(tree, 96, 0);

    TEST_ASSERT_TRUE(depth1 <= depth2);
    TEST_ASSERT_TRUE(depth2 <= depth3);
    TEST_ASSERT_TRUE(depth3 <= depth4);
    TEST_ASSERT_TRUE(depth4 <= depth5);

    huffman_tree_destroy(tree);
}

void test_huffman_find_codes() {
    huffman_node *tree = huffman_create_tree(SAMPLE_FREQS);
    huffman_code *codes = huffman_find_codes(tree);

    TEST_ASSERT_EQUAL_INT(_find_leaf_depth(tree, 53, 0), codes[53].length);
    TEST_ASSERT_EQUAL_INT(_find_leaf_depth(tree, 75, 0), codes[75].length);
    TEST_ASSERT_EQUAL_INT(_find_leaf_depth(tree, 122, 0), codes[122].length);
    TEST_ASSERT_EQUAL_INT(_find_leaf_depth(tree, 175, 0), codes[175].length);
    TEST_ASSERT_EQUAL_INT(_find_leaf_depth(tree, 210, 0), codes[210].length);
    TEST_ASSERT_EQUAL_INT(_find_leaf_depth(tree, 255, 0), codes[255].length);
    
    huffman_tree_destroy(tree);
}

int main() {
    file_random_256B = fopen("test/files/random_256B.bin", "rb");
    file_random_1M = fopen("test/files/random_1M.bin", "rb");

    UNITY_BEGIN();
    RUN_TEST(test_huffman_node_create);
    RUN_TEST(test_huffman_heap_insert);
    RUN_TEST(test_huffman_heap_pop);
    RUN_TEST(test_huffman_read_frequencies_256B);
    RUN_TEST(test_huffman_read_frequencies_1M);
    RUN_TEST(test_huffman_create_tree);
    RUN_TEST(test_huffman_find_codes);
    UNITY_END();

    fclose(file_random_256B);
    fclose(file_random_1M);

    return 0;
}