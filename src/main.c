#include <string.h>
#include <stdio.h>

#include "huffman.h"
#include "lzw.h"

int main(int argc, char *argv[]) {
    const char* usage = "USAGE: algolabra [compress/decompress] [huffman/lzw] [input file] [output file]\n";

    if (argc != 5) {
        printf("Invalid numer of arguments.\n%s", usage);
        return 1;
    }
    
    int operation;
    if (strcmp(argv[1], "compress") == 0) {
        operation = 0;
    } else if (strcmp(argv[1], "decompress") == 0) {
        operation = 1;
    } else {
        printf("Invalid operation: %s\n%s", argv[1], usage);
        return 1;
    }
    
    int algorithm;
    if (strcmp(argv[2], "huffman") == 0) {
        algorithm = 0;
    } else if (strcmp(argv[2], "lzw") == 0) {
        algorithm = 1;
    } else {
        printf("Invalid algorithm: %s\n%s", argv[2], usage);
        return 1;
    }

    FILE *in_file = fopen(argv[3], "rb");
    if (!in_file) {
        printf("Failed to open input file %s.", argv[3]);
        return 1;
    }

    FILE *out_file = fopen(argv[4], "wb");
    if (!out_file) {
        printf("Failed to open/create output file %s", argv[4]);
        fclose(in_file);
        return 1;
    }

    if (algorithm == 0) {
        if (operation == 0) {
            huffman_encode(in_file, out_file);
        } else {
            huffman_decode(in_file, out_file);
        }
    } else if (algorithm == 1) {
        if (operation == 0) {
            lzw_encode(in_file, out_file);
        } else {
            lzw_decode(in_file, out_file);
        }
    }

    fclose(out_file);
    fclose(in_file);
    
    return 0;
}