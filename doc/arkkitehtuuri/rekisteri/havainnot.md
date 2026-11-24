# Havainnot / Findings

Kentät ja arvot: `README.md`. Polut suhteessa `TPsource/V52/`.
Fields and values: `README.md`. Paths relative to `TPsource/V52/`.

## Viat / Defects (H)

| ID | Havainto / Finding | Näyttö / Evidence | Vakavuus / Severity | Varmuus / Confidence | Todennus / Verification | Rev | Lähde / Source |
|---|---|---|---|---|---|---|---|
| H-001 | `memset(stsk, 0, sizeof(st))` kirjoittaa 120 tavua 40-tavuisen puskurin yli / writes 120 bytes past a 40-byte buffer | `Juk/VTulostus.cpp:2228`, `:3981`; MSVC C4789 | korkea / high | [todennettu] | ei tarkastettu / not reviewed | f7fe553 | issue-drafts/01 |
| H-002 | `sscanf("%s")` kirjoittaa `int`-arvon osoittamaan paikkaan / writes through an int value | `Juk/vSarjat.cpp:377`, `:400`; `Juk/VDef.h:399` `int paikat[]`; C4477 | korkea / high | [todennettu] koodi; [päätelty] milloin laukeaa / when it triggers | ei tarkastettu | f7fe553 | issue-drafts/02 |
| H-003 | 16 muotoilumerkkijonon ja argumenttien ristiriitaa; 2 voi kaataa / 16 format mismatches; 2 can crash | `Juk/VTulostus.cpp:352`, `Juk/VIx.cpp:1198` (kaatuminen / crash); muut / others issue-drafts/03; C4477/C4474/C4778 | keskitaso / medium | [todennettu] | ei tarkastettu | f7fe553 | issue-drafts/03 |
| H-004 | U+FFFD käyttäjälle näkyvissä teksteissä / in UI strings | `Hk/HkAjat.cpp:234`, `Juk/vajat.cpp:191`; syntyi / introduced `5a65c64` | matala / low | [todennettu] (tavut / bytes) | ei tarkastettu | f7fe553 | issue-drafts/04 |
| H-005 | UTF-8-teksti väärin MSVC-käännöksessä / UTF-8 text wrong in MSVC build | `Tp/TpLaitteet.cpp:2581`; `HkMaali.exe` sisältää / contains `00C3 00A4` | matala / low | [todennettu] | vahvistettu ajamalla / confirmed (binäärit / binaries) | f7fe553 | issue-drafts/04 |
| H-006 | `Tests/run.sh` rikki (puuttuva `\`, `-I Hk`); testejä ei ajeta CI:ssä / broken; tests not in CI | `Tests/run.sh:14`; merge `6107c8f`; `.github/workflows/build.yml` | matala / low | [todennettu] koodi; [päätelty] skriptin ajo / script run | ei tarkastettu | f7fe553 | issue-drafts/05 |
| H-007 | `tark_kilp` lukee tietueen ennen `tall_CS`-lukkoa → paikallinen muutos kumoutuu ja leviää vertaisille / reads record before lock → local change reverted and propagated | `Hk/HkCom32.cpp:260`, `:326-332`; `Hk/HkDat.cpp:964`, `:1211` | korkea / high | [todennettu] mekanismi; [päätelty] ikkunan pituus / window length | tarkastettu / reviewed | f7fe553 | `../tallenna-samanaikaisuus.md` |
| H-008 | `tall_sak` palaa lukkoa vapauttamatta; laukeaa virheellisestä laitesanomasta tai sarjasta, jossa `lsak == 0` / returns holding `tall_CS` | `Hk/HkSak.cpp:89`, `:99`; `Tp/TpLaitteet.cpp:1670`; `Hk/HkCom32.cpp:647` | korkea / high | [todennettu] koodi; [päätelty] jumiutuminen / hang | tarkastettu | f7fe553 | sama / same |
| H-009 | Lukko pidossa `SendMessage`-kutsun ajan (`add_sakko`, `writeerror_w`) → lukkiutumisvaara / lock held across SendMessage → deadlock risk | `Hk/HkDat.cpp:944`, `:1061`, `:1077`, `:1250-1251`, `:1377`; `cbHk/UnitASakot.cpp:269-273`; `cbTp/cbTpApu.cpp:467-490` | korkea / high | [todennettu] mekanismi; [päätelty] lukkiutuminen | tarkastettu | f7fe553 | sama |
| H-010 | ~35 lukitsematonta `tallenna`-kutsua; muokattavilla rakenteilla ei omaa lukkoa / unlocked call sites, unlocked structures | `../tallenna-samanaikaisuus.md` "Kutsupaikat"; `Hk/HkIx.cpp:800-1512`; `tputilv2/dbindex.cpp:51-106`; `Hk/HkEmit.cpp:971-1154` | keskitaso / medium | [todennettu] lukkotila; [päätelty] kilpa / race | tarkastettu osittain / partly (yksi alaväite kumottu / one sub-claim refuted) | f7fe553 | sama |
| H-011 | `tallenna` kirjoittaa emit-tiedostoon ilman `emit_CS`:ää / writes emit file without `emit_CS` | `Hk/HkDat.cpp:1114`, `:1146`, `:1169`; `Hk/HkEmit.cpp:1600-1623` | keskitaso | [todennettu] | tarkastettu | f7fe553 | sama |
| H-012 | `vaihdakilp` palaa lukkoa vapauttamatta (konsoli) / returns holding `tall_CS` (console) | `Hk/HkEmit.cpp:3390`, `:3405` | keskitaso | [todennettu] | tarkastettu | f7fe553 | sama |
| H-013 | `teejarj`: silmukkamuuttujavirhe, tietue 1 tallennetaan toistuvasti, muita ei / loop variable bug | `Hk/HkIx.cpp:1834` | keskitaso | [todennettu] | tarkastettu | f7fe553 | sama |
| H-014 | MySQL-kirjoitus pysähtyy vaiheen vaihdon jälkeen; epäonnistuneet ja poistot katoavat; ei poikkeuskäsittelyä / SQL writer stops after stage change; failures and deletes lost | `cbHk/UnitMySQL.cpp:637`, `:651-654`, `:699-701`, `:748-751`; `Hk/HkInit.cpp:3740` | korkea SQL-käytössä / high with SQL | [todennettu] | tarkastettu | f7fe553 | sama |
| H-015 | `KILP.DAT`-kirjoitusvirhe ohitetaan; virheikkuna lukon ollessa varattuna / write error ignored; dialog under lock | `dbboxm/dbbox1.cpp:64-100`; `tputilv2/perr.cpp:51-58`; `Hk/HkDat.cpp:586-590` | keskitaso | [todennettu] | tarkastettu | f7fe553 | sama |
| H-016 | Replikoinnissa ei versiota; paketit sovelletaan tietuenumerolla ilman kilpailijanumeron tarkistusta / no versioning; packets applied by record number without bib check | `Hk/HkDef.h:1235-1280`; `Hk/HkCom32.cpp:270-331`, `:830-841` | keskitaso | [todennettu] | tarkastettu | f7fe553 | sama |
| H-017 | Lomakkeiden pitkään pidetyt tietuekopiot kumoavat välissä tulleet muutokset / long-held form copies overwrite intervening changes | `cbHk/UnitEmit.cpp:3483`; `cbHk/UnitKilpailijatiedot.cpp:613-626` | keskitaso | [päätelty] | ei tarkastettu / not reviewed | f7fe553 | sama |
| H-018 | VCL-käsittely työsäikeistä / VCL access from worker threads | `cbHk/WinHk.cpp:1623-1625`; `cbHk/UnitMySQL.cpp:655`, `:721-722` | keskitaso | [todennettu] esiintyy; [päätelty] vaikutus / effect | tarkastettu | f7fe553 | sama |
| H-019 | `writeerror_w` käyttää globaalia puskuria ilman lukkoa / global buffer without lock | `cbTp/cbTpApu.cpp:470-489` | matala / low | [päätelty] | ei tarkastettu | f7fe553 | sama |
| H-020 | `add_sakko` määritelty kahdesti samassa ohjelmassa / defined twice in one program | `cbHk/UnitASakot.cpp:269`, `cbHk/UnitASuunn.cpp:289` | matala | [tuntematon] mikä linkitetään / which is linked | ei todennettavissa staattisesti / not statically verifiable | f7fe553 | sama |
| H-021 | Muistipuskuritilassa täysi lähtöjono pudottaa paketin / memory-queue mode drops packets when full | `Tp/TpComY.cpp:520-527` | matala–keskitaso | [todennettu] | tarkastettu | f7fe553 | sama |

## Tekninen velka / Technical debt (T)

| ID | Havainto / Finding | Näyttö / Evidence | Vakavuus / Severity | Varmuus / Confidence | Rev |
|---|---|---|---|---|---|
| T-001 | Ydin ottaa mukaan GUI-otsikot / core includes GUI headers | `Hk/HkDeclare.h:23-28`, `Juk/VDeclare.h:26-27` | keskitaso / medium | [todennettu] | f7fe553 |
| T-002 | Kehäriippuvuudet / dependency cycles `Hk↔Tp`, `Juk↔Tp`, `cbTp↔cbHk/ViestiWin` | `../riippuvuudet.md` | keskitaso / medium | [todennettu] | f7fe553 |
| T-003 | Noin 850 `extern`-globaalia / about 850 extern globals | `Hk/HkDeclare.h` 453, `Juk/VDeclare.h` 396 | keskitaso / medium | [todennettu] | f7fe553 |
| T-004 | Sekoitetut merkistöt, 2 tiedostoa sekoitettuja / mixed encodings, 2 mixed files | `../merkistot.md`; `tputilv2/aakjarjws.cpp`, `aakjarjws2.cpp` | keskitaso / medium | [todennettu] | f7fe553 |
| T-005 | Indy- ja ODBC-riippuvuudet ilman käyttöä / unused Indy and ODBC dependencies | `RADStudio10/*.cbproj`, `VS/V/JukMaali520.vcxproj` | matala / low | [todennettu] haku; [päätelty] turha / unnecessary | f7fe553 |
| T-006 | JukLuenta linkittää `vc10/`-hakemistoon / links against `vc10/` | `VS/V/JukLuenta520.vcxproj` | matala / low | [todennettu] | f7fe553 |
| T-007 | 32 orpoa tiedostoa / orphan files | `../inventaario.md` | matala / low | [päätelty] | f7fe553 |
| T-008 | Konsoli- ja GUI-versioissa eri ominaisuusvalitsimet / different feature switches | `../inventaario.md` | keskitaso / medium | [todennettu] valitsimet; [tuntematon] vaikutus / effect | f7fe553 |
| T-009 | `makitp maki;` luokan ulkopuolella `#ifdef MAKI`-lohkossa / outside a class | `Hk/HkDef.h` (kilppvtp:n jälkeen / after kilppvtp) | matala / low | [todennettu]; ei käännetä / not compiled | f7fe553 |
| T-010 | CLAUDE.md:n vanhentuneet tiedot (korjattu `c273b9f`) / outdated CLAUDE.md (fixed) | `c273b9f` | matala / low | [todennettu] | c273b9f |
