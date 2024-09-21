#pragma once

#include <stdio.h>
#include <stdint.h>

#define BITWRITER_BLOCK_SIZE 16384

typedef struct bitwriter {
    FILE *file;
    uint8_t *buffer;
    size_t block_first;
    size_t current_byte;
    uint8_t current_bit;
} bitwriter;

typedef struct bitreader {
    FILE *file;
    uint8_t *buffer;
    size_t block_first;
    size_t current_byte;
    uint8_t current_bit;
} bitreader;

bitwriter bitwriter_init(FILE *file, size_t start_pos);
void bitwriter_write8(bitwriter *writer, uint8_t bits, uint8_t count);
void bitwriter_write32(bitwriter *writer, uint32_t bits, uint8_t count);
void bitwriter_finish(bitwriter *writer);

bitreader bitreader_init(FILE *file, size_t start_pos);
uint8_t bitreader_read8(bitreader *reader, uint8_t count);
uint32_t bitreader_read32(bitreader *reader, uint8_t count);
void bitreader_finish(bitreader *reader);