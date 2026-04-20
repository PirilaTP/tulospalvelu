# Tulosten automaattinen siirto palvelimelle rsync + SSH -yhdistelmällä (Windows / WSL)

Tämä ohje kuvaa, miten HkKisaWin / HkMaali -ohjelman **automaattinen
tiedostotulostus** (luku 9.7) yhdistetään modernin `rsync`-työkalun kanssa,
joka siirtää HTML-tulokset omalle www-palvelimelle SSH:n yli. Ohje korvaa
[Liite 10](https://www.tulospalvelu.fi/pirila/ohjeet/Help/index.htm?page=Liite_10._Komentotiedoston_automaattinen_suorittaminen.htm)
mukaiset vanhat `ftp`- ja `sftp2`-ratkaisut.

## Miksi rsync?

- Siirtää vain muuttuneet tiedostot → nopea ja verkkoa säästävä
- Atominen vaihto (kirjoittaa ensin väliaikatiedoston ja nimeää sen sitten
  oikeaksi) → katsojat eivät näe puolivalmista HTML:ää
- Toimii SSH:n yli julkisen avaimen tunnistuksella, ei salasanaa
- Saatavilla kaikissa Linux-palvelimissa oletuksena
- `sftp2` ja tavallinen `ftp` ovat käytännössä poistuneet käytöstä

## Esivaatimukset

- Windows 10/11 (22H2 tai uudempi)
- WSL2 + Ubuntu (tai muu jakelu) — asennus: `wsl --install` PowerShellissä
  järjestelmänvalvojana
- SSH-pääsy kohdepalvelimelle (esim. `user@kisapalvelin.fi`)
- Hakemisto palvelimella, johon käyttäjällä on kirjoitusoikeus
  (esim. `/var/www/html/kisa/`)

## 1. SSH-avainparin luonti WSL:ssä

Avaa WSL-terminaali (komento `wsl` komentoriviltä tai Ubuntu Start-valikosta).
Luo avain ilman salafraasia, jotta tulospalveluohjelma pystyy ajamaan siirron
automaattisesti:

```bash
ssh-keygen -t ed25519 -N "" -f ~/.ssh/id_ed25519_kisa
```

Kopioi julkinen avain palvelimelle:

```bash
ssh-copy-id -i ~/.ssh/id_ed25519_kisa.pub user@kisapalvelin.fi
```

Testaa, että kirjautuminen toimii ilman salasanaa:

```bash
ssh -i ~/.ssh/id_ed25519_kisa user@kisapalvelin.fi echo ok
```

Tuloksena pitäisi näkyä pelkkä `ok`. Ensimmäinen kerta kysyy vielä palvelimen
host-avaimen hyväksynnän (`yes`).

## 2. rsync-komennon testaus käsin

Tulospalveluohjelma kirjoittaa HTML-tiedostot yleensä hakemistoon kuten
`C:\kisa\html\`. Sama polku näkyy WSL:ssä muodossa `/mnt/c/kisa/html/`.

Kokeile ensin käsin WSL-terminaalista:

```bash
rsync -av --delete-after --delay-updates \
  -e "ssh -i /home/KÄYTTÄJÄ/.ssh/id_ed25519_kisa" \
  /mnt/c/kisa/html/ \
  user@kisapalvelin.fi:/var/www/html/kisa/
```

Vaihda `KÄYTTÄJÄ` oman WSL-tunnuksesi mukaiseksi. Lippujen merkitys:

- `-a` = arkistotila (rekursiivinen, säilyttää aikaleimat)
- `-v` = näyttää siirretyt tiedostot (testaukseen; lopullisessa skriptissä
  pois)
- `--delete-after` = poistaa palvelimelta tiedostot, jotka eivät enää ole
  lähteessä — poistaa vasta onnistuneen siirron jälkeen
- `--delay-updates` = siirtää kaikki tiedostot ensin väliaikanimillä ja
  vaihtaa ne lopulliseen nimeen vasta kun koko erä on valmis

Jos tässä vaiheessa tiedostot ilmestyvät palvelimelle, SSH-tunnistus ja
rsync toimivat.

## 3. Windows-komentotiedosto `siirto.cmd`

Luo tiedosto, jonka tulospalveluohjelma käynnistää. Tulospalveluohjelma
edellyttää, että komento **ei tulosta mitään näytölle** ja **ei vaadi
käyttäjältä mitään** (ks. vanha Liite 10). Siksi tulostus ohjataan nul-
laitteeseen ja virheet lokitiedostoon.

Tallenna esim. nimellä `C:\kisa\html\siirto.cmd`:

```cmd
@echo off
REM Tulospalvelun HTML-tiedostojen siirto palvelimelle rsyncillä WSL:n kautta.
REM Kutsutaan automaattisesti HkKisaWin:n KOMENTO-parametrista.

set LOG=C:\kisa\html\siirto.log

wsl -- rsync -a --delete-after --delay-updates ^
  -e "ssh -i /home/KÄYTTÄJÄ/.ssh/id_ed25519_kisa -o StrictHostKeyChecking=accept-new -o ConnectTimeout=10" ^
  /mnt/c/kisa/html/ ^
  user@kisapalvelin.fi:/var/www/html/kisa/ ^
  >> "%LOG%" 2>&1

exit /b 0
```

Huomioita:

- `wsl -- komento` ajaa komennon oletus-WSL-jakelussa ilman uutta ikkunaa
- `^`-merkit jatkavat riviä .cmd-tiedostossa
- `ConnectTimeout=10` varmistaa, ettei komento jää roikkumaan, jos verkko
  on poikki
- `exit /b 0` kertoo tulospalvelulle onnistumista — tulospalveluohjelma ei
  muutenkaan reagoi paluukoodiin, mutta on siistiä
- Siirron lokia voi seurata PowerShellistä komennolla
  `Get-Content C:\kisa\html\siirto.log -Wait`

## 4. Tulospalveluohjelman konfigurointi

### Vaihtoehto A: parametrit konfiguraatiotiedostossa

Lisää kilpailun `.cfg`-tiedostoon (esim. `laskenta.cfg`) rivit:

```
HTML=c:\kisa\html\tulokset.htm/60/S
KOMENTO=c:\kisa\html\siirto.cmd
```

Merkitys:

- `HTML=...` — kirjoittaa HTML-tiedostot hakemistoon `c:\kisa\html\`
  60 sekunnin välein, `/S` = sarjakohtaiset tiedostot (muutokset talletetaan
  eri sarjoihin erikseen)
- `KOMENTO=...` — ajaa `siirto.cmd`-komentotiedoston aina, kun
  tulostiedostot on kirjoitettu

Halutessasi tarkempaa hallintaa tulostettavista sarjoista ja väliaikapisteistä
käytä `AUTOFILE.LST`-tiedostoa samassa hakemistossa (kts. luku 9.7).

### Vaihtoehto B: UI-valikosta

1. Tulostuskaavakkeella: *Tiedosto → Automaattinen tiedostotulostus*
2. Määrittele tiedostonimi, tulostusväli ja valitse sarjakohtainen kirjoitus
3. Syötä *Suoritettava komento* -kenttään `c:\kisa\html\siirto.cmd`

## 5. Vianetsintä

| Oire | Tarkista |
|------|----------|
| Mikään ei siirry | Aja `siirto.cmd` käsin komentoriviltä ja tarkista `siirto.log` |
| `ssh: permission denied` | Julkinen avain ei ole palvelimen `~/.ssh/authorized_keys`-tiedostossa |
| `ssh: Host key verification failed` | Käytä `-o StrictHostKeyChecking=accept-new` ja poista tarvittaessa `~/.ssh/known_hosts` |
| `wsl: command not found` | WSL ei ole asennettu — `wsl --install` PowerShellissä |
| `rsync: command not found` | `sudo apt install rsync` WSL:ssä (yleensä asennettuna valmiiksi) |
| Tulospalvelu jumittuu | `ConnectTimeout`-lippu puuttuu; katkaise verkkoyhteys ja kokeile uudestaan |
| Selain näyttää puolivalmista | Varmista että `--delay-updates` on mukana |

## 6. Vaihtoehto ilman WSL:ää

Jos WSL ei ole käytettävissä, voidaan käyttää Windowsin omaa OpenSSH-clientia
(mukana Windows 10/11:ssa) ja erikseen asennettua rsync-pakettia (esim.
`cwRsync` tai `MSYS2`-kautta). Tällöin `siirto.cmd`:ssä kutsutaan `rsync.exe`:tä
suoraan ja polut ovat Windows-muodossa (`C:\kisa\html\`). SSH-avain tallennetaan
`%USERPROFILE%\.ssh\`-hakemistoon. WSL on kuitenkin suositeltu, koska rsync ja
ssh toimivat siellä natiivisti ilman erillisiä asennuksia.

## 7. Turvallisuushuomioita

- Säilytä tuotantopalvelimen SSH-avainta vain siinä koneessa, jossa sitä
  tarvitaan. Jos kisakone katoaa, poista kyseinen julkinen avain palvelimen
  `authorized_keys`-tiedostosta.
- Rajoita avaimen oikeuksia palvelimen päässä komennolla
  `command="rsync --server ..."` `authorized_keys`-tiedostossa, jos haluat
  estää avaimen käytön muuhun kuin tähän yhteen siirtoon
  (ks. `man sshd` → `AUTHORIZED_KEYS FILE FORMAT`).
- Jos siirto ei ole aikakriittistä, harkitse avaimelle
  `from="kisakone.ip"`-rajoitusta.
