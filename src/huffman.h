#include <stdint.h>
#include <stdio.h>

#include "util.h"

typedef struct huffman_freqs {
    uint64_t arr[256];
} huffman_freqs;

typedef struct huffman_node_st {
    uint8_t byte;
    uint64_t freq;
    struct huffman_node_st *left;
    struct huffman_node_st *right;
} huffman_node_st;
typedef huffman_node_st* huffman_node;

typedef struct huffman_heap {
    huffman_node arr[258];
    int size;
} huffman_heap;

typedef struct huffman_codes {    
    uint32_t code[256];
    uint8_t length[256];
} huffman_codes;

huffman_node huffman_node_create(uint8_t byte, uint64_t freq, huffman_node left, huffman_node right);
void huffman_destroy_tree(huffman_node tree);
void huffman_heap_insert(huffman_heap *heap, huffman_node node);
huffman_node huffman_heap_pop(huffman_heap *heap);

huffman_freqs huffman_read_frequencies(FILE *f);
huffman_node huffman_create_tree(const huffman_freqs *freqs);
huffman_codes huffman_find_codes(huffman_node tree);
void huffman_write_tree(huffman_node tree, bitwriter *writer);
size_t huffman_write_data(FILE *file, huffman_codes *codes, bitwriter *writer);
void huffman_encode(FILE *in, FILE *out);

huffman_node huffman_read_tree(bitreader *reader);
uint8_t huffman_tree_get(huffman_node tree, uint32_t code, uint8_t *depth);
void huffman_decode_data(FILE *out_file, uint64_t byte_count, huffman_node tree, bitreader *reader);
void huffman_decode(FILE *in_file, FILE *out_file);