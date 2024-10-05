#pragma once

#include <stdio.h>
#include <stdint.h>

#define FILE_BUFFER_SIZE 16384

/*****************************************************************************/
typedef struct compress_result {
    size_t size_before;
    size_t size_after;
} compress_result;

/*****************************************************************************/

typedef struct bytewriter {
    FILE *file;
    uint8_t *buffer;
    size_t block_first;
    size_t current_byte;
} bytewriter;

bytewriter bytewriter_init(FILE *file, size_t start_pos);
void bytewriter_write(bytewriter *writer, uint8_t byte);
void bytewriter_finish(bytewriter *writer);

/*****************************************************************************/

typedef struct bytereader {
    FILE *file;
    uint8_t *buffer;
    size_t block_first;
    size_t current_byte;
    size_t bytes_left;
} bytereader;

bytereader bytereader_init(FILE *file, size_t start_pos);
uint8_t bytereader_read(bytereader *reader);
void bytereader_finish(bytereader *reader);


/*****************************************************************************/

typedef struct bitwriter {
    FILE *file;
    uint8_t *buffer;
    size_t block_first;
    size_t current_byte;
    uint8_t current_bit;
} bitwriter;

bitwriter bitwriter_init(FILE *file, size_t start_pos);
void bitwriter_write8(bitwriter *writer, uint8_t data, uint8_t count);
void bitwriter_write(bitwriter *writer, const uint8_t *data, uint8_t count);
void bitwriter_write16r(bitwriter *writer, uint16_t data, uint8_t count);
size_t bitwriter_finish(bitwriter *writer);

/*****************************************************************************/

typedef struct bitreader {
    FILE *file;
    uint8_t *buffer;
    size_t block_first;
    size_t current_byte;
    uint8_t current_bit;
} bitreader;


bitreader bitreader_init(FILE *file, size_t start_pos);
uint8_t bitreader_read8(bitreader *reader, uint8_t count);
uint16_t bitreader_read16r(bitreader *reader, uint8_t count);
void bitreader_finish(bitreader* reader);