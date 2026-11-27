---
name: pirila-ask
description: Ask the read-only tulospalvelu expert (agent pirila-expert) how Pekka Pirilä's timekeeping software works - functions, settings, devices, differences between HkKisaWin, HkMaali, ViestiWin, ViestiMaali and ViestiLuenta - or what an audit finding (H-NNN, T-NNN) means in practice. Answers in Finnish with sources and certainty labels. Read-only; refuses to answer if the restricted agent is not available.
argument-hint: <kysymys suomeksi>
---

Tämä skill välittää kysymyksen vain luku -agentille `pirila-expert`. Se toimii fail-closed:
jos rajattua agenttia ei voi käynnistää, kysymykseen ei vastata.

Kysymys:

$ARGUMENTS

Tee täsmälleen näin:

1. Kutsu Agent-työkalua kerran: `subagent_type: "pirila-expert"`, `model: "sonnet"`,
   `description: "pirila-ask"`, ja `prompt` = yllä oleva kysymys sellaisenaan (jos se on tyhjä:
   "Kerro lyhyesti, millaisiin kysymyksiin voit vastata, ja anna kolme esimerkkiä.").
   Älä käytä mitään muuta työkalua: älä lue tiedostoja itse, älä aja komentoja, älä hae verkosta.
2. Jos Agent-kutsu epäonnistuu (esimerkiksi "Agent type 'pirila-expert' not found") tai
   Agent-työkalua ei ole, **lopeta**. Älä vastaa kysymykseen itse äläkä käytä toista agenttia.
   Kerro käyttäjälle vain tämä:
   "pirila-ask: rajattua agenttia pirila-expert ei voitu käynnistää, joten kysymykseen ei
   vastattu. Agentti latautuu istunnon alussa: käynnistä Claude Code uudelleen, tai aja kysymys
   komentoriviltä `doc/tietopohja/pirila-ask.ps1 "<kysymys>"` (PowerShell) tai
   `doc/tietopohja/pirila-ask.sh "<kysymys>"` (Git Bash)."
3. Muuten palauta agentin vastaus käyttäjälle sellaisenaan, ilman omia lisäyksiä, tulkintoja tai
   korjausehdotuksia.
