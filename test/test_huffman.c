#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "unity/unity.h"

#include "tests.h"
#include "../src/util.h"
#include "../src/huffman.h"

// Calculated using a python script
const uint64_t FILE_RANDOM_10K_FREQS[256] = {
    43, 42, 35, 42, 35, 31, 48, 47, 54, 34, 47, 42, 37, 45, 50, 43, 
    40, 33, 42, 24, 34, 37, 46, 41, 36, 32, 40, 38, 41, 37, 39, 45, 
    36, 34, 36, 51, 34, 47, 43, 46, 35, 35, 44, 40, 41, 35, 33, 50, 
    35, 31, 48, 48, 43, 25, 34, 40, 40, 32, 41, 38, 37, 39, 32, 38, 
    44, 35, 32, 35, 35, 31, 45, 41, 35, 42, 44, 35, 38, 31, 48, 40, 
    37, 40, 51, 43, 36, 39, 58, 44, 35, 42, 54, 37, 30, 32, 36, 29, 
    36, 34, 35, 42, 40, 32, 50, 40, 33, 41, 46, 39, 33, 29, 36, 51, 
    42, 48, 41, 41, 37, 31, 46, 38, 38, 39, 38, 50, 37, 40, 37, 34, 
    32, 42, 27, 47, 49, 51, 38, 38, 52, 47, 57, 37, 47, 45, 37, 44, 
    42, 29, 41, 48, 37, 42, 34, 42, 28, 50, 41, 48, 32, 35, 43, 42, 
    46, 30, 40, 41, 37, 50, 45, 42, 43, 42, 35, 33, 47, 49, 41, 41, 
    36, 44, 45, 46, 36, 40, 30, 33, 39, 45, 45, 51, 29, 45, 58, 36, 
    40, 28, 51, 33, 46, 44, 52, 33, 60, 36, 41, 44, 40, 39, 44, 40, 
    36, 47, 41, 42, 40, 40, 30, 33, 52, 39, 39, 41, 41, 32, 46, 33, 
    47, 46, 30, 46, 41, 36, 42, 45, 39, 37, 39, 57, 36, 45, 33, 30, 
    43, 28, 45, 32, 38, 48, 44, 41, 34, 35, 48, 39, 41, 29, 39, 31
};

const size_t TREE_SERIALIZATION_SIZE = 320;

// Calculated using a python script
const uint8_t INVERTED_CODING_SERIALIZATION[320] = {
    0x0,  0xff, 0xff, 0x9f, 0xdf, 0xe1, 0xfb, 0xfd, 0x3f, 0x3f, 0x81, 0xf7, 0xfb, 0x3e, 0xbf, 0x43,
    0xe7, 0xf2, 0x7c, 0x7e, 0x1,  0xef, 0xf7, 0x3d, 0xbe, 0xc3, 0xd7, 0xea, 0x7a, 0x7d, 0x3,  0xcf,
    0xe6, 0x79, 0x7c, 0x87, 0x8f, 0xc4, 0xf0, 0xf8, 0x1,  0xdf, 0xef, 0x3b, 0xbd, 0xc3, 0xb7, 0xda,
    0x76, 0x7b, 0x3,  0xaf, 0xd6, 0x75, 0x7a, 0x87, 0x4f, 0xa4, 0xe8, 0xf4, 0x3,  0x9f, 0xce, 0x73,
    0x79, 0x87, 0x2f, 0x94, 0xe4, 0xf2, 0x7,  0x1f, 0x8c, 0xe2, 0xf1, 0xe,  0x1f, 0x9,  0xc1, 0xe0,
    0x1,  0xbf, 0xdf, 0x37, 0xbb, 0xc3, 0x77, 0xba, 0x6e, 0x77, 0x3,  0x6f, 0xb6, 0x6d, 0x76, 0x86,
    0xcf, 0x64, 0xd8, 0xec, 0x3,  0x5f, 0xae, 0x6b, 0x75, 0x86, 0xaf, 0x54, 0xd4, 0xea, 0x6,  0x9f,
    0x4c, 0xd2, 0xe9, 0xd,  0x1e, 0x89, 0xa1, 0xd0, 0x3,  0x3f, 0x9e, 0x67, 0x73, 0x86, 0x6f, 0x34,
    0xcc, 0xe6, 0x6,  0x5f, 0x2c, 0xca, 0xe5, 0xc,  0x9e, 0x49, 0x91, 0xc8, 0x6,  0x3f, 0x1c, 0xc6,
    0xe3, 0xc,  0x5e, 0x29, 0x89, 0xc4, 0xc,  0x3e, 0x19, 0x85, 0xc2, 0x18, 0x3c, 0x13, 0x3,  0x80,
    0x1,  0x7f, 0xbf, 0x2f, 0xb7, 0xc2, 0xf7, 0x7a, 0x5e, 0x6f, 0x2,  0xef, 0x76, 0x5d, 0x6e, 0x85,
    0xce, 0xe4, 0xb8, 0xdc, 0x2,  0xdf, 0x6e, 0x5b, 0x6d, 0x85, 0xae, 0xd4, 0xb4, 0xda, 0x5,  0x9e,
    0xcc, 0xb2, 0xd9, 0xb,  0x1d, 0x89, 0x61, 0xb0, 0x2,  0xbf, 0x5e, 0x57, 0x6b, 0x85, 0x6e, 0xb4,
    0xac, 0xd6, 0x5,  0x5e, 0xac, 0xaa, 0xd5, 0xa,  0x9d, 0x49, 0x51, 0xa8, 0x5,  0x3e, 0x9c, 0xa6,
    0xd3, 0xa,  0x5d, 0x29, 0x49, 0xa4, 0xa,  0x3d, 0x19, 0x45, 0xa2, 0x14, 0x3a, 0x12, 0x83, 0x40,
    0x2,  0x7f, 0x3e, 0x4f, 0x67, 0x84, 0xee, 0x74, 0x9c, 0xce, 0x4,  0xde, 0x6c, 0x9a, 0xcd, 0x9,
    0x9c, 0xc9, 0x31, 0x98, 0x4,  0xbe, 0x5c, 0x96, 0xcb, 0x9,  0x5c, 0xa9, 0x29, 0x94, 0x9,  0x3c,
    0x99, 0x25, 0x92, 0x12, 0x39, 0x12, 0x43, 0x20, 0x4,  0x7e, 0x3c, 0x8e, 0xc7, 0x8,  0xdc, 0x69,
    0x19, 0x8c, 0x8,  0xbc, 0x59, 0x15, 0x8a, 0x11, 0x38, 0x92, 0x23, 0x10, 0x8,  0x7c, 0x39, 0xd,
    0x86, 0x10, 0xb8, 0x52, 0x13, 0x8,  0x10, 0x78, 0x32, 0xb,  0x4,  0x20, 0x70, 0x24, 0x6,  0x0
};

// Calculated using a python script
const uint8_t ALL_LENGTHS_CODING_SERIALIZATION[320] = {
    0x20, 0x24, 0xc,  0x82, 0x90, 0x72, 0x12, 0x42, 0xc8, 0x69, 0xf,  0x22, 0x24, 0x4c, 0x8a, 0x91, 
    0x72, 0x32, 0x46, 0xc8, 0xe9, 0x1f, 0x24, 0x24, 0x8c, 0x92, 0x92, 0x72, 0x52, 0x4a, 0xc9, 0x69, 
    0x2f, 0x26, 0x24, 0xcc, 0x9a, 0x93, 0x72, 0x72, 0x4e, 0xc9, 0xe9, 0x3f, 0x28, 0x25, 0xc,  0xa2, 
    0x94, 0x72, 0x92, 0x52, 0xca, 0x69, 0x4f, 0x2a, 0x25, 0x4c, 0xaa, 0x95, 0x72, 0xb2, 0x56, 0xca, 
    0xe9, 0x5f, 0x2c, 0x25, 0x8c, 0xb2, 0x96, 0x72, 0xd2, 0x5a, 0xcb, 0x69, 0x6f, 0x2e, 0x25, 0xcc, 
    0xba, 0x97, 0x72, 0xf2, 0x5e, 0xcb, 0xe9, 0x7f, 0x30, 0x26, 0xc,  0xc2, 0x98, 0x73, 0x12, 0x62, 
    0xcc, 0x69, 0x8f, 0x32, 0x26, 0x4c, 0xca, 0x99, 0x73, 0x32, 0x66, 0xcc, 0xe9, 0x9f, 0x34, 0x26, 
    0x8c, 0xd2, 0x9a, 0x73, 0x52, 0x6a, 0xcd, 0x69, 0xaf, 0x36, 0x26, 0xcc, 0xda, 0x9b, 0x73, 0x72, 
    0x6e, 0xcd, 0xe9, 0xbf, 0x38, 0x27, 0xc,  0xe2, 0x9c, 0x73, 0x92, 0x72, 0xce, 0x69, 0xcf, 0x3a, 
    0x27, 0x4c, 0xea, 0x9d, 0x73, 0xb2, 0x76, 0xce, 0xe9, 0xdf, 0x3c, 0x27, 0x8c, 0xf2, 0x9e, 0x73, 
    0xd2, 0x7a, 0xcf, 0x69, 0xef, 0x3e, 0x27, 0xcc, 0xfa, 0x9f, 0x73, 0xf2, 0x7e, 0xcf, 0xeb, 0xff, 
    0xfe, 0xfe, 0x7e, 0xbf, 0x1f, 0x6f, 0xa7, 0xcb, 0xe1, 0xee, 0xf6, 0x7a, 0xbd, 0x1e, 0x6f, 0x27, 
    0x8b, 0xc1, 0xde, 0xee, 0x76, 0xbb, 0x1d, 0x6e, 0xa7, 0x4b, 0xa1, 0xce, 0xe6, 0x72, 0xb9, 0x1c, 
    0x6e, 0x27, 0xb,  0x81, 0xbe, 0xde, 0x6e, 0xb7, 0x1b, 0x6d, 0xa6, 0xcb, 0x61, 0xae, 0xd6, 0x6a, 
    0xb5, 0x1a, 0x6d, 0x26, 0x8b, 0x41, 0x9e, 0xce, 0x66, 0xb3, 0x19, 0x6c, 0xa6, 0x4b, 0x21, 0x8e, 
    0xc6, 0x62, 0xb1, 0x18, 0x6c, 0x26, 0xb,  0x1,  0x7e, 0xbe, 0x5e, 0xaf, 0x17, 0x6b, 0xa5, 0xca, 
    0xe1, 0x6e, 0xb6, 0x5a, 0xad, 0x16, 0x6b, 0x25, 0x8a, 0xc1, 0x5e, 0xae, 0x56, 0xab, 0x15, 0x6a, 
    0xa5, 0x4a, 0xa1, 0x4e, 0xa6, 0x52, 0xa9, 0x14, 0x6a, 0x25, 0xa,  0x81, 0x3e, 0x9e, 0x4e, 0xa7, 
    0x13, 0x69, 0xa4, 0xca, 0x61, 0x2e, 0x96, 0x4a, 0xa5, 0x12, 0x69, 0x24, 0x8a, 0x41, 0x1e, 0x8e, 
    0x46, 0xa3, 0x11, 0x68, 0xa4, 0x4a, 0x21, 0xe,  0x86, 0x42, 0xa1, 0x10, 0x68, 0x24, 0xa,  0x0
};

/*****************************************************************************/

void setUp(){};
void tearDown(){};

/**
 * @brief: A helper function that checks whether two **full** trees are equal
 */
static bool _trees_equal(huffman_node *first, huffman_node *second) {
    if (first->left == NULL && second->left == NULL) { // both leaves
        return first->byte == second->byte;
    } else if (first->left == NULL && second->left != NULL) { // one leaf
        return false;
    } else { // both internal
        return _trees_equal(first->left, second->left)
                && _trees_equal(first->right, second->right);
    }
}

/**
 * @brief: A helper function that builds a Huffman tree such that the code of
 *         every byte the inverse its own (8 bits long) binary representation.
 */
static huffman_node *_inverted_coding_tree(uint8_t leftpos, int depth) {
    if (depth == 8) {
        return huffman_node_create(255-leftpos, 0, NULL, NULL);
    }
    huffman_node *left = _inverted_coding_tree(leftpos, depth+1);
    huffman_node *right = _inverted_coding_tree(leftpos+(1<<(7-depth)), depth+1);
    return huffman_node_create(0, 0, left, right);
}

/**
 * @brief: A helper function that builds such codes that the code of every byte
 *         is the inverse its own (8 bits long) binary representation.
 */
static huffman_code *_inverted_coding_codes() {
    huffman_code *codes = calloc(256, sizeof(huffman_code));
    for (int i = 0; i < 256; i++) {
        codes[i].length = 8;
        codes[i].data[0] = 255-i;
    }
    return codes;
}

/**
 * @brief: A helper function that builds a Huffman tree such that the all codes
 *         (except the first two) are of different lengths (1...255)
 */
static huffman_node *_all_lengths_coding_tree() {
    huffman_node *node = huffman_node_create(255, 0, NULL, NULL); // byte 0
    for (int i = 254; i >= 0; i--) {
        huffman_node *leaf = huffman_node_create(i, 0, NULL, NULL); // 1-255
        if (i%2 == 0) {    
            node = huffman_node_create(0, 0, node, leaf); // left for byte 0
        } else {
            node = huffman_node_create(0, 0, leaf, node); // right for byte 0
        }
    }

    return node;
}

/**
 * @brief: A helper function that builds codes  such that the all codes
 *         (except the first two) are of different lengths (1...255).
 */
static huffman_code *_all_lengths_coding_codes() {
    huffman_code *codes = calloc(256, sizeof(huffman_code));
    for (int i = 0; i < 256; i++) {
        codes[i].length = i+1;
        for (int j = 0; j < i/8; j++) {
            codes[i].data[j] = 0b01010101;
        }
        switch (i%8) {
            case 0: codes[i].data[i/8] = 0b10000000; break;
            case 1: codes[i].data[i/8] = 0b00000000; break;
            case 2: codes[i].data[i/8] = 0b01100000; break;
            case 3: codes[i].data[i/8] = 0b01000000; break;
            case 4: codes[i].data[i/8] = 0b01011000; break;
            case 5: codes[i].data[i/8] = 0b01010000; break;
            case 6: codes[i].data[i/8] = 0b01010110; break;
            case 7: codes[i].data[i/8] = 0b01010100; break;
        }
    }
    codes[255].length = 255;
    return codes;
}

/*****************************************************************************/

/**
 * @brief Checks that the created node corresponds to paramters given.
 */
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

/**
 * @brief Inserts the max amonut (256) of nodes. Checks that the size is correct,
 *        every node has been inserted and the heap condition is satisfied.
 */
void test_huffman_heap_insert_256_nodes() {
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

    for (int k = 1; k < 256; k++) {
        free(heap.arr[k]);
    }
}

/**
 * @brief Pops the max amonut (256) of nodes and checks that all nodes are
 *        returned in correct order.
 */
void test_huffman_heap_pop_256_nodes() {
    huffman_heap heap ={};

    uint64_t correct_sum = 0;
    for (int i = 0; i < 257; i++) {
        huffman_node *node = huffman_node_create(0, (i*123456)%654321, NULL, NULL);
        correct_sum += node->freq;
        huffman_heap_insert(&heap, node);
    }

    uint64_t sum = 0;
    uint64_t max = 0;
    for (int i = 0; i < 257; i++) {
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

/*****************************************************************************/

/**
 * @brief Checks the byte frequencies and byte count are calculated correctly
 *        for the random 10K file.
 */
void test_huffman_read_frequencies_file_random_10K() {
    uint64_t total;
    uint64_t *freqs = huffman_read_frequencies(file_random_10K, &total);

    TEST_ASSERT_EQUAL_UINT64(FILE_10K_SIZE, total);
    TEST_ASSERT_EQUAL_UINT64_ARRAY(FILE_RANDOM_10K_FREQS, freqs, 256);
    free(freqs);
}


// A helper function for test_huffman_create_tree_uniform_freqs
static void _perfect_tree_check(huffman_node *node, bool *found, int depth) {
    TEST_ASSERT_NOT_NULL(node);
    if (depth < 8) {
        _perfect_tree_check(node->left, found, depth+1);
        _perfect_tree_check(node->right, found, depth+1);
    } else if (depth == 8) {
        found[node->byte] = true;
    } else {
        TEST_FAIL();
    }
}

/**
 * @brief Checks that the tree generated for uniform byte distribution is
 *        perfect and includes every byte.
 */
void test_huffman_create_tree_uniform_freqs() {
    uint64_t freqs[256]; bool found[256];
    for (int i = 0; i < 256; i++) {
        freqs[i] = 123+(i%6-3); // a bit of noise
        found[i] = false;
    }

    huffman_node* tree = huffman_create_tree(freqs);
    _perfect_tree_check(tree, found, 0);
    for (int i = 0; i < 256; i++) {
        TEST_ASSERT_TRUE(found[i]);
    }
    huffman_destroy_tree(tree);
}

/**
 * @brief Checks that for a heavily biased byte distribution most common bytes
 *        get the shortes codes.
 */
void test_huffman_create_tree_biased_freqs() {
    uint64_t freqs[256];
    for (int i = 0; i < 256; i++) {
        freqs[i] = i;
    }
    freqs[123] = 1234567;
    freqs[234] = 123456;
    
    huffman_node* tree = huffman_create_tree(freqs);
    TEST_ASSERT_NOT_NULL(tree);
    TEST_ASSERT_NOT_NULL(tree->right);
    TEST_ASSERT_EQUAL_UINT8(123, tree->right->byte);
    TEST_ASSERT_NOT_NULL(tree->left);
    TEST_ASSERT_NOT_NULL(tree->left->right);
    TEST_ASSERT_EQUAL_UINT8(234, tree->left->right->byte);
    huffman_destroy_tree(tree);
}

/**
 * @brief Checks that correct codes are found for the inverted coding tree. 
 */
void test_huffman_find_codes_inverted_coding() {
    huffman_node *tree = _inverted_coding_tree(0, 0);
    huffman_code *correct_codes = _inverted_coding_codes();
    
    huffman_code *codes = huffman_find_codes(tree);
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(correct_codes, codes, sizeof(huffman_code), 256);

    free(codes);
    free(correct_codes);
    huffman_destroy_tree(tree);
}

/**
 * @brief Checks that correct codes are found for the all lengths coding tree. 
 */
void test_huffman_find_codes_all_lengths_coding() {
    huffman_node *tree = _all_lengths_coding_tree();
    huffman_code *correct_codes = _all_lengths_coding_codes();
    
    huffman_code *codes = huffman_find_codes(tree);
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(correct_codes, codes, sizeof(huffman_code), 256);

    free(codes);
    free(correct_codes);
    huffman_destroy_tree(tree);
}

static void _test_write_tree(huffman_node *tree, const uint8_t *correct_serialization) {
    FILE *file = tmpfile();
    bitwriter writer = bitwriter_init(file, 0);
    huffman_write_tree(&writer, tree);
    bitwriter_finish(&writer);

    fseek(file, 0, SEEK_SET);
    uint8_t bytes[TREE_SERIALIZATION_SIZE+1];
    size_t bytes_read = fread(bytes, 1, TREE_SERIALIZATION_SIZE+1, file);

    TEST_ASSERT_EQUAL_size_t(TREE_SERIALIZATION_SIZE, bytes_read);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(correct_serialization, bytes, TREE_SERIALIZATION_SIZE);

    fclose(file);
    huffman_destroy_tree(tree);
}

/**
 * @brief Checks that the tree for inverted coding is written/serialized correctly.
 */
void test_huffman_write_tree_inverted_coding() {
    _test_write_tree(_inverted_coding_tree(0, 0), INVERTED_CODING_SERIALIZATION);
}

/**
 * @brief Checks that the tree for all lengths coding is written/serialized correctly.
 */
void test_huffman_write_tree_all_lengths_coding() {
    _test_write_tree(_all_lengths_coding_tree(), ALL_LENGTHS_CODING_SERIALIZATION);
}

static void _test_read_tree(const uint8_t *serialization, huffman_node *correct_tree) {
    FILE *file = tmpfile();
    fwrite(serialization, 1, 320, file);

    bitreader reader = bitreader_init(file, 0);
    huffman_node *tree = huffman_read_tree(&reader);
    bitreader_finish(&reader);

    TEST_ASSERT_TRUE(_trees_equal(correct_tree, tree));

    free(tree);
    fclose(file);
    free(correct_tree);
}

/**
 * @brief Checks that the tree for inverted coding is read/deserialized correctly.
 */
void test_huffman_read_tree_inverted_coding() {
    _test_read_tree(INVERTED_CODING_SERIALIZATION, _inverted_coding_tree(0, 0));
}

/**
 * @brief Checks that the tree for all lengths coding is read/deserialized correctly.
 */
void test_huffman_read_tree_all_lengths_coding() {
    _test_read_tree(ALL_LENGTHS_CODING_SERIALIZATION, _all_lengths_coding_tree());
}

/**
 * @brief Checks that all codes are encoded correctly to a file using inverted coding.
 */
void test_huffman_encode_data_inverted_coding() {
    huffman_code *codes = _inverted_coding_codes();

    FILE *out_file = tmpfile();
    bitwriter writer = bitwriter_init(out_file, 0);
    huffman_encode_data(&writer, file_up_and_down_512B, codes);
    bitwriter_finish(&writer);

    fseek(out_file, 0, SEEK_SET);
    uint8_t out_data[512+1];
    size_t out_bytes_read = fread(out_data, 1, 512+1, out_file);
    
    TEST_ASSERT_EQUAL_size_t(512, out_bytes_read);
    for (int i = 0; i < 256; i++) {
        TEST_ASSERT_EQUAL_UINT8(255-i, out_data[i]);
        TEST_ASSERT_EQUAL_UINT8(255-i, out_data[511-i]);
    }
}

/**
 * @brief Checks that all codes are decoded correctly from a file using inverted coding.
 */
void test_huffman_decode_data_inverted_coding() {
    huffman_node *tree = _inverted_coding_tree(0, 0);

    FILE *out_file = tmpfile();
    bitreader reader = bitreader_init(file_up_and_down_512B, 0);
    huffman_decode_data(&reader, out_file, 512, tree);
    bitreader_finish(&reader);

    fseek(out_file, 0, SEEK_SET);
    uint8_t out_data[512+1];
    size_t out_bytes_read = fread(out_data, 1, 512+1, out_file);
    
    TEST_ASSERT_EQUAL_size_t(512, out_bytes_read);
    for (int i = 0; i < 256; i++) {
        TEST_ASSERT_EQUAL_UINT8(255-i, out_data[i]);
        TEST_ASSERT_EQUAL_UINT8(255-i, out_data[511-i]);
    }

    fclose(out_file);
    huffman_destroy_tree(tree);
}

/**
 * @brief A helper function. Compresses a file, decompresses it and then makes
 *        sure that the result is identical to the original. In addition can
 *        check that the ratio was good enough.
 * @param in_file The file to test with
 * @param size Size of the file in bytes
 * @param target_ratio target compressed/uncompressed ratio.
 */
static void _test_huffman_full(FILE *in_file, size_t size, double target_ratio) {
    FILE* packed_file = tmpfile();
    FILE* out_file = tmpfile();

    compress_result result = huffman_compress(in_file, packed_file);
    huffman_decompress(packed_file, out_file);

    fseek(in_file, 0, SEEK_SET);
    uint8_t *in_bytes = malloc(size);
    fread(in_bytes, 1, size, in_file);

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

void test_huffman_full_correctness_lorem_256B() {
    _test_huffman_full(file_lorem_256B, 256, 100);
}

void test_huffman_full_correctness_random_10K() {
    _test_huffman_full(file_random_10K, FILE_10K_SIZE, 100);
}

void test_huffman_full_correctness_random_5M() {
    _test_huffman_full(file_random_5M, FILE_5M_SIZE, 100);
}

void test_huffman_full_correctness_empty_file() {
    _test_huffman_full(file_empty, 0, 100);
}

void test_huffman_full_correctness_only_ones_10K() {
    _test_huffman_full(file_only_ones_10K, FILE_10K_SIZE, 100);
}

void test_huffman_full_correctness_and_ratio_ebook() {
    _test_huffman_full(
        file_pride_and_prejudice,
        FILE_PRIDE_AND_PREJUDICE_SIZE,
        0.6
    );
}