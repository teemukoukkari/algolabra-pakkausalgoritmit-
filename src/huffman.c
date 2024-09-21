#include "huffman.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "util.h"

#define BLOCK_SIZE 16384

huffman_node huffman_node_create(uint8_t byte, uint64_t freq, huffman_node left, huffman_node right) {
    huffman_node node = malloc(sizeof(huffman_node_st));
    node->byte = byte;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

void huffman_destroy_tree(huffman_node tree) {
    if (tree == NULL) return;
    huffman_destroy_tree(tree->left);
    huffman_destroy_tree(tree->right);
    free(tree);
}

void huffman_heap_insert(huffman_heap *heap, huffman_node node) {
    int k = heap->size+1;
    heap->arr[k] = node;
    while (k > 1 && heap->arr[k/2]->freq > heap->arr[k]->freq) {
        huffman_node temp = heap->arr[k/2];
        heap->arr[k/2] = heap->arr[k];
        heap->arr[k] = temp;
        k /= 2;
    }
    heap->size += 1;
}

huffman_node huffman_heap_pop(huffman_heap *heap) {
    huffman_node node = heap->arr[1];

    heap->arr[1] = heap->arr[heap->size];
    heap->arr[heap->size] = NULL;
    heap->size -= 1;

    int k = 1;
    while (k < 258/2) {
        int min = k;
        if (heap->arr[2*k] && heap->arr[2*k]->freq < heap->arr[min]->freq) {
            min = 2*k;
        }
        if (heap->arr[2*k+1] && heap->arr[2*k+1]->freq < heap->arr[min]->freq) {
            min = 2*k+1;
        }

        if (min == k) break;

        huffman_node temp = heap->arr[min];
        heap->arr[min] = heap->arr[k];
        heap->arr[k] = temp;
        k = min;
    }

    return node;
}


huffman_freqs huffman_read_frequencies(FILE* f) {
    huffman_freqs freqs = {};

    size_t bytes_read;
    uint8_t *buffer = malloc(BLOCK_SIZE);
    fseek(f, 0, SEEK_SET);
    do {
        bytes_read = fread(buffer, 1, BLOCK_SIZE, f);
        for (size_t i = 0; i < bytes_read; i++) {
            freqs.arr[buffer[i]]++;
        }
    } while(bytes_read);
    free(buffer);
    
    return freqs;
}

huffman_node huffman_create_tree(const huffman_freqs *freqs) {
    huffman_heap heap = {};
    for (int i = 0; i < 256; i++) {
        huffman_heap_insert(&heap, huffman_node_create(i, freqs->arr[i], NULL, NULL));
    }

    huffman_node left, right, new;
    while (heap.size > 1) {
        left = huffman_heap_pop(&heap);
        right = huffman_heap_pop(&heap);
        new = huffman_node_create(0, left->freq+right->freq, left, right);
        huffman_heap_insert(&heap, new);
    }

    return huffman_heap_pop(&heap);
}

static void _find_codes_helper(huffman_codes* codes, huffman_node node, uint32_t code, uint8_t length) {
    if (node == NULL) return;

    if (node->left == NULL && node->right == NULL) {
        if (length > 32) {
            printf("ERROR: too long code, must fix lol\n");
            exit(1);
        }

        codes->code[node->byte] = code;
        codes->length[node->byte] = length;
        return;
    }
    
    _find_codes_helper(codes, node->left, code, length+1);
    code |= (1 << (31-length));
    _find_codes_helper(codes, node->right, code, length+1);
}

huffman_codes huffman_find_codes(huffman_node tree) {
    huffman_codes codes;
    _find_codes_helper(&codes, tree, 0, 0);
    return codes;
}

void huffman_write_tree(huffman_node node, bitwriter *writer) {
    if (node->left == NULL) {
        bitwriter_write8(writer, 0b10000000, 1);
        bitwriter_write8(writer, node->byte, 8);
    } else {
        bitwriter_write8(writer, 0, 1);
        huffman_write_tree(node->left, writer);
        huffman_write_tree(node->right, writer);
    }
}

size_t huffman_write_data(FILE *file, huffman_codes *codes, bitwriter *writer) {
    size_t total_bytes = 0;

    size_t bytes_read;
    uint8_t *buffer = malloc(BLOCK_SIZE);
    fseek(file, 0, SEEK_SET);
    do {
        bytes_read = fread(buffer, 1, BLOCK_SIZE, file);
        total_bytes += bytes_read;
        
        for (size_t i = 0; i < bytes_read; i++) {
            bitwriter_write32(writer, codes->code[buffer[i]], codes->length[buffer[i]]);
        }
    } while(bytes_read);
    free(buffer);

    return total_bytes;
}

void huffman_encode(FILE *in, FILE *out) {
    huffman_freqs freqs = huffman_read_frequencies(in);
    huffman_node tree = huffman_create_tree(&freqs);
    huffman_codes codes = huffman_find_codes(tree);

    bitwriter writer = bitwriter_init(out, sizeof(uint64_t));
    huffman_write_tree(tree, &writer);
    uint64_t original_size = huffman_write_data(in, &codes, &writer);
    bitwriter_finish(&writer);

    fseek(out, 0, SEEK_SET);
    fwrite(&original_size, sizeof(uint64_t), 1, out);

    huffman_destroy_tree(tree);
}

huffman_node huffman_read_tree(bitreader *reader) {
    uint8_t is_leaf = bitreader_read8(reader, 1);
    if (is_leaf) {
        uint8_t byte = bitreader_read8(reader, 8);
        return huffman_node_create(byte, 0, NULL, NULL);
    } else {
        huffman_node left = huffman_read_tree(reader);
        huffman_node right = huffman_read_tree(reader);
        return huffman_node_create(0, 0, left, right);
    }
    return NULL;
}

uint8_t huffman_tree_get(huffman_node node, uint32_t code, uint8_t *depth) {
    if (node->left == NULL && node->right == NULL) {
        return node->byte;
    }
    
    *depth += 1;
    if (code & (1<<31)) {
        return huffman_tree_get(node->right, code << 1, depth);
    } else {
        return huffman_tree_get(node->left, code << 1, depth);
    }
}

void huffman_decode_data(FILE *out_file, uint64_t byte_count, huffman_node tree, bitreader *reader) {
    size_t current_byte = 0;
    uint8_t *buffer = malloc(BLOCK_SIZE);

    uint32_t code = bitreader_read32(reader, 32);
    for (uint64_t i = 0; i < byte_count; i++) {
        uint8_t depth = 0;
        uint8_t byte = huffman_tree_get(tree, code, &depth);

        buffer[current_byte++] = byte;
        if (current_byte == BLOCK_SIZE) {
            fwrite(buffer, 1, BLOCK_SIZE, out_file);
            current_byte = 0;
        }

        code = code << depth;
        code |= bitreader_read32(reader, depth) >> (32-depth);
    }

    fwrite(buffer, 1, current_byte, out_file);
    free(buffer);
}

void huffman_decode(FILE *in_file, FILE *out_file) {
    uint64_t original_size;
    fseek(in_file, 0, SEEK_SET);
    fread(&original_size, sizeof(uint64_t), 1, in_file);

    bitreader reader = bitreader_init(in_file, sizeof(uint64_t));
    huffman_node tree = huffman_read_tree(&reader);
    huffman_decode_data(out_file, original_size, tree, &reader);
}   