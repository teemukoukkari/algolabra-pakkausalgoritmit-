#include <string.h>
#include <stdio.h>
#include <time.h>

#include "huffman.h"
#include "lzw.h"
#include "util.h"

void print_size(const char* text, size_t size) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"}; 
    
    double amount = (double)size;
    int unit = 0;
    while (amount >= 1024 && unit < 4) {
        amount /= 1024;
        unit++;
    }

    printf("%s%ld bytes (%.2f %s)\n", text, size, amount, units[unit]);
}   

int main(int argc, char *argv[]) {
    const char* usage = "USAGE: algolabra [compress/decompress] [huffman/lzw] [source file] [destination file]\n";

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

    if (strcmp(argv[3], argv[4]) == 0) {
        printf("Source and destination files must be different.");
    }

    FILE *in_file = fopen(argv[3], "rb");
    if (!in_file) {
        printf("Failed to open source file '%s'.\n", argv[3]);
        return 1;
    }

    FILE *out_file = fopen(argv[4], "wb");
    if (!out_file) {
        printf("Failed to open/create destination file '%s'.\n", argv[4]);
        fclose(in_file);
        return 1;
    }

    clock_t start_time = clock();
    compress_result result = {};
    if (algorithm == 0) {
        if (operation == 0) {
            result = huffman_compress(in_file, out_file);     
        } else {
            huffman_decompress(in_file, out_file);
        }
    } else if (algorithm == 1) {
        if (operation == 0) {
            result = lzw_compress(in_file, out_file);
        } else {
            lzw_decompress(in_file, out_file);
        }
    }
    long execution_time = (clock() - start_time) / (CLOCKS_PER_SEC / 1000);

    if (operation == 0) {
        double ratio = (double)result.size_after / result.size_before;
        printf("Compressed file    '%s' as '%s'.\n", argv[3], argv[4]);
        print_size("Original size:     ", result.size_before);
        print_size("Compressed size:   ", result.size_after);
        printf("Ratio:             %.3f (compressed/original)\n", ratio);
        printf("Execution time:    %ld ms\n", execution_time);
    } else if (operation == 1) {
        printf("Uncompressed file '%s' as '%s'.\n", argv[3], argv[4]);
        printf("The execution took %ld milliseconds.\n", execution_time);
    }

    fclose(out_file);
    fclose(in_file);
    
    return 0;
}