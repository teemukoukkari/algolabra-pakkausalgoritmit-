#include "huffman.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE* file = fopen(argv[1], "rb");

    huffman_freqs freqs = huffman_read_frequencies(file);
    huffman_node tree = huffman_create_tree(&freqs);
    huffman_codes codes = huffman_find_codes(tree);

    uint64_t original = 0;
    uint64_t new = 0;
    for (int i = 0; i < 256; i++) {
        original += freqs.arr[i];
        new += freqs.arr[i]*codes.length[i];
    }
    new /= 8;
    printf("RESULT: %ld --> %ld\n", original, new);

    fclose(file);
    return 0;
}