# AGENTS.md

Instructions for AI coding agents (Claude Code, Codex and others) working in this repository.
Human contributors: see `CONTRIBUTING.md`. Build details: `CLAUDE.md` (sections "Build
instructions" and "Architecture"). Published version of the knowledge base:
https://pirilatp.github.io/tulospalvelu/ (sections "Kehittäjille" and "Ohjelmiston laatu").

## Project in one paragraph

Pekka Pirilä's sports timekeeping suite (GPLv3, since 1986): five programs from one C++ code base
in `TPsource/V52/` - individual competition HkKisaWin (GUI) and HkMaali (console), relay ViestiWin
(GUI), ViestiMaali and ViestiLuenta (console). Console programs build with MSVC, GUI programs with
the classic C++Builder `bcc32` compiler. UI, identifiers and most comments are Finnish. An
architecture audit (phases C and K) produced registers, reports, defect cards, a functional
handbook and a portal in `doc/`.

## Hard rules

1. **Encodings.** Source files mix cp1252, UTF-8 and ASCII, and the two compilers read UTF-8
   differently. Never change a file's encoding, BOM or line endings. Read
   `doc/arkkitehtuuri/merkistot.md` before editing any source file.
2. **Language.** New code comments in English, ASCII only. Do not translate or rewrite Pekka
   Pirilä's original comments. Architecture documents in `doc/arkkitehtuuri/` are bilingual in
   the same file (Finnish paragraph first, English after). User manuals in `docs-site/docs/` are
   Finnish.
3. **Production code is not changed as a side effect.** Documentation, audit and knowledge-base
   work does not touch `TPsource/`, project files (`.sln`, `.vcxproj`, `.cbproj`) or the release
   process. Do not build or run the production programs unless the task says so.
4. **No publishing by agents.** Do not push, open or merge pull requests, create GitHub issues or
   deploy the site unless a human explicitly asks for that specific action.
5. **Security.** The audit documents describe unfixed defects, some reachable from the network.
   Describe defects and fixes; never write exploitation instructions or attack tooling.
6. **One canonical source.** Audit facts live in the registers. A card, a handbook chapter or a
   site page never corrects a register; new information goes into the register first.

## Where to find what

| Question | Canonical source | Then |
|---|---|---|
| What does the program do now? | the code in `TPsource/V52/` | `doc/tietopohja/kasikirja/` (functional handbook, `toiminnot.csv`) |
| Architecture, modules, data flow | `doc/arkkitehtuuri/README.md`, `yleiskuva.md` | the topic report below |
| File formats, records | `doc/arkkitehtuuri/tiedostomuodot.md` | `tietorakenteet-ja-tila.md` |
| Workstation protocol | `doc/arkkitehtuuri/tiedonsiirto.md` | `Tp/TpComY.cpp`, `Hk/HkCom32.cpp`, `Juk/VCom.cpp` |
| Threads and locks | `doc/arkkitehtuuri/saikeet-ja-lukot.md` | `globaali-tila.md` |
| Devices, SQL, external dependencies | `doc/arkkitehtuuri/riippuvuudet.md` | handbook chapter 10 |
| Settings, command line, compile switches | `doc/arkkitehtuuri/rekisteri/asetusavaimet.csv`, `valitsimet.csv` | `ominaisuusvalitsimet.md` |
| A defect H-NNN / technical debt T-NNN | `doc/arkkitehtuuri/rekisteri/havainnot.md`, `riskiarvio.csv`, `tekninen-velka.csv`, `vaikutukset.csv` | card `doc/tietopohja/vikakortit/H/H-NNN.md` |
| Fix plan | `rekisteri/korjauskokonaisuudet.csv`, `korjaussuunnitelma.md` | `regressiotestaussuunnitelma.md` |
| Owner decisions, open questions | `omistajan-paatokset.md` (OP), `seuranta.md` (Q) | |
| What has been reviewed | `rekisteri/tiedostot.csv`, `todisteet/c14/lahtotaso/`, `tyokalut/kattavuus.py` | `c14-suunnitelma.md` |
| Phase state | `rekisteri/vaiheet.csv` | `doc/arkkitehtuuri/CHECKPOINT.md` (resume notes) |

Paths of code references in the audit material are relative to `TPsource/V52/` and refer to the
revision in the register column Rev (mostly `f7fe553`).

## Keep five kinds of knowledge apart

When you answer or write, say which of these a statement is, and never present one as another:
A. behaviour of the current code (read now); B. behaviour at audit time (audit documents, with
revision); C. audit finding (register id; `tila` = `avoin` means not fixed and usually not shown by
running the program); D. fix made later (`tila` = `korjattu`, column `korjaus`); E. planned change
(fix plan, owner decisions; never describe as done). The audit did not review the whole code base:
the absence of a finding is not evidence that a function is correct
(`laatu/kattavuus` on the site, `doc/arkkitehtuuri/auditoinnin-yhteenveto.md`).

## Read-only expert

`.claude/agents/pirila-expert.md` is a read-only expert prompt for questions about how the software
works and what a finding means. In Claude Code: `/pirila-ask <kysymys>` (skill) or
`doc/tietopohja/pirila-ask.ps1` / `.sh` (fresh process limited to Read, Grep, Glob; fail-closed).
Other tools: use the body of that file as the system prompt and give the session read-only file
access only. Its answers can still contain wrong details; check the cited sources.

## Maintaining the knowledge base

Run from the repository root (Python 3, no packages needed; Node only for the portal test):

```
python -I doc/arkkitehtuuri/tyokalut/tarkista_auditointi.py .           # register structure and references
python -I doc/arkkitehtuuri/tyokalut/synteesi.py . <tmp_dir> --tarkista  # risk register and fix packages
python -I doc/arkkitehtuuri/tyokalut/viitteet.py .                       # references against their revision
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . tarkista            # defect cards and handbook
python -I doc/arkkitehtuuri/tyokalut/portaali.py . --tarkista            # local portal data up to date
node doc/arkkitehtuuri/tyokalut/portaali_testi.js                        # portal smoke test
python -I doc/arkkitehtuuri/tyokalut/sivusto.py . --tarkista             # site mapping files and links
```

New finding or a fix: register row first (`havainnot.md`, `riskiarvio.csv` `tila`/`korjaus`,
`vaikutukset.csv` via `tietopohja.py . tuo`), then `tietopohja.py . runko|lohkot|kuittaa`, then
`tietopohja.py . kasikirja` and `kooste`. The site and the portal regenerate themselves from these
on the next build; never edit generated pages. Full procedure:
`docs-site/docs/kehittajat/auditointi.md`.

## Pull request review with the knowledge base

For a change to `TPsource/`:
1. List the findings whose references touch the changed files: search `rekisteri/havainnot.md` for
   the changed paths (relative to `TPsource/V52/`), and the handbook functions in
   `doc/tietopohja/kasikirja/toiminnot.csv`.
2. Check that the PR does not silently change behaviour described in the handbook, and that a fix
   claims only findings it actually fixes; the register rows must be updated in the same PR
   (`tila` = `korjattu`, `korjaus` = commit, version, test).
3. Use the regression test plan (`regressiotestaussuunnitelma.md`) and the fix package's `testit`.
4. Check encoding, BOM and line endings of every changed source file (`merkistot.md`).
5. Report findings with file and line; do not post to GitHub without a human's approval.

## Agent definitions

`.claude/agents/` holds the audit agents (architecture, build, defect and concurrency, encoding,
documentation, independent verifier, expert). They are read-only except `doc-writer`, which
writes only in `doc/arkkitehtuuri/`. Personal settings (`.claude/settings.local.json`) are never
shared. Model choice used in the audit: Sonnet for inventories and writing, Opus for hard technical
interpretation and independent review.
