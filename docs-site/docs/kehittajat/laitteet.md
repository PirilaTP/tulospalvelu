# Laiteintegraatiot

Ohjelmat ottavat tietoa leimaus- ja ajanottolaitteilta (Emit-lukijat ja emiTag, maali- ja
lähtökellot, RFID-lukijat) ja ohjaavat näyttötauluja ja tulostimia. Laitteiden kuvaus on jaettu
kolmeen asiakirjaan; tämä sivu kertoo, mistä mikin löytyy.

| Kysymys | Asiakirja |
|---|---|
| Mitä laitteen kanssa tapahtuu, mitkä laiteperheet ovat tuettuja, miten yhteys palautuu | [Käsikirja: Laitteet](kasikirja/10-laitteet.md) |
| Emit-kortin luku ja leimantarkastus | [Käsikirja: Leimaukset](kasikirja/05-leimaukset.md), [Aineisto: Emit-luenta](kasikirja/aineisto/emit-luenta.md) |
| Maali- ja väliaikojen kirjaus laitteelta | [Käsikirja: Ajanotto](kasikirja/04-ajanotto.md) |
| Protokollat, kirjastot, ajonaikaiset riippuvuudet | [Riippuvuudet ja laitteet](arkkitehtuuri/riippuvuudet.md#13-laitteet-ja-protokollat-devices-and-protocols) |
| Laitteiden kytkeminen päälle valitsimilla ja asetuksilla | [Asetukset ja käännösvalitsimet](arkkitehtuuri/ominaisuusvalitsimet.md), [Asetusavaimet](asetukset/asetusavaimet.md) |
| Laitesanomien käsittely muistiturvallisuuden kannalta | [Muistiturvallisuus](../laatu/raportit/muistiturvallisuus.md) |
| Lukijasäikeet ja lukot | [Säikeet ja lukot](arkkitehtuuri/saikeet-ja-lukot.md) |

Laitteisiin liittyvät tunnetut viat näkyvät käsikirjan luvun lopussa ja
[portaalissa](../laatu/portaali.md) suodattimella *Käytännön vaikutus: Laitteiden toiminta*.
Käyttöohjeet laitteiden käyttöönotosta ovat HkKisaWin-referenssin luvussa 6 ja liitteissä
(esimerkiksi [emiTag-laitteet](../hkkisawin/liite_9._emitag-laitteiden_kaytto.md) ja
[Alge Timy](../hkkisawin/liite_7._algen_timy_usb_portissa.md)).

## Ilman laitetta

Emit-leimasimen toimintaa voi kokeilla selaimessa
[EMIT250-simulaattorilla](https://ikivela.github.io/emit250-simulator/). Käsikirjan luku
[Laiteyhteys ilman laitetta](kasikirja/10-laitteet.md#110-laiteyhteys-ilman-laitetta) kertoo, mitä
koodissa on testaamista varten.
