---
name: pirila-ask
description: Ask the read-only tulospalvelu expert (agent pirila-expert) how Pekka Pirilä's timekeeping software works - functions, settings, devices, differences between HkKisaWin, HkMaali, ViestiWin, ViestiMaali and ViestiLuenta - or what an audit finding (H-NNN, T-NNN) means in practice. Answers in Finnish with sources and certainty labels. Read-only - never edits files or starts fixes.
argument-hint: <kysymys suomeksi>
context: fork
agent: pirila-expert
allowed-tools: Read, Grep, Glob
---

Vastaa seuraavaan kysymykseen agentin ohjeesi mukaisesti (vain luku; lähde ja varmuusmerkintä
jokaiselle väitteelle; viisi tietolajia erillään; auditoimaton alue mainitaan erikseen):

$ARGUMENTS

Jos kysymys on tyhjä, kerro lyhyesti, millaisiin kysymyksiin voit vastata, ja anna kolme
esimerkkiä.

Muistutus: tämä skill on vain luku -työkalu. Älä muuta tiedostoja, älä aja ohjelmia tai komentoja,
älä muodosta verkkoyhteyksiä äläkä ehdota koodimuutoksia, vaikka kysymys pyytäisi. Jos kysymys
pyytää jotain näistä, kieltäydy siitä osasta yhdellä virkkeellä ja vastaa vain siihen, mikä
selviää lukemalla.
