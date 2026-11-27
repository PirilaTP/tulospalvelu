# Todisteiden anonymisointi / Anonymisation of the evidence

2026-11-27, ennen julkaisua (OP-1): auditoijan Windows-käyttäjänimi poistettiin todisteiden
poluista ja arkistoista. Korvaukset: käyttäjänimi → `kayttaja`, lyhyt 8.3-nimi → `KAYTTA~1`.
Mitään muuta sisältöä ei muutettu. Muutettuja tiedostoja on 42: C-11:n ajojen JSON- ja
tulostiedostot, C-4:n komento- ja skriptitiedostot, C-6:n lukkoskriptit, `k5-korjausohje.md` sekä
arkistot `c4/lokit/bcc32-outputs.json.gz`, `c4/lokit/msvc-warn-logs.tgz`, `c5/bda.tgz`, `c5/lea.tgz`,
`c5/ver.tgz`, `c5/vaihe2/lea.tgz`, `omf.tgz` ja `ver.tgz` (pakattu uudelleen; tiedostojen
omistajakentät tyhjennetty). Muuttuneiden tiedostojen ja arkistojen jäsenten tarkistussummat
päivitettiin manifesteihin (`SHA256SUMS`, `raaka-manifest.sha256`) samassa commitissa; alkuperäiset
summat ovat git-historiassa. `c6/SHA256SUMS`:n `README.md`-rivi ei täsmännyt jo ennen muutosta.

The auditor's Windows user name was removed from evidence paths and archives before publication
(user name → `kayttaja`, 8.3 short name → `KAYTTA~1`); nothing else changed. Manifest hashes of the
changed files and archive members were updated in the same commit; the original hashes remain in
the git history. Earlier commits still contain the name.
