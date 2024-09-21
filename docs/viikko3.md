## Tuntikirjanpito
|pvm|tunnit|mitä?|
|-|-|-|
|20.9|4|I/O-koodin kirjoittamista|
|21.9|5|Huffmanin koodaus "loppuun", muttei sitten kuitenkaan|

## Q&A

Mitä olen tehnyt tällä viikolla? / Miten ohjelma on edistynyt?
> Kirjoitin tiedostojen lukuun/kirjoittamiseen tarvittavan koodin hyvälle mallille. Lisäksi sain huffmanin koodauksen toimimaan osittain.

Mitä opin tällä viikolla / tänään?
> Kikkaileen tiedostojen lukemisen ja kirjoittamisen kanssa, kun käsitellään bittejä eikä kokonaisia tavuja. Alkaa se jo luonnistua!

Mikä jäi epäselväksi tai tuottanut vaikeuksia?
> Huomasin, että ohjelma toimii osalla syötteistä, muttei kaikilla. Selvittelyn jälkeen myös taustalla oleva syy löytyi: jostain syystä oletin, että koodit olisivat enintään 32 bitin mittaisia, mutta jos syötteen frekvenssit ovat erittäin epätasaiset, on mahdollista, että näistä tulee jopa 255-bittisiä! Olin tähän mennessä testannut vain satunnaisilla syötteillä, jolloin puu on ollut aina syvyydeltään $\log_{2}^256$.

Mitä teen seuraavaksi?
> Korjaan yllä olevan ongelman. Tämä vaatii jonkin verran koodin muokkaamista, sillä nyt koodit ovat kaikkialla uint32_t-tyyppisiä. Ratkaisu ei kuitenkaan ole erityisen monimutkainen, kun sen ehdin tekemään - luon koodeille uuden structin, joka sisältää 32 tavun taulukon sekä koodin pituuden. Tämän käsittely on I/O:ssa yksinkertaista, eikä muutokset muuallakaan ole liian suuria.
>
> Tämän jälkeen dokumentaatio kuntoon ja testit ajan tasalle, niin vertaisarviointiin on sitten toinen algoritmi kokonaan toteutattuna, ja mahdollisesti toista aloitettu.