## Tuntikirjanpito
|pvm|tunnit|mitä?|
|-|-|-|
|10.9.|3|Kurssiin tutustuminen, aiheen valinta, määrittelydokumentti|
|11.9.|2|Ohjelmointiympräristön luominen, testauskirjaston valinta ja opettelu|
|13.9.|4|Tarkempi tutustuminen Huffman-koodaukseen ja ohjelmoinnin aloitus, testien kattavuusscripti|
|15.9.|10|Huffmanin algoritmia eteenpäin, testejen kirjoittamista|
|16.9.|1|Repon siistimistä, viikkoraportti|

## Q&A
Mitä olen tehnyt tällä viikolla?
> Aloitin viikon tutustumalla kurssin suoritukseen ja aiheisiin, päätyen pakkausalgoritmien toteuttamisen.
>
>Valitsin harjoitustyöhöni kieleksi C:n, sillä se soveltuu erinomaisesti projektiin, jossa käsitellään tiedostoja tehokkaasti tavu/bittitasolla. Lisäksi en ole päässyt käyttämään sitä pitkään aikaan, vaikka kovasti tykkäänkin. Toinen vaihtoehto olisi toki ollut C++, mutta en suoranaisesti tarvitse sen ominaisuuksia ja turhaa monimutkaisuutta näin pieneen projektiin - ja tuleepahan nyt toteutettua kaikki perustietorakenteetkin itse.
>
>Testikirjastoksi valitsin [Unity](https://github.com/ThrowTheSwitch/Unity):n sen yksinkertaisuuden johdosta. Lisäksi toteutin scriptin testikattavuuden selvittämiseksi.
>
>Tällä viikolla sain opiskeltua Huffman-koodauksen toiminnan sekä pääsin hyvään vauhtiin ohjelman kirjoittamisessa. Nyt tehtynä ovat keko ja puurakenne Huffman-koodausta varten, tavujen frekvenssejen luku tiedostosta, Huffman-puun luonti sekä koodien haku puusta.


Miten ohjelma on edistynyt?
> Olen päässyt mielestäni hyvin alkuun algoritmin ja testejen kirjoittamisessa. Tällä hetkellä Huffmanin koodaus on noin puolessa välissä. Koodi vaikuttaa tähän asti toimivan järkevästi, mitä olen saanut kokeiltua mallisyötteillä.

Mitä opin tällä viikolla / tänään?
> Paljon Huffman-koodauksesta ja monimutkaisemman algoritmit toteuttamisesta yleisesti. Tein testit ensimmäistä kertaa näin hyvissä ajoin (jopa etukäteen), ja niistä oli oikeasti hyötyä koodia kirjoittaessa! Lisäksi koodin toimintavarmuutta/bugittomuutta tullut mietittyä aiempaa enemmän, ei pointereita turhaan...

Mikä jäi epäselväksi tai tuottanut vaikeuksia?
> Toistaiseksi ei ongelmia.

Mitä teen seuraavaksi?
> Tällä hetkellä koodia ei ole kommentoitu ollenkaan, se olisi hyvä hoitaa ennen kuin jatkaa. Testejen pilkkomista pitää harkita. Seuraavaksi aion kirjoittaa Huffmanin koodauksen loppuun. Haastavana asiana tiedoston luku/kirjoittaminen bitti kerrallaan tavurajoista välittämättä mahdollisimman tehokkaasti.