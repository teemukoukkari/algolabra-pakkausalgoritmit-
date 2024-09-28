## Tuntikirjanpito
|pvm|tunnit|mitä?|
|-|-|-|
|24.9|5|Tarvittavat muutokset huffmanin koodaukseen|
|25.9|4|Pikkubugin etsiminen huffmanin algoritmin toteutuksesta|
|27.9|8|LZW opettelu ja toteutus hyvälle mallille|
|28.9|12|LZW loppuun! Paljon pikkuparannuksia ympäri projektia. |

## Q&A

Mitä olen tehnyt tällä viikolla? / Miten ohjelma on edistynyt?
> Aluksi korjasin edellisellä viikolla ilmenneen ongelman Huffmanin algoritmin toteutuksessa. Tämän jälkeen etsin tuntikaupalla epämääräistä bugia, joka paljastui lopulta johtuvan väärästä laskujärjestyksestä koodien muodostamisen yhteydessä. Ongelma koski vain koodeja, joissa jokin kahdeksalle jaollinen bitti oli 1, joten kohdan löytäminen oli haastavaa - lopulta README-dokumentin M-kirjain paljasti!

> Kun Huffman-koodaus tuli valmiiksi, lähdin tutustumana LZ*-algoritmeihin, ja päädyin valita toteutettavaksi LZW:n. Tämän kirjoittelu sujuikin mukavasti maratonina, eikä mitään erityisen suuria ongelmia ilmennyt.

> Loppuajan käyntin jo kirjoitetun koodin rakenteen parantamiseen, pääohjelman kirjoittamiseen, kaiken IO:n siirtämiseen util.c-tiedostoon, sekä jonkintasoiseen koodin kommentointiin. Tein myös lyhyen ja ytimekkään käyttöohjeen.

Mitä opin tällä viikolla / tänään?
> Opin paljon eri LZ*-algoritmien toiminnasta ja eroista, sillä käytin näihin perehtymiseen ja valitsemiseen hyvin aikaa. Suurin oivallus lienee, että tavallaan LZ77 ja LZSS ovat samankaltaisia peruslogiikaltaan, samoin LZ78 ja LZW. Yleisesti hieman suuremmaan C-projektin toteuttaminen on ollut opettavainen kokemus.

Mikä jäi epäselväksi tai tuottanut vaikeuksia?
> Ei mitään selvittämätöntä!

Mitä teen seuraavaksi?
> Nyt kun algoritmit toimivat ja omat tietorakenteet on totetutettu, on hyvin aikaa keskittyä muihin asioihin. Tällä hetkellä yksikkötestien kattavuus on jäänyt pahasti jälkeen ollen noin 20%, joten niiden kirjoittaminen on edessä seuraavaksi. Näiden lisäksi ohjelmaa on hyvä testata monipuolisesti erilaisilla tiedostotyypeillä, ja toimivuuden lisäksi tutkia myös pakkaustehoa ja suoritusaikaa. Ohella kirjoitan toteutus- ja testausdokumentit.