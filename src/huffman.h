#include <stdint.h>
#include <stdio.h>

#include "util.h"

/*****************************************************************************/

typedef struct huffman_node {
    uint8_t byte;
    uint64_t freq;
    struct huffman_node *left;
    struct huffman_node *right;
} huffman_node;

typedef struct huffman_heap {
    huffman_node *arr[258];
    int size;
} huffman_heap;

huffman_node *huffman_node_create(
    uint8_t byte, uint64_t freq, huffman_node *left, huffman_node *right
);
void huffman_tree_destroy(huffman_node *tree);

void huffman_heap_insert(huffman_heap *heap, huffman_node *node);
huffman_node *huffman_heap_pop(huffman_heap *heap);

/*****************************************************************************/

typedef struct huffman_code {
    uint8_t length;
    uint8_t data[32];
} huffman_code;

uint64_t *huffman_read_frequencies(FILE *file, uint64_t *total);
huffman_node *huffman_create_tree(uint64_t *freqs);
huffman_code *huffman_find_codes(huffman_node *tree);

void huffman_write_tree(bitwriter *writer, huffman_node *tree);
void huffman_write_data(bitwriter *writer, FILE *in_file, huffman_code *codes);
void huffman_encode(FILE *in, FILE *out);

/*****************************************************************************/

huffman_node *huffman_read_tree(bitreader *reader);
uint8_t huffman_read_code(bitreader *reader, huffman_node *node);
void huffman_decode_data(
    bitreader *reader, FILE *out_file, uint64_t byte_count, huffman_node *tree
);
void huffman_decode(FILE *in_file, FILE *out_file);