#include "huffman.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
        codes->code[node->byte] = code;
        codes->length[node->byte] = length;
        return;
    }
    
    _find_codes_helper(codes, node->left, code, length+1);
    code |= 1 << length;
    _find_codes_helper(codes, node->right, code, length+1);
}

huffman_codes huffman_find_codes(huffman_node tree) {
    huffman_codes codes;
    _find_codes_helper(&codes, tree, 0, 0);
    return codes;
}