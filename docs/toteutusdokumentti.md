# Toteutusdokumentti
## Ohjelman yleisrakenne
### Tiedostot
Ohjelman koodi on jaettu neljään tiedostoon:
* src/util.c sisältää I/O-koodin puskuroivaan tiedoston kirjoittamiseen ja lukemiseen sekä tavu- että bittitasolla.
* src/huffman.c sisältää Huffman-koodauksen sekä siihen tarvittavat tietorakenteet (puu, minimikeko)
* src/lzw.c sisältää LZW-koodauksen sekä siihen tarvittavat tietorakenteet (hajautustaulu, merkkijonotaulukko)
* src/main.c sisältää yksinkertaisen komentorivikäyttöliittymän toteutuksen

Yksikkötestit löytyvät tiedostoista test/test_huffman.c ja test/test_lzw.c sekä nämä ajava koodi test/test_main.c-tiedostosta.

### Huffman
Huffman-koodausta varten on toteutettu puutitietorakenne sekä minimikeko (tavujen esiintymistihyden mukaan) tirakirjan kuvausten mukaisesti.

Varsinainen pakkaaminen toimii seuraavasti:
1. Luetaan tavujen esiintymistiheydet tiedostosta
2. Luodaan näiden perusteella Huffmanin puu
3. Etsitään puusta tavuja vastaavat koodit taulukkoon
4. Kirjoitetaan tiedostoon alkuperäisen tiedoston koko (8 tavua)
5. Serialisoidaan Huffmanin puu tiedostoon (320 tavua)
6. Pakataan varsinainen data selvitettyjen koodien avulla

ja purkaminen:
1. Luetaan alkuperäisen tiedoston koko
2. Deserialisoidaan koodaukseen käytetty puu
3. Puretaan koodit yksitellen, edeten puussa aina bitti kerrallaan kunnes koodi päättyy.

### LZW
Varsinainen pakkaaminen ja purkaminen on toteutettu viitteissä olevan pseudokoodin pohjalta. Tätä varten tarvittiin kaksi tietorakennetta: sanakirja, jossa avaimet ovat merkkijonoja ja arvot kokonaislukuja sekä sanakirja, jossa avaimet ovat kokonaislukuja ja arvot merkkijonoja.

Algoritmin luonteesta johtuen jokainen lisätty merkkijono muodostuu aina jostain aiemmasta merkkijonosta, johon on lisätty yksi uusi merkki. Kokonaisluvut kasvavat järjestyksessä ja ovat siten uniikkeja. Toteutinkin ensimmäisen sanakirjan hajautustauluna, jonka avaimet ovat (prefix, token)-pareja, joissa prefix on alkumerkkijonolle annettu kokonaisluku ja token uuden merkin tavuarvo. Toisen sanakirjan toteutin yksinkertaisesti tauluna, jossa avainta vastaa pari, joka koostuu alkumerkkijonon avaimesta sekä lisättävästä merkistä. Tällöin kaikki jonon merkit voidaan lukea rekursiivisesti. Toteutukset ovat tehokkaita, ja kattavat kaikki tässä käyttötarkoituksessa vastaan tulevat tilanteet.

## Saavutetut aika- ja tilavaatimukset
Kokonaisten pakkausalgoritmien O-analyysi- ja tilavaatimuksien analysointi ei ole kovin mielekästä. Sekä pakkaaminen että purkaminen molemmilla algoritmeilla tapahtuu lineaarisessa ajassa suhteessa syötteen (lähdetiedoston) kokoon yhtään suuremmilla syötteillä. Tilavaatimus (muistinkäyttö) on aina vakio. Huffman-pakkaamisen kohdalla tiedosto käydään läpi kaksi kertaa: ensin luetaan frekvenssit, myöhemmin tehdään datan pakkaaminen. Molemmat näistä tapahtuvat puhtaasti lineaarisessa ajassa. Varsinaisen Huffman-puun muodostaminen tapahtuu vakioajassa tiedoston koon suhteen. LZW:n tapauksessa tiedosto käsitellään vain kerran, ja samalla kasataan koodaustaulu. Koska hajautustaulun koko on rajattu, voidaan sitä pitää vakioaikaisena suhteeessa syötteen kokoon. Toki on mahdollista, että se toimii joillakin syötteillä paremmin kuin toisilla, mutta tällä ei ole niinkään yhteyttä syötteen kokoon. On hyvä huomata, että käytännössä jo lineaarisessa ajassa tapahtuvat tiedostojen luku ja kirjoitus ovat suoritusta eniten rajoittava tekijä (tämän huomaa jo suoritusajan suurista eroista eri kiintolevyillä). Siispä lähes kaikki pakkausalgoritmit ovat tässä suhteessa samankaltaisia.

## Algoritmien vertailu
Aika- ja tilavaatimusten sijaan mielekkäämpää on vertailla pakkaustehoa ja suorituskykyä. Keskityn erityisesti pakatun tiedoston kokoon, sillä suorituskyky on niin samansuuntainen molemmilla algoritmeilla, että toteutuksen yksityiskohdat vaikuttavat siihen merkittävästi. Täysin satunnaiselle yhden gigatavun tiedostolle suoritusajat olivat fuksiläppärillä seuraavat:
||Huffman|LZW|
|-|-|-|
|Pakkausaika|10 443 ms|14 894 ms|
|Purkuaika|24 687 ms|8 426 ms|

Prosessin muistinkäyttö oli aina enimmillään 1560K, sisältäen projektin ja standardikirjaston ohjelmakoodit, tiedostojen puskurit ja algoritmien tarvitsemat tietorakenteet.

Testausdokumentissa on pakkaustehot taulukoituna eri testitiedostoille. Kaikkien testattujen UTF8-muotoisten e-kirjojen kohdalla Huffmanin pakkausteho (pakatun tiedoston koko suhteessa alkuperäiseen) oli keskimäärin 0.62 ja LZW:n 0.47. LZW pakkasi tehokkaammin jokaisen kirjan kohdalla.

Tiivistetysti algoritmien ero on käytännössä se, että Huffman luo koodit suoraan yksittäisten merkkien esiintymistiheyksistä ja LZW toistuvista merkkijonoista. Toisin sanoen Huffman toimii hyvin erityisesti, kun eri merkkejä esiintyy tiedostossa epätasaisesti - näin on usein luonnollisen kielen kohdalla. LZW puolestaan toimii hyvin, kun samat merkkijonot toistuvat usein - tämäkin usein luonnollisen kielen kohdalla.

Testauksessa käy myös ilmi, että varsinainen pakattu data ei ikinä vie Huffmanin tapauksessa enempää tilaa kuin alkuperäinen, ja tälle löytyy myös matemaattinen perusta. LZW:llä pakattu tiedosto saattoi taas olla huomattavastikin suurempi kuin alkuperäinen, kun syötettin jo valmiiksi pakattu tiedostomuoto tai satunnaista dataa.

Huffmanin algoritmissa kätetty puu täytyy kirjoittaa pakattuun tiedostoon, ja toteutuksessani tähän menee aina 320 tavua. Lisäksi tiedoston alkuun kirjoitetaan tiedoston koko, johon menee 8 tavua. LZW:n kohdalla tällaista ylimääräistä lisää ei ole lopetuskoodia (1-2 lisätavua) lukuunottamatta. Tällä voi olla huomattava vaikutus erityisesti pienien tiedostojen kohdalla, mutta nopeasti se käy suhteessa pieneksi.

## Työn mahdolliset puutteet ja ratkaisuehdotukset
Huffmanin pakkauksessa tiedoston kokoa lisää hieman alkuun kirjoitettu tiedoston koko. Tämän voisi kenties hoitaa lyhemminkin luottaen, että kaikki tavut on luettu tiedoston loppuessa ja varmistaen, että viimeisen oikean bitin jälkeen ei tule enää todellista koodia vastaavaa dataa.

LZW lopettaa sanakirjan täyttämisen, kun koodin maksimipituus tavoitetaan. Tämä voi aiheuttaa ongelmia, jos tiedoston alkuosa ei vastaa sisällöllisesti jatkoa. Ratkaisuna voisi olla sanakirjan tyhjentäminen sen täyttyessä. Algoritmi voisi myös seurata pakkaustehoa, ja päättää sen perusteella, miten esimerkiksi sanakirjan tyhjentämisen osalta toimitaan - tämä parantaisi tulosta erityisetsi datalla, jossa ei juuri ole toisteisuutta.

Yleisesti ohjelma on tällä hetkellä hyvin virheherkkä, eikä niitä käsitellä ollenkaan. Esimerkiksi jos purettavaksi annettu syöte on virheellinen, ohjelma todennäköisesti kaatuu tai tuottaa hiljaa virheellisen lopputuloksen. Jos kirjoittaminen keskeytyy, keskeneräinen tiedosto jää levylle. Edes malloc-operaatioiden onnistumista ei tarkisteta. Ratkaisuna olisi lisätä yleisesti varmistuksia ja näihin liittyviä virheilmoituksia koodiin sekä käsitellä virheet oikeaoppisesti.

Yksikkötestejä tehdään vain yksittäisillä koodauksilla/syötteillä, jotka tosin kattavat eri tilanteet mielestäni melko kattavasti. Taustalla on se, että tämän kaltaisille algoritmeille oikeiden syötteiden laskeminen käsin on työlästä. Ratkaisuna voisi olla laskea koneellisesti testisyötteitä, mutta toki myös tässä voi tulla virheitä.

## Laajojen kielimallien käyttö
En käyttänyt tekoälyä oman projektini toteutuksessa.

## Viitteet
Algoritmit on totettu pääosin seuraavien sivujen pseudokoodien pohjalta:
* https://www.geeksforgeeks.org/huffman-coding-greedy-algo-3/
* https://www.geeksforgeeks.org/lzw-lempel-ziv-welch-compression-technique/

Perusalgoritmien toteutuksessa hyödynsin myös [tirakirjaa](https://www.cs.helsinki.fi/u/ahslaaks/tirakirja/).