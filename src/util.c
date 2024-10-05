#include "util.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

/*****************************************************************************/

/**
 * @brief Creates an instance of bytewriter for writing whole bytes.
 * @param file Output file.
 * @param start_pos Byte number from which to start
 * @return A bytewriter instance. Caller must free using bytewriter_destroy.
 */
bytewriter bytewriter_init(FILE *file, size_t start_pos){
    bytewriter writer = {
        .file = file,
        .buffer = malloc(FILE_BUFFER_SIZE),
        .block_first = start_pos,
        .current_byte = 0
    };
    return writer;
}

/**
 * @brief Write a byte to the output file.
 * @param writer Bytewriter instance.
 * @param byte The byte to write.
 */
void bytewriter_write(bytewriter *w, uint8_t byte) {
    w->buffer[w->current_byte++] = byte;
    if (w->current_byte == FILE_BUFFER_SIZE) {
        fseek(w->file, w->block_first, SEEK_SET);
        fwrite(w->buffer, 1, FILE_BUFFER_SIZE, w->file);
        w->block_first += FILE_BUFFER_SIZE;
        w->current_byte = 0;
    }
}

/**
 * @brief Flushes the buffer and frees memory
 * @param writer Bytewriter instance.
 */
void bytewriter_finish(bytewriter *w) {
    fseek(w->file, w->block_first, SEEK_SET);
    fwrite(w->buffer, 1, w->current_byte, w->file);
    free(w->buffer);
}

/*****************************************************************************/

/**
 * @brief Creates an instance of bytereader for reading whole bytes.
 * @param file Input file.
 * @param start_pos Byte number from which to start.
 * @return A bytereader instance. Caller must free using bytereader_destroy.
 */
bytereader bytereader_init(FILE *file, size_t start_pos) {
    bytereader reader = {
        .file = file,
        .buffer = malloc(FILE_BUFFER_SIZE),
        .block_first = start_pos,
        .current_byte = 0,
        .bytes_left = 0
    };

    fseek(file, start_pos, SEEK_SET);
    reader.bytes_left = fread(reader.buffer, 1, FILE_BUFFER_SIZE, file);
    
    return reader;
}

/**
 * @brief Read a byte from the input file.
 * @param writer Byterader instance.
 * @return The byte read.
 */
uint8_t bytereader_read(bytereader *r) {
    uint8_t byte = r->buffer[r->current_byte++];
    r->bytes_left--;

    if (!r->bytes_left) {
        r->block_first += FILE_BUFFER_SIZE;
        fseek(r->file, r->block_first, SEEK_SET);
        r->bytes_left = fread(r->buffer, 1, FILE_BUFFER_SIZE, r->file);
        r->current_byte = 0;
    }

    return byte;
}

void bytereader_finish(bytereader *r) {
    free(r->buffer);
}

/*****************************************************************************/

/**
 * @brief Creates an instance of bitwrititer for writing bit-level data.
 * @param file Output file.
 * @param start_pos Byte number from which to start
 * @return A bitwriter instance. Caller must free using bitwriter_destroy.
 */
bitwriter bitwriter_init(FILE *file, size_t start_pos) {
    bitwriter writer = {
        .file = file,
        .buffer = calloc(FILE_BUFFER_SIZE+1, 1),
        .block_first = start_pos,
        .current_byte = 0,
        .current_bit = 0
    };
    return writer;
}

/**
 * @brief Writes a maximum of 8 bits to the output file.
 * @param data The bits to write. Leftmost (most significant) bits of the integer
 *             are written from left to right.
 * @param count The number of bits to write, 1-8.
 */
void bitwriter_write8(bitwriter *w, uint8_t data, uint8_t count) {
    w->buffer[w->current_byte] |= data >> w->current_bit; // first byte
    if (w->current_bit + count > 8) { // some bits need to be written to another
        w->buffer[w->current_byte+1] = data << (8-w->current_bit);
    }

    w->current_bit += count;
    if (w->current_bit >= 8) {
        w->current_byte++;
        w->current_bit -= 8;
        if (w->current_byte == FILE_BUFFER_SIZE) {
            fseek(w->file, w->block_first, SEEK_SET);
            fwrite(w->buffer, 1, FILE_BUFFER_SIZE, w->file);
            w->block_first += FILE_BUFFER_SIZE;
            w->current_byte = 0;

            w->buffer[0] = w->buffer[FILE_BUFFER_SIZE]; // "overflow byte"
            memset(w->buffer+1, 0, FILE_BUFFER_SIZE);
        }
    }
}

/**
 * @brief Writes a maximum of 256 bits to the output file.
 * @param data The bits to write. First integers of the array, and the most
 *             significant bits of the integers are written first.
 * @param count The number of bits to write, 1-256.
 */
void bitwriter_write(bitwriter *w, const uint8_t *data, uint8_t count) {
    uint8_t current_byte = 0;

    while (count > 0) {
        if (count >= 8) {
            bitwriter_write8(w, data[current_byte++], 8);
            count -= 8;
        } else {
            bitwriter_write8(w, data[current_byte], count);
            count = 0;
        }
    }
}

/**
 * @brief Writes 9-16 bits to the output file.
 * @param data The bits to write. The rightmost (least significant) bits of the
 *             integer are written from left to right.
 * @param count The number of bits to write, 9-16.
 */
void bitwriter_write16r(bitwriter *w, uint16_t data, uint8_t count) {
    bitwriter_write8(w, data >> (count-8), 8);
    bitwriter_write8(w, data << (16-count), count-8);
}

/**
 * @brief Flushes the buffer and frees memory
 * @param writer Bytewriter instance.
 */
size_t bitwriter_finish(bitwriter *w) {
    fseek(w->file, w->block_first, SEEK_SET);
    fwrite(w->buffer, 1, w->current_byte + (w->current_bit ? 1 : 0), w->file);
    free(w->buffer);
    return w->block_first + w->current_byte + (w->current_bit ? 1 : 0);
}

/*****************************************************************************/

/**
 * @brief Creates an instance of bitreader for reading bit-level data.
 * @param file Input file.
 * @param start_pos Byte number from which to start
 * @return A bitreader instance. Caller must free using bitreader_destroy.
 */
bitreader bitreader_init(FILE *file, size_t start_pos) {
    bitreader reader = {
        .file = file,
        .buffer = calloc(FILE_BUFFER_SIZE+1, 1),
        .block_first = start_pos,
        .current_byte = 0,
        .current_bit = 0
    };

    fseek(file, start_pos, SEEK_SET);
    fread(reader.buffer, 1, FILE_BUFFER_SIZE+1, file);

    return reader;
}

/**
 * @brief Reads a maximum of 8 bits from the input file.
 * @param count The number of bits to read.
 * @return The bits read. The bits are stored from left to right in the leftmost
 *         (most significant) bits of the integer.
 */
uint8_t bitreader_read8(bitreader *r, uint8_t count) {
    uint8_t result = r->buffer[r->current_byte] << r->current_bit; // first byte
    if (r->current_bit + count > 8) { // some bits need to be read from another
        result |= r->buffer[r->current_byte+1] >> (8-r->current_bit);
    }
    result &= (0xFF << (8-count)); // Make sure that the excess bits are zeros.

    r->current_bit += count;
    if (r->current_bit >= 8) {
        r->current_byte++;
        r->current_bit-=8;
        if (r->current_byte == FILE_BUFFER_SIZE) {
            r->block_first += FILE_BUFFER_SIZE;
            fseek(r->file, r->block_first, SEEK_SET);
            fread(r->buffer, 1, FILE_BUFFER_SIZE+1, r->file);
            r->current_byte = 0;
        }
    }

    return result;
}


/**
 * @brief Reads 9-16 bits from the input file.
 * @param count The number of bits to read, 9-16.
 * @return The bits read. The rightmost (least significant) bits of the integer
 *         are used, from left to right.
 */
uint16_t bitreader_read16r(bitreader *r, uint8_t count) {
    uint16_t res = bitreader_read8(r, 8) << (count-8);
    res |= bitreader_read8(r, count-8) >> (16-count);
    return res;
}

void bitreader_finish(bitreader* r) {
    free(r->buffer);
}