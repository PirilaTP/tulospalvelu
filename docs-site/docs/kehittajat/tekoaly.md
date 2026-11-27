# Tekoälyavusteinen kehitys

Auditointi ja tietopohja on tehty tekoälyagenttien avulla, ja samat välineet ovat kaikkien
kehittäjien käytettävissä. Ne toimivat Claude Codella sellaisinaan; tietopohja on tavallisia
Markdown- ja CSV-tiedostoja, joten Codex ja muut työkalut voivat käyttää sitä suoraan.

## Säännöt kaikille työkaluille

Yhteiset säännöt ovat repositorion juuren tiedostossa
[`AGENTS.md`](https://github.com/PirilaTP/tulospalvelu/blob/main/AGENTS.md): merkistöt, kommenttien
kieli, kanoninen tietolähde, tietopohjan kartta, ylläpitokomennot ja PR-katselmoinnin
tarkistuslista. Claude Code lukee sen `CLAUDE.md`:n kautta, Codex ja useimmat muut työkalut
suoraan. Tietopohjaa ei kopioida ohjetiedostoihin: ohje kertoo, mistä tieto löytyy.

## Asiantuntija: `pirila-ask`

Vain luku -asiantuntija vastaa kysymyksiin ohjelmien toiminnasta, asetuksista, laitteista ja
auditointihavaintojen merkityksestä. Se vastaa suomeksi, merkitsee jokaisen väitteen lähteen ja
varmuuden, erottaa auditoinnin aikaisen tiedon nykyisestä koodista ja kertoo, kun aluetta ei ole
auditoitu. Se ei muuta tiedostoja, aja ohjelmia eikä ehdota koodimuutoksia.

```
/pirila-ask Mitä H-071 tarkoittaa kilpailun järjestäjälle?        # Claude Code -istunnossa
doc/tietopohja/pirila-ask.ps1 "Miten emiTag-lukija palautuu katkon jälkeen?"   # PowerShell
doc/tietopohja/pirila-ask.sh "Mitä tapahtuu vaiheen vaihdossa?"              # Git Bash
```

Komentorivikäynnistimet ajavat uuden `claude`-prosessin, jonka työkalut on rajattu lukemiseen
(Read, Grep, Glob) eikä sillä ole MCP-palvelimia. Jos agenttia ei löydy, kysymykseen ei vastata
(fail-closed). Vaativiin kysymyksiin: `-Model opus` (PowerShell) tai `PIRILA_MODEL=opus` (Bash).
Vastauksissa voi silti olla vääriä yksityiskohtia; tarkista lähteet ennen päätöksiä.

Muilla työkaluilla: käytä tiedoston `.claude/agents/pirila-expert.md` runkoa järjestelmäkehotteena ja
anna istunnolle vain lukuoikeus repositorioon.

Portaalin vikakortilla on painike, joka kopioi valmiin `/pirila-ask`-kysymyksen.

## Agentit

Auditoinnin agentit (`.claude/agents/`). Taulukko on luettu määrittelytiedostoista:

<!-- tp:agentit -->

Kaikki muut paitsi `doc-writer` on ohjeistettu toimimaan vain lukien, ja `doc-writer` kirjoittaa vain
hakemistoon `doc/arkkitehtuuri/`. Bash-työkalun saaneilla agenteilla tämä on ohje eikä tekninen
rajoitus; vain `pirila-expert` on rajattu työkaluluettelollaan (Read, Grep, Glob). Auditoinnissa käytettiin Sonnetia inventointiin ja kirjoittamiseen
sekä Opusta vaikeaan tekniseen tulkintaan ja riippumattomaan tarkastukseen.

## Skillit

<!-- tp:skillit -->

## Käyttöönotto omalla koneella

1. Asenna [Claude Code](https://docs.claude.com/en/docs/claude-code) ja kirjaudu.
2. Kloonaa repositorio ja käynnistä `claude` repositorion juuressa. Agentit ja skillit latautuvat
   hakemistosta `.claude/` istunnon alussa; muutokset niihin tulevat voimaan vasta uudessa
   istunnossa (komentorivikäynnistimet lukevat määrittelyn aina tuoreena).
3. Kokeile: `doc/tietopohja/pirila-ask.ps1 "Kerro lyhyesti, mihin voit vastata."`
4. Omat käyttöoikeusasetukset kuuluvat tiedostoon `.claude/settings.local.json`, joka ei ole
   versionhallinnassa. Henkilökohtaisia asetuksia ei jaeta repositoriossa.

Tietopohjan ylläpitoon tarvitaan vain Python 3; agentteja ei tarvita. Työkalut:

<!-- tp:tyokalut -->
