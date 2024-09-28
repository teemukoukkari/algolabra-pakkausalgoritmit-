#include "util.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

/*****************************************************************************/

// Kokonaisten tavujen kirjoittamiseen tarkoitettu puskuroiva apuväline.
bytewriter bytewriter_init(FILE *file, size_t start_pos){
    bytewriter writer = {
        .file = file,
        .buffer = malloc(FILE_BUFFER_SIZE),
        .block_first = start_pos,
        .current_byte = 0
    };
    return writer;
}

// Kirjoittaa tavun tiedostoon ja tarvittaessa tallentaa ja tyhjentää puskurin.
void bytewriter_write(bytewriter *w, uint8_t byte) {
    w->buffer[w->current_byte++] = byte;
    if (w->current_byte == FILE_BUFFER_SIZE) {
        fseek(w->file, w->block_first, SEEK_SET);
        fwrite(w->buffer, 1, FILE_BUFFER_SIZE, w->file);
        w->block_first += FILE_BUFFER_SIZE;
        w->current_byte = 0;
    }
}

// Tallentaa ja vapauttaa puskurin.
void bytewriter_finish(bytewriter *w) {
    fseek(w->file, w->block_first, SEEK_SET);
    fwrite(w->buffer, 1, w->current_byte, w->file);
    free(w->buffer);
}

/*****************************************************************************/

// Kokonaisten tavujen lukuemiseen tarkoitettu puskuroiva apuväline.
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

// Palauttaa tiedoston seuraavan tavun ja tarvittaesa päivittää puskurin.
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

// Ei-tavumuotoisen tiedon kirjoittamiseen tarkoitettu puskuroiva apuväline.
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

// Kirjoittaa enintään tavun verran bittejä tiedostoon.
// Parametri data sisältää kirjoittettavat bitit siten, että tavusta count kpl
//  merkitsevintä bittiä kirjoitetaan, merkitsevin ensimmäisenä.
void bitwriter_write8(bitwriter *w, uint8_t data, uint8_t count) {
    // Aluksi tallennetaan käsiteltävään tavuun niin monta bittiä, kuin mahtuu.
    // Mikäli kaikki eivät mahdu, kirjoitetaan loput seuraavan puolelle.
    w->buffer[w->current_byte] |= data >> w->current_bit;
    if (w->current_bit + count > 8) {
        w->buffer[w->current_byte+1] = data << (8-w->current_bit);
    }

    w->current_bit += count;
    if (w->current_bit >= 8) {
        w->current_byte++;
        w->current_bit -= 8;

        // Mikäli puskuri täyttyy, se tallennetaan ja tyhjennetään.
        // Viimeisenä on yksi ylimääräinen tavu sitä varten, jos kirjoittaminen
        //  menisi seuraavan tavun puolelle puskurin ulkopuolelle.
        // Keskeneräinen tavu siirretään ensimmäiseksi tyhjennyksen jälkeen.
        if (w->current_byte == FILE_BUFFER_SIZE) {
            fseek(w->file, w->block_first, SEEK_SET);
            fwrite(w->buffer, 1, FILE_BUFFER_SIZE, w->file);
            w->block_first += FILE_BUFFER_SIZE;
            w->current_byte = 0;

            w->buffer[0] = w->buffer[FILE_BUFFER_SIZE];
            memset(w->buffer+1, 0, FILE_BUFFER_SIZE);
        }
    }
}

// Kirjoittaa count-muuttujan verran bittejä tiedostoon, enintään siis 256 kpl.
// Bitit luetaan data-taulukosta siten, että vasemmaisin/merkitsevin 
//  kirjoitetaan ensimmäisenä. Ylimääräisten bittien tulee olla nollia.
void bitwriter_write(bitwriter *w, uint8_t *data, uint8_t count) {
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

// Kirjoittaa enintään 16 bittiä tiedostoon.
// Kirjoitettavat bitit ovat data-parametrissa siten,
//  vähiten merkitsevät count kpl kirjoitetaan, näistä merkitsevin ekana
// Käyttämättömien bittien tulee olla nollia.
void bitwriter_write16r(bitwriter *w, uint16_t data, uint8_t count) {
    if (count >= 8) {
        bitwriter_write8(w, data >> (count-8), 8);
        bitwriter_write8(w, data << (16-count), count-8);
    } else {
        printf("ERROR bitwriter_write16r");
        exit(1);
    }
}

// Tyhjentää ja vapauttaa puskurin. Jos tavu jäi kesken, täytetään se nollilla.
void bitwriter_finish(bitwriter *w) {
    fseek(w->file, w->block_first, SEEK_SET);
    fwrite(w->buffer, 1, w->current_byte + (w->current_bit ? 1 : 0), w->file);
    free(w->buffer);
}

/*****************************************************************************/

// Ei-tavumuotoisen tiedon lukemiseen tarkoitettu puskuroiva apuväline.
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

// Lukee enintään tavun verran bittejä tiedostosta, yhteensä count kpl.
// Pyydetyt bitit ovat palautusarvon merkitsevimmät, loput jätetään nolliksi.
uint8_t bitreader_read8(bitreader *r, uint8_t count) {
    // Luetaan nykyisen tavun puolella olevat bitit valmiiksi oikeaan kohtaan.
    uint8_t result = r->buffer[r->current_byte] << r->current_bit;
    if (r->current_bit + count > 8) {
        // Mikäli tässä ei ollut kaikki, luetaan loput seuraavasta tavusta
        result |= r->buffer[r->current_byte+1] >> (8-r->current_bit);
    }
    // Varmistetaan vielä, että palautettavat ylimääräiset bitit ovat nollia.
    result &= (0xFF << (8-count));

    r->current_bit += count;
    if (r->current_bit >= 8) {
        r->current_byte++;
        r->current_bit-=8;

        // Puskurissa pidetään aina myös yhtä ylimääräistä tavua, sillä luku
        //  saattaa mennä tavurajan yli.
        if (r->current_byte == FILE_BUFFER_SIZE) {
            r->block_first += FILE_BUFFER_SIZE;
            fseek(r->file, r->block_first, SEEK_SET);
            fread(r->buffer, 1, FILE_BUFFER_SIZE+1, r->file);
            r->current_byte = 0;
        }
    }

    return result;
}

uint16_t bitreader_read16r(bitreader *r, uint8_t count) {
    if (count >= 8) {
        uint16_t res = bitreader_read8(r, 8) << (count-8);
        res |= bitreader_read8(r, count-8) >> (16-count);
        return res;
    } else {
        printf("ERROR bitreader_read16r");
        exit(1);
    }
}

void bitreader_finish(bitreader* r) {
    free(r->buffer);
}