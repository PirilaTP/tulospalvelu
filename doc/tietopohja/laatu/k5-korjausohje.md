# K-5: vikakorttien korjauskierros – tehtävänanto

Repo `C:\dev\tulospalvelu`. Korjaat vikakortteja riippumattoman Opus-tarkastuksen perusteella
(`doc/tietopohja/laatu/tarkastukset/k5/Q1.md`–`Q6.md`). Lue ensin `doc/tietopohja/kirjoitusohje.md`.
Generoidut lohkot ja rivi `<!-- rekisteritila: ... -->` jäävät koskematta. Rekistereitä et muokkaa:
vaikutusrivien muutokset kirjoitat omaan CSV-tiedostoosi (`id,ulottuvuus,arvo,peruste,lahde`,
koko rivi, korvaa vanhan). Riskiarvioon, prioriteetteihin ja havaintoriveihin ei kosketa: jos
tarkastaja toteaa riskiarvion tai rekisterin vääräksi, kortti kuvaa koodin mukaisen tilanteen ja
sanoo luvussa 12, että rekisterin arvo tarkistetaan (`seuranta.md` KE-taulukko).

## Osa 1: tarkastuserän korjaukset

Toteuta sinulle annettujen tarkastusraporttien jokainen "Korjattavat kohdat" -kohta. Tarkista
korjaus lähteestä ennen kirjoittamista; jos et ole samaa mieltä, jätä tekemättä ja perustele.

## Osa 2: järjestelmälliset virhetyypit koko kortistosta

Käy läpi sinulle annetut kortit (koko kortisto on jaettu kolmeen osaan) ja korjaa nämä tyypit,
kun ne esiintyvät. Älä etsi uusia vikoja.

| # | Virhetyyppi | Korjaus |
|---|---|---|
| S1 | Luvuissa 1, 7 tai 11 sana "vahvistettu" pelkälle koodiluennalle tai riippumattomalle lähdetarkastukselle | "luettu koodista ja riippumattomasti tarkastettu"; "vahvistettu" vain kokeelle, testikehykselle tai ajolle |
| S2 | "aineisto ei kerro" / "ei tarkistettu", vaikka `#if`, projektin lähdeluettelo tai kutsujahaku ratkaisee | ratkaise rajatulla haulla ja merkitse "[luettu koodista K-vaiheessa, ei tarkastettu]", tai jätä avoimeksi, jos haku ei riitä |
| S3 | Ohjelmat rajattu toiseen haaraan tarkistamatta toista (esim. viesti suljettu pois, koska siinä ei ole vaiheen vaihtoa, vaikka sama koodi laukeaa muista toiminnoista) | tarkista toisen haaran vastine; kirjaa ohjelmatieto ohjelmatiedostoosi (`id,ohjelmat,peruste`) |
| S4 | Ehdollinen toiminta kuvattu ehdottomana (hylkäys, välitys, raja, tallennus) | lisää ehto |
| S5 | Verkko- tai laitesyötteen korteissa sanoman kenttäarvoja, merkkipaikkoja tai raja-arvoja, jotka toimisivat ohjeena (sääntö 7) | yleistä: "sanoma, jonka luokka- tai pituuskenttä on rajojen ulkopuolella" |
| S6 | Vanhentunut lause "Rekisterin ohjelmaluettelo sisältää …" (luettelo on jo kavennettu) | poista tai muotoile: "lähdetiedosto käännetään myös X:ään, mutta …" |
| S7 | Ilmenemisrivit epäjohdonmukaiset: asetusta vaativa vika → `normaalikaytto = ei_perusteltu` (jos asetus ei ole oletus) tai `ei_tiedossa` (jos asetuksen yleisyys tuntematon) ja `poikkeustilanne = mahdollinen` (harvinainen asetus on poikkeustilanne); kumpikaan ei saa olla `ei_perusteltu` molemmissa, ellei vika ole kuollutta koodia tai ei koske ajettavaa ohjelmaa | korjaa vaikutusrivit CSV-tiedostoosi |
| S8 | Vaikutusrivi `vahvistettu` testikehyksen perusteella, vaikka kehys ohitti laukaisupolun tai seurausta ei havaittu | `mahdollinen`, peruste kertoo miksi |
| S9 | Vaikutusrivi `ei_perusteltu` pelkän riskiarvion "ei"-arvon perusteella ilman omaa perustetta, vaikka välillinen vaikutus on mahdollinen | `ei_tiedossa` |
| S10 | Jumiutuvan säikeen vaikutus rajattu yhteen sanomatyyppiin, vaikka sama silmukka käsittelee kaikki | laajenna |
| S11 | Asetusvaatimus päätelty käsittelijän yhdestä haarasta, vaikka funktiota kutsutaan myös toisesta haarasta oletusasetuksella | tarkista kutsupaikat |
| S12 | Globaali lukko tai lippu kuvattu kapeammaksi kuin koodissa (esim. "TCP-lähetysten lukko", vaikka sama lukko suojaa kaikkea TCP-liikennettä) | kuvaa koodin mukainen laajuus |
| S13 | Päätelty seuraus kirjoitettu luvussa 1 tosiasiana (esim. MSVC-päättyminen, ViestiLuennan päättyminen ilman tarkistusta) | konditionaali ja [päätelty] |
| S14 | Ohjelmarajauksessa kääntäminen sekoitettu kutsumiseen ("käännetään vain GUI:hin", vaikka käännetään kaikkiin mutta kutsutaan vain yhdestä) | erottele: käännetään X, kutsutaan Y |

## Tuotokset

- Korjatut kortit.
- `C:\Users\NIKO-P~1\AppData\Local\Temp\claude\a25cf22582434270\acb6ada1-4283-4682-805b-353a58fe6cff\scratchpad\k5korj\vaikutukset_<osa>.csv` ja `ohjelmat_<osa>.csv` (vain muuttuvat rivit).
- Loppuraportti: osa 1 kohta kerrallaan (tehty / ei tehty, miksi); osa 2 virhetyyppi kerrallaan
  korjatut kortit; uudet ristiriidat rekisterin kanssa.

Saat muokata vain sinulle annettuja kortteja ja omia CSV-tiedostojasi. Ei skriptejä (paitsi lukevat
komennot), ei verkkoa.
