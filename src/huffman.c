#include "huffman.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "util.h"

/*****************************************************************************/

/**
 * @brief Creates a huffman heap/tree node.
 * @param byte Byte this node represents (0 if internal node).
 * @param freq Frequency used by the huffman heap as value (0 if not used).
 * @param left Pointer to a child node or NULL.
 * @param right Pointer to a child node or NULL.
 * @return Pointer to the node. Caller must free the memory.
 */
huffman_node *huffman_node_create(uint8_t byte, uint64_t freq, huffman_node *left, huffman_node *right) {
    huffman_node *node = malloc(sizeof(huffman_node));
    node->byte = byte;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

/**
 * @brief Recursively frees all nodes of the tree.
 * @param tree Tree to remove from memory.
 */
void huffman_destroy_tree(huffman_node *tree) {
    if (tree == NULL) return;
    huffman_destroy_tree(tree->left);
    huffman_destroy_tree(tree->right);
    free(tree);
}

/**
 * @brief Adds an node to the minimum heap and sifts it up until heap condition
 *        is satisfied.
 * @param heap Pointer to the heap.
 * @param node The node to insert. Must NOT be freed until popped.
 */
void huffman_heap_insert(huffman_heap *heap, huffman_node *node) {
    int k = heap->size+1;
    heap->arr[k] = node;
    while (k > 1 && heap->arr[k/2]->freq > heap->arr[k]->freq) {
        huffman_node *temp = heap->arr[k/2];
        heap->arr[k/2] = heap->arr[k];
        heap->arr[k] = temp;
        k /= 2;
    }
    heap->size += 1;
}

/**
 * @brief Pops the node with smallest frequency from the heap and fixes the
 *        heap condition by selecting new root node and sifting it down.
 * @param heap Pointer to the heap.
 * @return The popped node. May be freed by the caller.
 */
huffman_node *huffman_heap_pop(huffman_heap *heap) {
    huffman_node *node = heap->arr[1];

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

        huffman_node *temp = heap->arr[min];
        heap->arr[min] = heap->arr[k];
        heap->arr[k] = temp;
        k = min;
    }

    return node;
}

/*****************************************************************************/

/**
 * @brief Reads byte frequencies from the given file.
 * @param file File to read the frequencies from.
 * @param total Pointer to where the total number of bytes will be stored.
 * @return An array of 256 elements representing the frequency of each byte.
 *         Caller must free the memory.
 */
uint64_t *huffman_read_frequencies(FILE* file, uint64_t *total) {
    *total = 0;
    uint64_t *freqs = calloc(256*sizeof(uint64_t), 1);

    bytereader reader = bytereader_init(file, 0);
    while (reader.bytes_left) {
        freqs[bytereader_read(&reader)]++;
        *total += 1;
    }
    bytereader_finish(&reader);

    return freqs;
}

/**
 * @brief Builds a Huffman tree from given byte frequencies.
 * @param freqs An array of 256 elements representing the frequency of each byte.
 * @return The root node of the tree. Caller must free all nodes of the tree.
 */
huffman_node *huffman_create_tree(uint64_t *freqs) {
    huffman_heap heap = {};
    for (int i = 0; i < 256; i++) {
        huffman_node *node = huffman_node_create(i, freqs[i], NULL, NULL);
        huffman_heap_insert(&heap, node);
    }

    huffman_node *left, *right, *new;
    while (heap.size > 1) {
        left = huffman_heap_pop(&heap);
        right = huffman_heap_pop(&heap);
        new = huffman_node_create(0, left->freq+right->freq, left, right);
        huffman_heap_insert(&heap, new);
    }

    return huffman_heap_pop(&heap);
}

// A helper function for huffman_find_codes
static void _find_codes_helper(huffman_code *codes, huffman_node *node, huffman_code current) {
    if (node->left == NULL && node->right == NULL) {
        codes[node->byte].length = current.length;
        memcpy(codes[node->byte].data, current.data, 32);
        return;
    }

    current.length++;
    _find_codes_helper(codes, node->left, current);
    current.data[(current.length-1)/8] |= (1 << (256-current.length)%8);
    _find_codes_helper(codes, node->right, current);
}

/**
 * @brief Extracts the codes from a Huffman tree.
 * @param tree The tree to read codes from.
 * @return An array of 256 elements representing the code of each byte.
 *         The huffman_code.length leftmost bytes of huffman_code.data are used.
 *         Caller must free the memory.
 */
huffman_code *huffman_find_codes(huffman_node *tree) {
    huffman_code *codes = malloc(256*sizeof(huffman_code));
    huffman_code current = {};
    _find_codes_helper(codes, tree, current);
    return codes;
}

/**
 * @brief Serializes a huffman tree to a file. DFS order, bit 0 is written for
 *        internal nodes, bit 1 following 8-bit byte value for leaves.
 * @param writer Bitwriter handle for writing the output.
 * @param node Root node of a **full** binary tree.
 */
void huffman_write_tree(bitwriter *writer, huffman_node *node) {
    if (node->left == NULL) {
        bitwriter_write8(writer, 0b10000000, 1);
        bitwriter_write8(writer, node->byte, 8);
    } else {
        bitwriter_write8(writer, 0, 1);
        huffman_write_tree(writer, node->left);
        huffman_write_tree(writer, node->right);
    }
}

/**
 * @brief Reads and deserializes a Huffman tree from a file.
 * @param reader A bitreader handle for reading.
 * @return The root node of the tree. Caller must free all nodes of the tree.
 */
huffman_node *huffman_read_tree(bitreader *reader) {
    uint8_t is_leaf = bitreader_read8(reader, 1);
    if (is_leaf) {
        uint8_t byte = bitreader_read8(reader, 8);
        return huffman_node_create(byte, 0, NULL, NULL);
    } else {
        huffman_node *left = huffman_read_tree(reader);
        huffman_node *right = huffman_read_tree(reader);
        return huffman_node_create(0, 0, left, right);
    }
}

/**
 * @brief Encodes the input file using the given Huffman encoding.
 * @param writer Bitwriter handle for writing the output.
 * @param in_file The file to encode.
 * @param codes An array of 256 elements representing the code of each byte.
 */
void huffman_encode_data(bitwriter *writer, FILE *in_file, const huffman_code *codes) {
    bytereader reader = bytereader_init(in_file, 0);
    while (reader.bytes_left) {
        uint8_t byte = bytereader_read(&reader);
        bitwriter_write(writer, codes[byte].data, codes[byte].length);
    }
    bytereader_finish(&reader);
}

// A helper function for huffman_decode_data - reads next code
uint8_t _huffman_decode_code_helper(bitreader *reader, huffman_node *node) {
    if (node->left == NULL && node->right == NULL) {
        return node->byte;
    }

    if (bitreader_read8(reader, 1)) {
        return _huffman_decode_code_helper(reader, node->right);
    } else {
        return _huffman_decode_code_helper(reader, node->left);
    }
}

/**
 * @brief - Decodes the actual data using the given Huffman tree.
 * @param reader A bitreader handle for reading the encoded data.
 * @param out_file The file to write decoded output to.
 * @param byte_count The byte count of the original file.
 * @param tree The Huffman tree used for decoding.
 */
void huffman_decode_data(bitreader *reader, FILE *out_file, uint64_t byte_count, huffman_node *tree) {
    bytewriter writer = bytewriter_init(out_file, 0);
    while (byte_count--) {
        bytewriter_write(&writer, _huffman_decode_code_helper(reader, tree));
    }
    bytewriter_finish(&writer);
}

/**
 * @brief Compresseses a file using Huffman encoding
 * @param in_file Source file
 * @param out_file Destination file
 * @return returns original and compressed size
 */
compress_result huffman_compress(FILE *in_file, FILE *out_file) {
    uint64_t original_size;
    uint64_t *freqs = huffman_read_frequencies(in_file, &original_size);
    huffman_node *tree = huffman_create_tree(freqs);
    huffman_code *codes = huffman_find_codes(tree);

    fwrite(&original_size, sizeof(uint64_t), 1, out_file);
    
    bitwriter writer = bitwriter_init(out_file, sizeof(uint64_t));
    huffman_write_tree(&writer, tree);
    huffman_encode_data(&writer, in_file, codes);
    size_t compressed_size = bitwriter_finish(&writer);
    
    free(codes);
    huffman_destroy_tree(tree);
    free(freqs);

    return (compress_result) {
        .size_before = original_size,
        .size_after = compressed_size
    };
}

/**
 * @brief Decompresses a file using Huffman encoding
 * @param in_file Source file
 * @param out_file Destination file
 */
void huffman_decompress(FILE *in_file, FILE *out_file) {
    uint64_t original_size;
    fseek(in_file, 0, SEEK_SET);
    fread(&original_size, sizeof(uint64_t), 1, in_file);

    bitreader reader = bitreader_init(in_file, sizeof(uint64_t));
    huffman_node *tree = huffman_read_tree(&reader);
    huffman_decode_data(&reader, out_file, original_size, tree);
    bitreader_finish(&reader);
}