#include "huffman.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "util.h"

/*****************************************************************************/

// Luo uuden solmun annetuista tiedoista
huffman_node *huffman_node_create(
    uint8_t byte, uint64_t freq, huffman_node *left, huffman_node *right
) {
    huffman_node *node = malloc(sizeof(huffman_node));
    node->byte = byte;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

// Vapauttaa solmun ja sen lapsisolmut rekursiivisesti
void huffman_tree_destroy(huffman_node *tree) {
    if (tree == NULL) return;
    huffman_tree_destroy(tree->left);
    huffman_tree_destroy(tree->right);
    free(tree);
}

// Lisää solmun minimikekoon (freq-arvon suhteen). Aluksi solmu lisätään
// ensimmäiselle vapaalle  paikalle, jonka jälkeen sitä nostetaan, kunnes
// kekokoehto täyttyy. Taulukon indeksit alkavat ykkösestä.
void huffman_heap_insert(huffman_heap *heap, huffman_node *node) {
    int k = heap->size+1;
    heap->arr[k] = node;
    while (k > 1 && heap->arr[k/2]->freq > heap->arr[k]->freq) {
        huffman_node *temp = heap->arr[k/2];
        heap->arr[k/2] = heap->arr[k];
        heap->arr[k] = temp;
        k /= 2;
    }
    heap->size += 1;
}

// Palauttaa minimikeon pienimmän solmun ja poistaa sen keosta. Viimeinen
// solmu siirretään juurisolmuksi, ja sitä lasketaan, kunnes kekoehto täyttyy.
huffman_node *huffman_heap_pop(huffman_heap *heap) {
    huffman_node *node = heap->arr[1];

    heap->arr[1] = heap->arr[heap->size];
    heap->arr[heap->size] = NULL;
    heap->size -= 1;

    int k = 1;
    while (k < 258/2) {
        int min = k;
        if (heap->arr[2*k] && heap->arr[2*k]->freq < heap->arr[min]->freq) {
            min = 2*k;
        }
        if (heap->arr[2*k+1] && heap->arr[2*k+1]->freq < heap->arr[min]->freq) {
            min = 2*k+1;
        }

        if (min == k) break;

        huffman_node *temp = heap->arr[min];
        heap->arr[min] = heap->arr[k];
        heap->arr[k] = temp;
        k = min;
    }

    return node;
}

/*****************************************************************************/

// Lukee tiedostosta tavujen esiintymistiheydet.
// Palauttaa 256-kohtaisen taulukon ja total-parametriin tiedostonkoon
uint64_t *huffman_read_frequencies(FILE* file, uint64_t *total) {
    *total = 0;
    uint64_t *freqs = calloc(256*sizeof(uint64_t), 1);

    bytereader reader = bytereader_init(file, 0);
    while (reader.bytes_left) {
        freqs[bytereader_read(&reader)]++;
        *total = *total + 1;
    }
    bytereader_finish(&reader);

    return freqs;
}

// Luo tavujen esiintymistiheyksien perusteella Huffmanin puun.
// Algoritmin tomintaperiaate on seuraavalta sivulta:
//   https://www.w3schools.com/dsa/dsa_ref_huffman_coding.php
huffman_node *huffman_create_tree(uint64_t *freqs) {
    huffman_heap heap = {};
    for (int i = 0; i < 256; i++) {
        huffman_node *node = huffman_node_create(i, freqs[i], NULL, NULL);
        huffman_heap_insert(&heap, node);
    }

    huffman_node *left, *right, *new;
    while (heap.size > 1) {
        left = huffman_heap_pop(&heap);
        right = huffman_heap_pop(&heap);
        new = huffman_node_create(0, left->freq+right->freq, left, right);
        huffman_heap_insert(&heap, new);
    }

    return huffman_heap_pop(&heap);
}

// Kasaa kooditaulukon etenemällä puussa rekursiivisesti. Viimeinen parametri
// sisältää ylempänä muodostetun koodin. Kun saavutaan lapsisolmuun, koodi on 
// valmis ja tallennetaan taulukkoon omalle paikalleen.
static void _find_codes_helper(
    huffman_code *codes, huffman_node *node, huffman_code current
) {
    if (node->left == NULL && node->right == NULL) {
        codes[node->byte].length = current.length;
        memcpy(codes[node->byte].data, current.data, 32);
        return;
    }

    current.length++;
    _find_codes_helper(codes, node->left, current);
    current.data[(current.length-1)/8] |= (1 << (256-current.length)%8);
    _find_codes_helper(codes, node->right, current);
}

// Palauttaa 256-paikkaisen taulukon, jossa on tavuja vastaavat koodaukset.
// Yksittäisestä koodista on tallennettuna sen pituus bitteinä sekä
// uint8_t[32]-tyyppinen taulukko, jossa bitit ovat järjestyksessä vasemmalta 
// oikealle (merkitsevin vasemmalla). Käyttämättömät bitit jäävät nolliksi.
huffman_code *huffman_find_codes(huffman_node *tree) {
    huffman_code *codes = malloc(256*sizeof(huffman_code));
    huffman_code current = {};
    _find_codes_helper(codes, tree, current);
    return codes;
}

// Kirjoittaa puun tiedostoon. Solmut tallennetaan syvyyshaun mukaisessa
// järjestyksessä. Mikäli solmu on lehti, lisätään tiedostoon bitti 1 ja sen
// perään solmua vastaava tavu (8 bittiä). Mikäli solmu on välisolmu,
// kirjoitetaan vain bitti 0. Huffman-puu on täydellinen, eli jokainen
// solmu on joko lehti tai sillä on kaksi lapsisolmua.
void huffman_write_tree(bitwriter *writer, huffman_node *node) {
    if (node->left == NULL) {
        bitwriter_write8(writer, 0b10000000, 1);
        bitwriter_write8(writer, node->byte, 8);
    } else {
        bitwriter_write8(writer, 0, 1);
        huffman_write_tree(writer, node->left);
        huffman_write_tree(writer, node->right);
    }
}

// Kirjoittaa lähtötiedoston tavut annettuin huffman-koodein ilmaistuna
void huffman_write_data(bitwriter *writer, FILE *in_file, huffman_code *codes) {
    bytereader reader = bytereader_init(in_file, 0);
    while (reader.bytes_left) {
        uint8_t byte = bytereader_read(&reader);
        bitwriter_write(writer, codes[byte].data, codes[byte].length);
    }
    bytereader_finish(&reader);
}

// Luo pakatun tiedoston yllä kuvattujen funktioiden avulla. Tiedoston rakenne:
//    alkuperäisen tiedoston koko 8 tavua
//    käytetty Huffman-puu: 319 tavua 7 bittiä
//    varsinainen data n, missä n on luonnollinen luku :)
void huffman_encode(FILE *in_file, FILE *out_file) {
    uint64_t original_size;
    uint64_t *freqs = huffman_read_frequencies(in_file, &original_size);
    huffman_node *tree = huffman_create_tree(freqs);
    huffman_code *codes = huffman_find_codes(tree);

    fwrite(&original_size, sizeof(uint64_t), 1, out_file);
    
    bitwriter writer = bitwriter_init(out_file, sizeof(uint64_t));
    huffman_write_tree(&writer, tree);
    huffman_write_data(&writer, in_file, codes);
    bitwriter_finish(&writer);

    free(codes);
    huffman_tree_destroy(tree);
    free(freqs);
}
/*****************************************************************************/

// Lukee aiemmin tallennetun Huffman-puun rekursiviisesti. Tallennuksen periaate 
// on kuvattu huffman_write_tree-metodin yhteydessä.
huffman_node *huffman_read_tree(bitreader *reader) {
    uint8_t is_leaf = bitreader_read8(reader, 1);
    if (is_leaf) {
        uint8_t byte = bitreader_read8(reader, 8);
        return huffman_node_create(byte, 0, NULL, NULL);
    } else {
        huffman_node *left = huffman_read_tree(reader);
        huffman_node *right = huffman_read_tree(reader);
        return huffman_node_create(0, 0, left, right);
    }
}

// Purkaa tiedoston seuraavan koodin ja palauttaa sitä vastaavan tavun.
// Ykkösbitillä siirrytään puussa oikealle ja nollabitillä vasemmalle, kun
// päädytään lehteen, on purettu tavu lehden arvo.
uint8_t huffman_read_code(bitreader *reader, huffman_node *node) {
    if (node->left == NULL && node->right == NULL) {
        return node->byte;
    }

    if (bitreader_read8(reader, 1)) {
        return huffman_read_code(reader, node->right);
    } else {
        return huffman_read_code(reader, node->left);
    }
}

// Lukee huffman-koodin annetun puun avulla ja kirjoittaa tuloksen tiedostoon.
void huffman_decode_data(
    bitreader *reader, FILE *out_file, uint64_t byte_count, huffman_node *tree
) {
    bytewriter writer = bytewriter_init(out_file, 0);
    while (byte_count--) {
        bytewriter_write(&writer, huffman_read_code(reader, tree));
    }
    bytewriter_finish(&writer);
}

void huffman_decode(FILE *in_file, FILE *out_file) {
    uint64_t original_size;
    fseek(in_file, 0, SEEK_SET);
    fread(&original_size, sizeof(uint64_t), 1, in_file);

    bitreader reader = bitreader_init(in_file, sizeof(uint64_t));
    huffman_node *tree = huffman_read_tree(&reader);
    huffman_decode_data(&reader, out_file, original_size, tree);
    bitreader_finish(&reader);
}