#include "lzw.h"

#include <stdlib.h>
#include <stdbool.h>
#include "util.h"

/******************************************************************************
* LZW-pakkausalgoritmia varten tarvitaan hajautustaulu, jonka avaimet ovat
* merkkijonoja ja arvot lisäysjärjestyksessä kasvavia kokonaislukuja (koodeja).
*
* Algoritmin luonteesta johtuen jokainen avainmerkkijono koostuu aina jostakin
* aiemmin lisätystä merkkijonosta ja yhdestä uudesta merkistä. Siten avaimina
* voidaan käyttää (prefix, token)-pareja, missä prefix on aiemmin lisättyä
* merkkijonoa vastaava (uniikki) koodi ja token perään lisättävä merkki.  
*
* Tämän lisäksi hajautustaulu tarjoaa tiedon siitä, kuinka monta bittiä
* suurimman tallennetun koodin esittämiseen tarvitaan (code_length) sekä
* varmistaa, ettei liian suuria arvoja lisätä (LZW_CODEWORD_LENGTH_LIMIT) 
*/

// Alustaa hajautustaulun LZW-algoritmia varten. Tauluun lisätään valmiiksi
// avaimille (LZW_NULL,0-255) koodit 0-255. Lisäksi koodi LZW_EOF=256 varataan
lzw_dict lzw_dict_init() {
    lzw_dict dict = {
        .size = 0,
        .code_length = 9,
        .arr = malloc(LZW_DICT_SIZE*sizeof(lzw_dict_node))
    };

    for (size_t i = 0; i < LZW_DICT_SIZE; i++) {
        dict.arr[i].value = LZW_NULL;
    }

    for (int i = 0; i <= 255; i++) {
        lzw_dict_insert(&dict, LZW_NULL, i, LZW_NULL);
    }
    dict.size += 1; // LZW_EOF=256

    return dict;
}

void lzw_dict_destroy(lzw_dict *dict) {
    free(dict->arr);
}

// Hajautusfunktio. Osoittautui empiirisellä testauksella erittäin tehokkaaksi
// juuri tässä käyttötarkoituksessa. djb2-funktion taikaluku 33 toimii yhä!
uint32_t lzw_dict_hash(uint16_t prefix, uint8_t token) {
    return 33 * (prefix*256 + token);
}

// Hakee arvon hajautustaulusta käyttäen lineaarista kokeilua, mikäli hajautus-
// arvon kohdalla on sattunut yhteentörmäys. Jos avainta ei löydy taulusta,
// palautetaan LZW_NULL_INDEX ja kohta, johon tämä voidaan lisätä (free_pos).
uint16_t lzw_dict_get(
    lzw_dict *dict, uint16_t prefix, uint8_t token, uint32_t *free_pos
) {
    uint32_t pos = lzw_dict_hash(prefix, token) % LZW_DICT_SIZE;

    while (true) {
        if (dict->arr[pos].prefix == prefix && dict->arr[pos].token == token) {
            return dict->arr[pos].value;
        } else if (dict->arr[pos].value == LZW_NULL) {
            *free_pos = pos;
            return LZW_NULL;
        }
        pos = (pos + 1) % LZW_DICT_SIZE;
    }
}

// Lisää tauluun järjestyksessä seuraavan koodin annetulle avaimelle. Mikäli
// koodien enimmäismäärä on saavutettu, ei tehdä mitään. Viimeinen parametri
// voi olla hakufunktion palauttama free_pos tai LZW_NULL_INDEX. 
void lzw_dict_insert(
    lzw_dict *dict, uint16_t prefix, uint8_t token, uint32_t free_pos
) {
    if (dict->size >= LZW_CODEWORD_COUNT) {
        return;
    }

    if (free_pos == LZW_NULL) {
        lzw_dict_get(dict, prefix, token, &free_pos);
    }

    dict->arr[free_pos].value = dict->size++;
    dict->arr[free_pos].prefix = prefix;
    dict->arr[free_pos].token = token;

    if (dict->size >= (1 << dict->code_length)) {
        dict->code_length++;
    }
}

/******************************************************************************
* LZW-purkualgoritmia varten tarvitaan merkkijonotaulukko. Algoritmin
* luonteesta johtuen taulukko täytetään järjestyksessä siten, että jokainen uusi
* arvo koostuu jostain aiemmin lisätystä jonosta ja yhdestä uudesta merkistä.
* Siten kokonaisten merkkijonojen sijaan riittää tallentaa tieto, mitä aiempaa
* merkkijonoa (prefix) käytetään alkuosana ja mikä merkki (token) lisätään.
* Tämäkin rakenne pitää kirjaa siitä, miten monta bittiä tarvitaan koodien
* esittämiseen (code_length) ja varmistaa, että enimmäismäärää ylitetä.
*/

// Alustaa taulukon siten, että koodeja 0-255 vastaavat yhden merkin jonot
// (LZW_NULL, 0-255). Lisäksi paikka 256 varateen LZW_EOF-käyttöä varten
lzw_table lzw_table_init() {
    lzw_table table = {
        .size = 257,
        .code_length = 9,
        .arr = calloc(1, sizeof(lzw_table_node)*LZW_CODEWORD_COUNT)
    };

    for (int i = 0; i <= 256; i++) {
        table.arr[i].prefix = LZW_NULL;
        table.arr[i].token = i;
    }

    return table;
}

// Tulostaa tallennetun merkkijonon rekursiivisesti.
// Palauttaa jonon ensimmäisen '¨merkin myöhempää käyttöä varten.
uint8_t lzw_table_print(lzw_table *table, bytewriter *writer, uint16_t entry) {
    uint8_t res = entry;
    if (entry > 255) {
        res = lzw_table_print(table, writer, table->arr[entry].prefix);
    }
    bytewriter_write(writer, table->arr[entry].token);
    return res;
}

// Lisää annetun merkkijonon seuraavan koodin arvoksi, mikäli tilaa on.
void lzw_table_insert(lzw_table *table, uint16_t prefix, uint8_t token) {
    if (table->size >= LZW_CODEWORD_COUNT) {
        return;
    }

    table->arr[table->size].prefix = prefix;
    table->arr[table->size].token = token;
    table->size++;

    if (table->size+1 >= (1 << table->code_length)) {
        table->code_length++;
    }
}

void lzw_table_destroy(lzw_table *table) {
    free(table->arr);
}

/******************************************************************************
* Pakkaus- ja purkualgormit perustuvat pääosin sivulla
*   https://www.geeksforgeeks.org/lzw-lempel-ziv-welch-compression-technique/
* kuvattuun toimintalogiikkaan ja pseudokoodiin.
* 
* Suurin poikkeama lienee koodin pituuden mukauttaminen, sillä alkuun on niin
* vähän koodeja, että 9-bittiset esitykset riittävät. Pituutta kasvatetaan
* aina, kun on mahdollisisuus, että seuraava koodi vaati pidemmän esityksen.
* Pisin käytettävä koodi on 12 bittiä, tämän jälkeen uusia ei enää lisätä.
*/

void lzw_encode(FILE *in_file, FILE *out_file) {
    bytereader reader = bytereader_init(in_file, 0);
    bitwriter writer = bitwriter_init(out_file, 0);

    if (!reader.bytes_left) {
        bitwriter_finish(&writer);
        bytereader_finish(&reader);
        return;
    }

    lzw_dict dict = lzw_dict_init();
    uint16_t last_code = bytereader_read(&reader);

    while (reader.bytes_left) {
        uint8_t token = bytereader_read(&reader);

        uint32_t free_pos = 0;
        uint16_t code = lzw_dict_get(&dict, last_code, token, &free_pos);
        if (code == LZW_NULL) {
            bitwriter_write16r(&writer, last_code, dict.code_length);
            lzw_dict_insert(&dict, last_code, token, free_pos);
            last_code = token;
        } else {
            last_code = code;
        }
    }

    bitwriter_write16r(&writer, last_code, dict.code_length);
    bitwriter_write16r(&writer, LZW_EOF, dict.code_length);
    
    lzw_dict_destroy(&dict);
    bitwriter_finish(&writer);
    bytereader_finish(&reader);
}

void lzw_decode(FILE *in_file, FILE *out_file) {
    bitreader reader = bitreader_init(in_file, 0);
    bytewriter writer = bytewriter_init(out_file, 0);
    
    lzw_table table = lzw_table_init();
    
    uint16_t last_code = bitreader_read16r(&reader, table.code_length);
    if (last_code == LZW_NULL) {
        bytewriter_finish(&writer);
        bitreader_finish(&reader);
        return;
    }
    uint8_t last_first_char = lzw_table_print(&table, &writer, last_code);

    while (true) {
        uint16_t index = bitreader_read16r(&reader, table.code_length);
        if (index == LZW_EOF) {
            break;
        }

        uint8_t first_char;
        if (table.size <= index) {
            first_char = lzw_table_print(&table, &writer, last_code);
            lzw_table_print(&table, &writer, last_first_char);
        } else {
            first_char = lzw_table_print(&table, &writer, index);
        }

        lzw_table_insert(&table, last_code, first_char);

        last_code = index;
        last_first_char = first_char;
    }

    bytewriter_finish(&writer);
    bitreader_finish(&reader);
}