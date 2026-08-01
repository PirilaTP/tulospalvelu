# Changelog

Muutoshistoria pääkehityshaaraan (main) tehdyistä committeista, uusin ensin.

| Pvm | # | Muutos |
|---|---|---|
| 2026-07-28 | #50 | Ratatietoja: kartan kulmapisteiden muokkaus merkitsee nyt muutokset tallentamattomiksi — EdtMapLeft/Top/Right/Bottom- ja EdtScale-kentillä ei ollut OnChange-käsittelijää, joten pelkkien karttakalibrointitietojen muokkaus ei merkinnyt tietoja muuttuneiksi (rataMuutoksia). Tämä saattoi johtaa siihen, että käyttäjä sulki Ratatiedot-ikkunan ja muutokset katosivat tallentamatta ilman varoitusta. Korjattu lisäämällä yhteinen käsittelijä kaikille viidelle kentälle; koskee sekä HkKisaWiniä että ViestiWiniä, koska UnitRadat.cpp/h/dfm on jaettu koodi. |
| 2026-07-26 | #47 | 253 and 254 reader code fix — Emit-lukijakoodien 253 ja 254 käsittelyn korjaus relay-puolella |
| 2026-07-16 | — | Build & layout changes to Relay — Zebra-lukijatuki puuttui viestipuolelta; tulostuksen layout-muutoksia |
| 2026-07-16 | #43 | 42 support for zebra fx9600 rfid reader — Zebra FX9600 RFID-lukijatuki |
| 2026-07-16 | #40 | Show competitor number instead of OK text on accepted punch-check result — Näytetään kilpailijanumero "OK"-tekstin sijaan hyväksytyssä leimantarkistuksessa |
| 2026-07-13 | #41 | Fix cbHk/UnitSarja — Korjaus cbHk/UnitSarja.cpp:hen |
| 2026-06-28 | #34 | Fix badge registration for new relay team — Korjattu emit-badgen rekisteröinti uudelle viestijoukkueelle |
| 2026-06-26 | #29 | Fix piikkikuvio sequence numbering and red-marking for partio courses — Piikkikuvion juokseva numerointi ja punainen merkintä partioradoilla korjattu |
| 2026-06-26 | #25 | HenkHaku: fix sort order and Unicode search inconsistency — Henkilöhaun lajittelu ja Unicode-hakuepäjohdonmukaisuus korjattu |
| 2026-06-26 | #23 | print EI LÄHTIJÖITÄ for every empty start minute — "EI LÄHTIJÖITÄ" tulostetaan jokaiselle tyhjälle lähtöminuutille |
| 2026-06-24 | #31 | automatic file printing in viestiwin — Viestikisassa automaattinen tulostus HTML-muotoon; korjattu oletustila ja lisätty Sulje-painike |
| 2026-06-17 | — | Add optional free-text field (Lisäteksti) to start list and result printouts — Valinnainen vapaa tekstikenttä lähtö- ja tuloslistoihin |
| 2026-06-07 | #21 | fix: reading back split configurations — Korjattu CSV-tuonti (Luetekstitiedostosta1Click): kenttämäärän tarkistus ja lohkon offset olivat väärin, minkä takia kaikki rivit hylättiin ääneti |
| 2026-06-07 | #19 | Fix/pakotalaikaraja comparison — PAKOTALAIKARAJA-vertailu oli >= eikä <=, jolloin pakotettu lähtöaika tallentui vain kun ero ylitti rajan eikä kun se oli rajan sisällä |
| 2026-05-25 | #17 | UnitGAZ.dfm resize — Yhteydet-välilehden ruudukko ja painike venyvät nyt ikkunan koon mukaan |
| 2026-05-19 | #15 | wcstok ISO C11 — wcstok-kutsut päivitetty ISO C11 -muotoon (3-argumentti + ctx) koko koodikannassa; yhteensopivuusmakro bcc32:lle |
| 2026-05-14 | #13 | VInit.cpp: revert korostus.lst to hardcoded name — korostus.lst-nimi palautettu kovakoodatuksi |
| 2026-05-12 | — | CI: add manual console binary package workflow — manuaalinen konsoliversion paketointi CI:hin |
| 2026-05-13 | #12 | fix/getversionex-deprecated — Poistettu vanhentunut GetVersionEx Windows API -kutsu |
| 2026-05-13 | #11 | update_build_version_to_5.22 — HK:n versio päivitetty 5.22:een |
| 2026-05-12 | #10 | rsync_docs — Dokumentaatio: rsync-pohjainen automaattinen tulosten siirto palvelimelle |
| 2026-05-12 | #9 | fix/partio-readout-dialog — Korjattu leimantarkistuskaavake partiokisoissa |
| 2026-05-09 | #7 | korostus-file-with-paiva-number — korostus.lst nimetään päivämäärän mukaan (korostus1.lst, korostus2.lst ...) kun kilpailupäiviä on useita |
| 2026-05-11 | — | Update HkIV.cpp |
| 2026-05-10 | #6 | start-time-from-rfid-over-raspi — Lähtöaika tallennetaan RFID-lukijalta kun käytetään PAKOTALAIKA + LÄHDEPISTEET L-parametrilla |
| 2026-05-09 | #8 | fix/consolsize-error-87 — Korjattu konsoli-ikkunan koon asetus (Windows virhekoodi 87) |
| 2026-05-09 | — | Save start time — Lähtöajan tallennus |
| 2026-05-09 | — | Revert HkCom32.cpp: remove lahdepistehaku block from tark_aika |
| 2026-05-08 | — | haelahdepiste + tark_aika — haelahdepiste-funktion linkitys TU-moduulien välille ja tark_aika-integraatio |
| 2026-04-20 | #5 | docs-import — Olemassa oleva dokumentaatio tuotu repositorioon |
| 2026-04-20 | — | Remove orphaned VTulostus_1.cpp experimental fork |
| 2026-04-20 | — | Remove FTP distribution feature |
| 2026-04-18 | — | Updated readme |
| 2026-04-17 | #3 | remove-secure-bridge-dependency — Secure Bridge -riippuvuus poistettu |
| 2026-04-17 | #2 | piikikuviofiksi — Korjattu piikkien sijainnit suurilla emit-koodeilla (>176) |
| 2026-04-16 | #1 | ci/github-actions-build — GitHub Actions CI -buildi lisätty Windows-käännökselle |
| 2026-04-16 | #1 | Show-all-controls-same-emit-code — Karttanäkymässä näytetään kaikki rastitunnukset joilla on sama emit-koodi, kun rasti puuttuu |
| 2026-04-14 | — | Update HkInitVanha.cpp |
| 2026-04-14 | — | scandinavian letters |
| 2026-04-14 | — | Visual studio 2026 & SecureBridge for RAD Studio 12 |
| 2026-04-14 | — | Update README.md |
| 2026-04-14 | — | Päivitetty CLAUDE.md vastaamaan nykytilannetta |
| 2026-04-14 | — | Päivitetty README vastaamaan nykytilannetta (VS2022, C++ Builder Community) |
| 2026-04-14 | — | Lisätty suomenkieliset kommentit koko koodipohjaan ja korjattu VS2022 yhteensopivuus |

Ennen tätä jaksoa historia jatkuu suoraan projektin alkuperäiseen 2016 vuoden pohjaan
(mm. `Initial commit`, 2016-10-12), noin kymmenen vuoden tauon takana.
