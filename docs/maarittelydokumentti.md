# Määrittelydokumentti

* Ohjelmointikieli: C
* Kieli: suomi
* Voin vertaisarvioida: C/C++, Python, Java, C#, JavaScript
* Opinto-ohjelma: Tietojenkäsittelytieteen kandidaatti (TKT)


## Aihe
**Ongelma**: Usein tiedostot vievät rakenteestaan johtuen jopa merkittävästi enemmän levytilaa kuin olisi välttämätöntä. Yksi apu tähän ovat pakkausalgoritmit, joilla täysin sama tieto saadaan tiivistettyä selkeästi pienempään tilaan, ja purettua myöhemmin jopa häviöttömästi. Näin tiedostot vievät vähemmän tallennustilaa, ja esimerkiksi niiden siirtäminen verkon kautta on nopeampaa.

**Algoritmit**: Toteutan työssäni jonkin LZ-algoritmeista sekä Huffman-koodauksen. Pakkausalgoritmien toteuttamiseen tarvitsee todennäköisesti myös joitain perustietorakenteita, kuten keon ja jonkinlaisen puurakenteen, jotka kirjoitan myös itse. Mikäli aikaa jää ja mielenkiintoa riittää, on myös kolmannen pakkausalgoritmit tekeminen mahdollisuus.

**Syötteet**: Ohjelma saa syötteeksi tiedostoja, jotka se joko pakkaa tai purkaa valitulla algoritmilla. Kaikki purkamiseen tarvittava tieto löytyy pakatusta tiedostosta. Algoritmi on häviötön, joten purettu tiedosto vastaa täysin alkuperäistä.

**O-analyysi**: Kun n on syötteenä olevan tiedoston koko, on molemmilta algoritmeilla odotettavissa, että suoritusaika on O(n). Tämä johtuu siitä, että tiedoston lukeminen, pakkaus/purku ja kirjoittaminen ovat kaikki O(n)-aikaisia tehtäviä. Syötteen koko ei juuri vaikuta esimerkiksi Huffman-puun rakentamiseen, sillä kyseessä on O(k log k)-algoritmi, jossa k on tässä tapauksessa merkkien määrä, joka on vakio (256).

**Käytännön aika/tilavaatimus**: Yleisesti sanottakoon, että pyrin siihen, että ohjelma suoriutuu kohtuuajassa isoillakin tiedostoilla (max 1 min/GB?). Tilavaatimuksen suhteen pyrin tekemään tiedostojen käsittelyn pienemmissä paloissa, jolloin ohjelman käyttämä muisti ei mene tietyn rajan yli minkään kokoisella tiedostolla.

**YDIN: Harjoitustyöni ydin on toteuttaa Huffman-koodaus ja jokin LZ-algoritmi käytännöllisen tehokkaasti C-kielellä sekä vertailla näiden suoriutumista. Ohessa on tärkeää pitää testaus ja dokumentaatio kunnossa.**

Viitteet
- [https://en.wikipedia.org/wiki/LZ77_and_LZ78](https://en.wikipedia.org/wiki/LZ77_and_LZ78)
- [https://www.geeksforgeeks.org/huffman-coding-greedy-algo-3/](https://www.geeksforgeeks.org/huffman-coding-greedy-algo-3/)