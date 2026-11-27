# Auditointiin osallistuminen

Auditointi on vaiheittainen: erät C-1–C-13 (vaihe C), tietopohja (vaihe K), koko koodin
katselmointi (C-14) ja korjaukset (vaihe D). Vaiheiden tila: [Eteneminen](../laatu/eteneminen.md).
Menetelmä ja suunnitelmat: [Auditointisuunnitelma](../laatu/raportit/suunnitelma.md) ja
[C-14](../laatu/raportit/c14-suunnitelma.md).

## Kolme kerrosta

| Kerros | Missä | Kuka muuttaa |
|---|---|---|
| **Rekisterit** (kanoninen) | `doc/arkkitehtuuri/rekisteri/`: `havainnot.md`, `riskiarvio.csv`, `tekninen-velka.csv`, `vaikutukset.csv`, `korjauskokonaisuudet.csv`, `tiedostot.csv`, `vaiheet.csv`; kysymykset `seuranta.md`, päätökset `omistajan-paatokset.md` | auditoija, korjaaja |
| **Kirjoitetut johdannaiset** | vikakorttien teksti `doc/tietopohja/vikakortit/`, käsikirja `doc/tietopohja/kasikirja/` | kirjoittaja, tarkistettuna rekisteriä vasten |
| **Generoidut** | korttien lohkot, `vikakortisto.md`, portaalin data, tämän sivuston kehittäjä- ja laatuosiot | työkalut; ei koskaan käsin |

Alempi kerros ei koskaan korjaa ylempää. Jos kortti ja rekisteri ovat ristiriidassa, rekisteri
voittaa ja kortti korjataan.

## Uuden havainnon lisääminen

1. **Rekisteri.** Lisää rivi `rekisteri/havainnot.md`:hen (seuraava vapaa tunnus, erä, kuvaus,
   lähdeviitteet `polku:rivi` suhteessa `TPsource/V52/`, revisio, näyttö). Lisää riskiarvio
   `riskiarvio.csv`:hen (tai T-rivi `tekninen-velka.csv`:hen) ja käytännön vaikutukset
   (`tietopohja.py . tuo <csv>`). Kytke korjauskokonaisuuteen `korjauskokonaisuudet.csv`:ssä.
2. **Kortti.** `python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . runko H-NNN`, kirjoita kortti
   [kirjoitusohjeen](tietopohja/kirjoitusohje.md) mukaan, `lohkot`, sitten `kuittaa H-NNN`.
3. **Käsikirja.** Lisää havainto toiminnolle `doc/tietopohja/kasikirja/toiminnot.csv`:ssä ja aja
   `tietopohja.py . kasikirja` ja `tietopohja.py . kooste`.
4. **Tarkistukset.** Kaikki `AGENTS.md`:n ylläpitokomennot läpi (myös `sivusto.py . --tarkista`).
5. **Riippumaton tarkastus.** Uusi korkea tai keskitason väite tarkastetaan toisella agentilla tai
   ihmisellä, joka yrittää kumota sen (`independent-verifier`).

Kun rekisteri on päivitetty, havainto näkyy ilman muuta käsityötä vikakortistossa
(`laatu/vikakortit/H-NNN/`), prioriteettijakaumissa, juurisyy-, vaikutus- ja kokonaisuussivuilla,
käsikirjan vikaluettelossa, portaalissa ja tarvittaessa käyttöohjesivun tietolaatikossa.

## Havainnon korjaaminen

`riskiarvio.csv`: `tila` = `korjattu`, `korjaus` = commit, ohjelmaversio ja testi. Kortin luku 13
kirjoitetaan muotoon "Miten vika korjattiin". Korjattu havainto siirtyy sivustolla korjattujen
luetteloon, ja kortilla näkyy linkki korjauscommitiin ja versioon. Katso
[Kehittämiseen osallistuminen](osallistuminen.md).

## Käyttöohjesivun tietolaatikko

Käyttöohjesivuille voi liittää laatikon, joka linkittää käsikirjaan ja sivun aiheeseen liittyviin
tunnettuihin ongelmiin. Kytkennät ovat tiedostossa `doc/tietopohja/kasikirja/kayttoohjeet.csv`
(sarakkeet `sivu`, `luku`, `toiminnot`, `havainnot`, `peruste`, `tarkistettu`). Havainto lisätään
vain, kun sen soveltuvuus sivun aiheeseen on tarkistettu kortista; peruste kirjataan.
Alkuperäistä ohjetekstiä ei muuteta.

## C-14: koodin katselmointi

Katselmoinnin yksikkö on funktio. Kattavuus mitataan työkalulla `tyokalut/kattavuus.py`, ja
katselmointi kirjataan rekistereihin `koodialueet.csv` ja `katselmoinnit.csv`
([C-14-suunnitelma](../laatu/raportit/c14-suunnitelma.md), luku 3). Sivusto näyttää etenemisen
automaattisesti, kun rekisterit ovat olemassa: [Kattavuus](../laatu/kattavuus/index.md).
Lähdeviite ei ole katselmointi: vain taso K4 tai K5 tarkoittaa kokonaan katselmoitua.
