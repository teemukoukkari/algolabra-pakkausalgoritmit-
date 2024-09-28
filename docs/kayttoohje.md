# Käyttöohje
## Kääntäminen
Ohjelman kääntämisen ainoat riippuvuudet ovat GNU make ja gcc. Lähtökohtaisesti nämä löytyvät valmiiksi asennettuina kaikista linux-jakeluista.

Ohjelman kääntäminen onnistuu juurihakemistossa komennolla
```
$ make build
```

Testien ajaminen onnistuu komennolla
```
make test
```

Windowsille kääntämistä varten minulla ei ole valmista scriptia, mutta sen pitäisi olla suhteellisen yksinkertaista. Tarvittaessa voin myös tehdä suoritettavan exe-tiedoston valmiiksi.

## Ohjelman käyttäminen
Ohjelman käytetään kaikessa yksinkertaisuudessaan näin:

```
$ ./algolabra [compress/decompress] [huffman/lzw] [input file] [output file]
```

