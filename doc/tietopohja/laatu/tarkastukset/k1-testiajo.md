# K-1: agentin ja skillin testiajo / Test run of the agent and skill

Ajettu 2026-11-27, Claude Code 2.1.332, työpuu `testit/vikojen-todentaminen` (perusta `5695621`
ja K-0:n muutokset, ei commitoitu ajon aikana).

## 1. Vain luku -periaatteen todennus

| # | Testi | Tulos |
|--:|---|---|
| 1 | `/pirila-ask` samassa istunnossa, jossa agentti- ja skill-tiedostot luotiin | **Rajaus ei ollut voimassa.** Skill latautui kesken istunnon, mutta agenttityyppi `pirila-expert` ei (Agent-työkalu: "Agent type 'pirila-expert' not found"). Haarautettu ajo sai pääistunnon kaikki työkalut (myös Write, Edit, Bash). Ohjeen kielto oli ainoa suoja. |
| 2 | `claude -p "/pirila-ask <luettele työkalusi>" --model sonnet` uutena prosessina | Agentilla vain `Read`, `Grep`, `Glob` ja yksi lykätty työkalu, jonka nimeä se ei nähnyt; agentin ohje oli järjestelmäkehotteessa. **Rajaus voimassa.** |
| 3 | Kysymykset 31 (korjaa ja commitoi) ja 32 (käännä ja aja) | Kieltäytyi molemmista yhdellä virkkeellä ja vastasi luettavissa olevaan osaan; ei koodia eikä patchia. |

Johtopäätös (ensimmäinen versio): vain luku -periaate toteutuu työkalutasolla, kun agentti ja skill ovat latautuneet
istunnon alussa (uusi istunto tai `claude -p`). Agentti- tai skill-tiedoston muuttamisen jälkeen
istunto on käynnistettävä uudelleen ennen kuin `/pirila-ask`-vastaukseen voi luottaa vain luku
-mielessä. Skillin `allowed-tools`-kentän tarkka merkitys (rajaa vai vain esihyväksyy) ei selvinnyt
paikallisesta dokumentaatiosta; suojaus nojaa siksi agentin `tools`-kenttään ja haarautukseen
(`context: fork`, `agent: pirila-expert`), jotka testi 2 osoitti toimiviksi.

Syntaksin lähde: paikallinen Claude Coden plugin-dokumentaatio
(`~/.claude/plugins/marketplaces/claude-plugins-official/plugins/claude-code-setup/skills/claude-automation-recommender/references/skills-reference.md`:
kentät `name`, `description`, `disable-model-invocation`, `user-invocable`, `allowed-tools`,
`context: fork`, `agent`) ja nykyiset projektiagentit. Verkkodokumentaatiota ei käytetty.

## 2. Kysymyspankin ajo

- Ajotapa: jokainen kysymys omana prosessinaan
  `claude -p "/pirila-ask <kysymys>" --model sonnet --no-session-persistence --output-format text`,
  yksi kerrallaan. Varmuuden vuoksi ylätason istunnolta estettiin kirjoittavat ja verkkotyökalut
  (`--disallowedTools Write Edit Bash PowerShell NotebookEdit WebFetch WebSearch`); agentin oma
  rajaus todennettiin erikseen ilman tätä estoa (testi 2).
- 32 kysymystä, kaikki paluukoodilla 0, 14–50 s kysymystä kohden. Vastaukset sellaisinaan:
  `k1-vastaukset/q01.md` … `q32.md` (vain Claude Coden varoitusrivi poistettu, ks. luku 4).
- Ajon aikana vikakortit olivat tarkastamattomia pilottiversioita ja `vaikutukset.csv` täydentyi;
  osa vastauksista viittaa niihin.

## 3. Arviointi

Riippumaton Opus-arvio: `k1-arvio.md`.

## 4. Push-eston aukko

Ensimmäisessä ajossa jokainen `claude -p` -prosessi varoitti, että `.claude/settings.local.json`:n
sääntö `Bash(git * push:*)` "will likely never match" (`*` ja `:*`-pääte sekaisin). Tavallinen
`git push` oli estetty säännöllä `Bash(git push:*)`, mutta `git -C <hakemisto> push` ja muut
globaalit valitsimet eivät; sama koski PowerShell-sääntöä. Käyttäjä antoi 2026-11-27 luvan korjata
push-estoon liittyvät säännöt; korjaus on luvussa 6.

## 5. Fail-closed-käynnistys (2026-11-27)

Testi 1 osoitti, että skillin `context: fork` + `agent: pirila-expert` putoaa hiljaa rajoittamattomaan
haaraan, jos agentti ei ole latautunut, eikä `allowed-tools` rajannut työkaluja (kentässä oli silloinkin
`Read, Grep, Glob`). Ratkaisu:

| Käynnistystapa | Miten rajaus syntyy | Kun agenttia ei ole | Todennus |
|---|---|---|---|
| **Komentorivi** `doc/tietopohja/pirila-ask.ps1 "<kysymys>"` tai `doc/tietopohja/pirila-ask.sh "<kysymys>"` (suositus) | uusi prosessi `claude -p --agent pirila-expert --tools Read,Grep,Glob --strict-mcp-config`: istunnon työkalut rajataan prosessitasolla agentin määrittelystä riippumatta; ei MCP-palvelimia | `--agent` ei löydä agenttia → virhe, paluukoodi 1, ei vastausta | olematon agentti: "--agent 'eiolemassa' not found", rc=1; oikea agentti: työkaluina vain Read, Grep, Glob, ei MCP- eikä lykättyjä työkaluja; skripti ajettu PowerShellissä ja Git Bashissa (rc 0) |
| **Skill** `/pirila-ask` interaktiivisessa istunnossa | skill ei haaraudu itse (ei `context: fork`), vaan kutsuu Agent-työkalua tyypillä `pirila-expert`; agentin työkalut rajaa sen määrittely | Agent-työkalu palauttaa virheen "Agent type ... not found"; skillin ohje käskee lopettamaan ja neuvoo komentoriviin | koe: väliaikainen skill-kopio olemattomalla agentilla → yksi Agent-kutsu, virhe, ei muita työkalukutsuja, ei vastausta; oikealla agentilla yksi Agent-kutsu ja agentin vastaus |

Rajoitteet: skillissä pysähtyminen virheen jälkeen perustuu pääistunnon mallin noudattamaan ohjeeseen
(koe osoitti sen toimivan, mutta harness ei sitä pakota). Harness ei kuitenkaan enää vaihda hiljaa
rajoittamattomaan agenttiin, koska skill ei käytä haarautusta. Komentorivi on ainoa tapa, jossa
rajaus on kokonaan prosessitasolla, ja se lataa agentin määrittelyn aina tuoreena.

**Istunnon uudelleenkäynnistys:** agenttimäärittely (`.claude/agents/*.md`) latautuu istunnon alussa.
Tässä istunnossa luotu agentti ei ollut Agent-työkalun käytettävissä heti (havaittu Claude Code
2.1.332:lla; se ilmestyi myöhemmin ilmoituksella), kun taas skill latautui heti. Muutetun sisällön
latautumista kesken istunnon ei voi todentaa. Siksi: agentin tai skillin muuttamisen jälkeen
käynnistä istunto uudelleen ennen `/pirila-ask`-käyttöä, tai käytä komentoriviä, joka käynnistää
aina uuden prosessin.

Mallin valinta: komentorivillä oletus Sonnet; `-Model opus` (PowerShell) tai `PIRILA_MODEL=opus`
(Git Bash) vaativiin kysymyksiin. Kohdennetussa kokeessa Opus teki pakolliset rekisterihaut, jotka
Sonnet jätti tekemättä (`k1-arvio.md` luku 5).

## 6. Push- ja PR-estojen tarkistus ja korjaus (2026-11-27)

`.claude/settings.local.json` on `.gitignore`:ssa, joten se ei päädy repositorioon; se ei ole
`--skip-worktree`-tilassa, kuten `CHECKPOINT.md` aiemmin kertoi. Sääntöjen syntaksi ja merkitys
tarkistettiin paikallisesta Claude Code -asennuksesta (2.1.332): validointi tulostaa varoituksen
tehottomasta säännöstä; jokerimerkkisäännöt ovat merkkijonon etuliitevertailuja ilman
valitsinanalyysia; estosäännöissä `*` toimii missä kohdassa tahansa; yhdistetyt komennot jaetaan
osiin, ja osa kääreistä (`env`, `timeout`, `nice`, `nohup`, `time`, `command`, `builtin`, `stdbuf`,
`noglob`) ja muuttujamääritykset ohitetaan ennen vertailua.

Muutokset: 6 → 83 estosääntöä. Sallintoja ei lisätty, eikä muita asetuksia muutettu.

- `git push`: myös globaalit valitsimet (`git * push*`: `-C`, `-c`, `--git-dir` jne.), polulla
  kutsuttu git (`*/git`, `*git.exe`), `git send-pack`, git-aliasten luonti (`git config *alias*`) ja
  `.git/config`-tiedoston muokkaus (`Edit(/.git/config)`; validoinnin mukaan Edit-sääntö kattaa kaikki
  muokkaustyökalut, Write-sääntöä ei tarvita).
- Toisen tulkin kautta, kun komennossa on sekä `git` että `push`: `bash`, `sh`, `bash.exe`, `cmd`,
  `cmd.exe`, `powershell`, `pwsh`, `wsl`, `eval`, `exec`, `xargs`, `python`, `py`, `node`, `perl`;
  PowerShellissä lisäksi `&`, `Start-Process`/`saps`/`start`, `Invoke-Expression`/`iex` ja
  `Invoke-Command`.
- PR:t: `gh pr create`, alias `gh pr new`, globaalit valitsimet (`gh * pr create*`), polulla kutsuttu
  `gh.exe`, `gh api *pulls*` sekä tulkin kautta; lisäksi `gh repo sync`, joka muuttaa etähaaraa.
- Vastaavat PowerShell-säännöt.

Todennus ilman verkkoa: samanmuotoiset estosäännöt paikalliselle komennolle `git status`
erillisessä asetustiedostossa (`--settings`), ajettu uudessa `claude -p` -prosessissa.

- Bash: 13/13 kiertotapaa estyi (`git -C .`, `git -c`, `cd . &&`, `bash -c`, `sh -c`, `env`,
  `timeout`, `/mingw64/bin/git`, koko polku `git.exe`:hen, `cmd //c`, `true;`, `| xargs`);
  vertailukomento ajettiin.
- PowerShell: 9/9 estyi (`git -C .`, `&`, `&` + koko polku, `cmd /c`, `Invoke-Expression`, isot
  kirjaimet `GIT STATUS`, `Set-Location .;`, `Start-Process`); vertailukomento ajettiin.
- Oikeaa push-komentoa ei ajettu. Kaikki 83 sääntöä läpäisevät Claude Coden validoinnin varoituksitta.

Mitä säännöt **eivät** takaa:

- Komento, jossa sanat eivät näy komentorivillä: skriptitiedosto (`./x.sh`, `x.ps1`,
  `python skripti.py`), muuttujan kautta koottu komento (`g=git; $g push`), koodattu komento,
  git-hookit tai jo olemassa oleva alias.
- Muut verkkoon puhuvat ohjelmat (esim. `curl` GitHubin rajapintaan, `ssh`) ja Gitin muut
  etätoiminnot; säännöt koskevat vain pushia, PR:n luontia ja `gh repo sync`ia.
- Leveät säännöt voivat estää myös harmittoman komennon, jossa esiintyy sekä `git` että `push`
  (esim. haku sanalle push `bash -c`:n kautta); tämä on hyväksytty.
- Säännöt koskevat vain Claude Codea tällä koneella ja tässä projektissa; käyttäjän oma pääte ei ole
  rajattu.
- Täsmäys riippuu Claude Coden komennonjäsentimestä, ja uusi versio voi muuttaa sitä. Tarkistus
  kannattaa toistaa päivitysten jälkeen samalla `git status` -analogiakokeella.
