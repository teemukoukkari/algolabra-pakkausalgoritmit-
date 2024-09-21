#include "huffman.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE* in = fopen(argv[1], "rb");
    FILE* out = fopen(argv[2], "wb");
    huffman_encode(in, out);
    fclose(out);
    fclose(in);
    return 0;
}