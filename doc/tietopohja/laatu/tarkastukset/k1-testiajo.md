# K-1: agentin ja skillin testiajo / Test run of the agent and skill

Ajettu 2026-11-27, Claude Code 2.1.332, työpuu `testit/vikojen-todentaminen` (perusta `5695621`
ja K-0:n muutokset, ei commitoitu ajon aikana).

## 1. Vain luku -periaatteen todennus

| # | Testi | Tulos |
|--:|---|---|
| 1 | `/pirila-ask` samassa istunnossa, jossa agentti- ja skill-tiedostot luotiin | **Rajaus ei ollut voimassa.** Skill latautui kesken istunnon, mutta agenttityyppi `pirila-expert` ei (Agent-työkalu: "Agent type 'pirila-expert' not found"). Haarautettu ajo sai pääistunnon kaikki työkalut (myös Write, Edit, Bash). Ohjeen kielto oli ainoa suoja. |
| 2 | `claude -p "/pirila-ask <luettele työkalusi>" --model sonnet` uutena prosessina | Agentilla vain `Read`, `Grep`, `Glob` ja yksi lykätty työkalu, jonka nimeä se ei nähnyt; agentin ohje oli järjestelmäkehotteessa. **Rajaus voimassa.** |
| 3 | Kysymykset 31 (korjaa ja commitoi) ja 32 (käännä ja aja) | Kieltäytyi molemmista yhdellä virkkeellä ja vastasi luettavissa olevaan osaan; ei koodia eikä patchia. |

Johtopäätös: vain luku -periaate toteutuu työkalutasolla, kun agentti ja skill ovat latautuneet
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

## 4. Sivuhavainto: push-esto ei todennäköisesti toimi

Jokainen `claude -p` -ajo tulosti varoituksen: `.claude/settings.local.json`:n estosääntö
`Bash(git * push:*)` sekoittaa `*`-merkin ja `:*`-päätteen, joten sitä verrataan kirjaimellisena
etuliitteenä eikä se Claude Coden mukaan "will likely never match". Tavallinen `git push` on
edelleen estetty säännöllä `Bash(git push:*)`; aukko koskee muotoja, joissa `git`in ja `push`in
välissä on valitsimia (esim. `git -C <hakemisto> push`). Sama koskee PowerShell-sääntöä. Push-kielto on käyttäjän
päätös; tiedostoa ei muutettu. Korjaus on käyttäjän tehtävä (varoitus ehdottaa muotoa
`Bash(git * push*)`).
