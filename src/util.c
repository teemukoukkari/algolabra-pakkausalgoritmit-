#include "util.h"

#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <stdlib.h>

bitwriter bitwriter_init(FILE *file, size_t start_pos) {
    bitwriter writer = {
        .file = file,
        .buffer = calloc(BITWRITER_BLOCK_SIZE+1, 1),
        .block_first = start_pos,
        .current_byte = 0,
        .current_bit = 0
    };
    return writer;
}

void bitwriter_write8(bitwriter *w, uint8_t bits, uint8_t count) {
    w->buffer[w->current_byte] |= bits >> w->current_bit;
    if (w->current_bit + count > 8) {
        w->buffer[w->current_byte+1] = bits << (8-w->current_bit);
    }

    w->current_bit += count;
    if (w->current_bit >= 8) {
        w->current_byte++;
        w->current_bit -= 8;

        if (w->current_byte == BITWRITER_BLOCK_SIZE) {
            fseek(w->file, w->block_first, SEEK_SET);
            fwrite(w->buffer, 1, BITWRITER_BLOCK_SIZE, w->file);
            w->block_first += BITWRITER_BLOCK_SIZE;
            w->current_byte = 0;

            w->buffer[0] = w->buffer[BITWRITER_BLOCK_SIZE];
            memset(w->buffer+1, 0, BITWRITER_BLOCK_SIZE);
        }
    }
}

void bitwriter_write32(bitwriter *w, uint32_t bits, uint8_t count) {
    if (count <= 8) {
        bitwriter_write8(w, (bits & 0xFF000000) >> 24, count);
    }
    else if (count <= 16) {
        bitwriter_write8(w, (bits & 0xFF000000) >> 24, 8);
        bitwriter_write8(w, (bits & 0x00FF0000) >> 16, count-8);
    }
    else if (count <= 24) {
        bitwriter_write8(w, (bits & 0xFF000000) >> 24, 8);
        bitwriter_write8(w, (bits & 0x00FF0000) >> 16, 8);
        bitwriter_write8(w, (bits & 0x0000FF00) >> 8, count-16);
    }
    else {
        bitwriter_write8(w, (bits & 0xFF000000) >> 24, 8);
        bitwriter_write8(w, (bits & 0x00FF0000) >> 16, 8);
        bitwriter_write8(w, (bits & 0x0000FF00) >> 8, 8);
        bitwriter_write8(w, (bits & 0x0000FF00) >> 0, count-24);
    }
}

void bitwriter_finish(bitwriter *w) {
    fseek(w->file, w->block_first, SEEK_SET);
    fwrite(w->buffer, 1, w->current_byte + (w->current_bit ? 1 : 0), w->file);
    free(w->buffer);
}

bitreader bitreader_init(FILE *file, size_t start_pos) {
    bitreader reader = {
        .file = file,
        .buffer = calloc(BITWRITER_BLOCK_SIZE+1, 1),
        .block_first = start_pos,
        .current_byte = 0,
        .current_bit = 0
    };

    fseek(file, start_pos, SEEK_SET);
    fread(reader.buffer, 1, BITWRITER_BLOCK_SIZE+1, file);

    return reader;
}

uint8_t bitreader_read8(bitreader *r, uint8_t count) {
    uint8_t result = r->buffer[r->current_byte] << r->current_bit;

    if (r->current_bit + count > 8) {
        result |= r->buffer[r->current_byte+1] >> (8-r->current_bit);
    }
    result &= (0xFF << (8-count));

    r->current_bit += count;
    if (r->current_bit >= 8) {
        r->current_byte++;
        r->current_bit-=8;

        if (r->current_byte == BITWRITER_BLOCK_SIZE) {
            r->block_first += BITWRITER_BLOCK_SIZE;
            fseek(r->file, r->block_first, SEEK_SET);
            fread(r->buffer, 1, BITWRITER_BLOCK_SIZE+1, r->file);
            r->current_byte = 0;
        }
    }

    return result;
}

uint32_t bitreader_read32(bitreader *r, uint8_t count) {
    uint32_t result = 0;
    if (count <= 8) {
        result |= bitreader_read8(r, count) << 24;
    }
    else if (count <= 16) {
        result |= bitreader_read8(r, 8) << 24;
        result |= bitreader_read8(r, count-8) << 16;
    }
    else if (count <= 24) {
        result |= bitreader_read8(r, 8) << 24;
        result |= bitreader_read8(r, 8) << 16;
        result |= bitreader_read8(r, count-16) << 8;
    }
    else if (count <= 32) {
        result |= bitreader_read8(r, 8) << 24;
        result |= bitreader_read8(r, 8) << 16;
        result |= bitreader_read8(r, 8) << 8;
        result |= bitreader_read8(r, count-24) << 0;
    }
    return result;
}

void bitreader_finish(bitreader* r) {
    free(r->buffer);
}