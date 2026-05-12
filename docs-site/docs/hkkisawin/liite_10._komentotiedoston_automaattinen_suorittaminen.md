# Liite 10. Komentotiedoston automaattinen suorittaminen

## Liite 10. Komentotiedoston automaattinen suorittaminen

### A10.1  Komennon määrittely

Kun ohjelmassa on käynnistetty tulosten automaattinen
kirjoittaminen määrävälein tiedostoon ([luku 9.7](9.7_automaattinen_tiedostotulostus.md)), voidaan
aina tiedostojen kirjoittamisen jälkeen käynnistää ulkoinen komento. Tämä
toimintatapa käynnistetään joko automaattista tulostusta ohjaavalla kaavakkeella tai parametrilla

```
KOMENTO=suoritettava komento
```

Suoritettava komento voi olla miltei mikä tahansa
komento, joka ei vaadi käyttäjän toimenpiteitä toimiessaan ja joka ei tulosta
näytölle mitään muuten kuin "standard output" tulostusvirran kautta. Tämä
tulostusvirta on ohjattu automaattisesti näkymättömiin. Tämäkin rajoitus voidaan
poistaa käynnistämällä ohjelma uudessa ikkunassa komennolla
START.

Useimmissa tapauksissa kannattanee koota suoritettavat
komennot komentotiedostoon (BAT- tai CMD-tiedostoon) ja antaa tämän
komentotiedoston nimi ohjelman parametrissa.

### A10.2  Html-tiedostojen automaattinen siirto rsyncillä SSH:n yli (suositus)

Nykyaikainen ja tietoturvallinen tapa siirtää automaattisesti luodut
html-tulostiedostot omalle www-palvelimelle on `rsync` SSH-yhteyden yli.
`rsync` siirtää vain muuttuneet tiedostot, joten siirto on nopea ja säästää
verkkoa, ja lippu `--delay-updates` vaihtaa yksittäiset tiedostot
väliaikanimistä lopullisiin nimiin vasta siirron lopuksi. Näin katsojat
eivät normaalisti näe puolivalmista html-tiedostoa. SSH:n julkisen
avaimen tunnistus korvaa salasanan, joten siirto sopii myös automaattiseen
ajoon.

Tämä ohje korvaa aiemmat `ftp`- ja `sftp2`-esimerkit, joita ei enää
suositella nykyaikaisille palvelimille.

#### Esivaatimukset

- Windows 10/11 (22H2 tai uudempi)
- WSL2 + Ubuntu (tai muu jakelu), asennus `wsl --install` PowerShellissä
  järjestelmänvalvojana
- SSH-pääsy kohdepalvelimelle (esim. `user@kisapalvelin.fi`) ja
  kirjoitusoikeus sen hakemistoon (esim. `/var/www/html/kisa/`)

#### 1. SSH-avainparin luonti ja rekisteröinti palvelimelle

Avaa WSL-terminaali (komento `wsl` komentoriviltä). Luo avain ilman
salafraasia, jotta tulospalveluohjelma pystyy ajamaan siirron
automaattisesti:

```bash
ssh-keygen -t ed25519 -N "" -f ~/.ssh/id_ed25519_kisa
ssh-copy-id -i ~/.ssh/id_ed25519_kisa.pub user@kisapalvelin.fi
```

Testaa, että kirjautuminen onnistuu ilman salasanaa:

```bash
ssh -i ~/.ssh/id_ed25519_kisa user@kisapalvelin.fi echo ok
```

Ensimmäisellä kerralla pitää hyväksyä palvelimen host-avain (`yes`).

#### 2. Rsync-komennon kokeilu käsin

Tulospalveluohjelma kirjoittaa html-tiedostot yleensä omaan hakemistoon kuten
`C:\kisa\www\`. Sama polku näkyy WSL:ssä muodossa `/mnt/c/kisa/www/`.
Kokeile ensin käsin WSL-terminaalista:

```bash
rsync -av --delete-after --delay-updates \
  -e "ssh -i /home/KÄYTTÄJÄ/.ssh/id_ed25519_kisa" \
  /mnt/c/kisa/www/ \
  user@kisapalvelin.fi:/var/www/html/kisa/
```

Lippujen merkitys:

- `-a` = rekursiivinen arkistotila (säilyttää aikaleimat)
- `-v` = näyttää siirretyt tiedostot (testaukseen; lopullisessa skriptissä pois)
- `--delete-after` = poistaa palvelimelta tiedostot, jotka eivät enää ole
  lähteessä vasta kun siirto on onnistunut
- `--delay-updates` = siirtää tiedostot ensin väliaikanimillä ja vaihtaa ne
  lopullisiin nimiin vasta siirron lopuksi

Kun tiedostot ilmestyvät palvelimelle, SSH-tunnistus ja rsync toimivat.

#### 3. Windows-komentotiedosto siirto.cmd

Luo tiedosto, jonka tulospalveluohjelma käynnistää. Komento ei saa
tulostaa näytölle eikä kysyä mitään (ks. A10.1). Siksi tulostus ohjataan
lokitiedostoon. Tallenna skripti muualle kuin julkaistavaan html-hakemistoon,
esim. nimellä `C:\kisa\siirto\siirto.cmd`:

```cmd
@echo off
REM Tulospalvelun HTML-tiedostojen siirto palvelimelle rsyncillä WSL:n kautta.
REM Kutsutaan automaattisesti HkKisaWin:n KOMENTO-parametrista.

set LOG=C:\kisa\siirto\siirto.log

wsl -- rsync -a --delete-after --delay-updates ^
  -e "ssh -i /home/KÄYTTÄJÄ/.ssh/id_ed25519_kisa -o StrictHostKeyChecking=accept-new -o ConnectTimeout=10" ^
  /mnt/c/kisa/www/ ^
  user@kisapalvelin.fi:/var/www/html/kisa/ ^
  >> "%LOG%" 2>&1

exit /b %ERRORLEVEL%
```

Huomioita:

- `wsl -- komento` ajaa komennon oletus-WSL-jakelussa ilman uutta ikkunaa.
- `^`-merkit jatkavat riviä .cmd-tiedostossa.
- `ConnectTimeout=10` varmistaa, ettei komento jää roikkumaan, jos verkko
  on poikki.
- Siirron lokia voi seurata PowerShellistä komennolla
  `Get-Content C:\kisa\siirto\siirto.log -Wait`.
- Pidä `siirto.cmd` ja `siirto.log` eri hakemistossa kuin julkaistavat
  html-tiedostot, jotta niitä ei peilata palvelimelle.

#### 4. Kytkentä tulospalveluohjelmaan

Lisää kilpailun `.cfg`-tiedostoon (esim. `laskenta.cfg`):

```
HTML=c:\kisa\www\tulokset.htm/60/S
KOMENTO=c:\kisa\siirto\siirto.cmd
```

`HTML=...` kirjoittaa html-tiedostot 60 sekunnin välein sarjakohtaisesti
(`/S`) ja `KOMENTO=...` ajaa `siirto.cmd`-tiedoston aina tämän jälkeen.
Tarkemmin tulostettavia sarjoja ja väliaikapisteitä voi rajata tiedostolla
`AUTOFILE.LST` (ks. [luku 9.7](9.7_automaattinen_tiedostotulostus.md)).

#### 5. Vianetsintä

| Oire | Tarkista |
|------|----------|
| Mikään ei siirry | Aja `siirto.cmd` käsin komentoriviltä ja tarkista `siirto.log` |
| `ssh: permission denied` | Julkinen avain puuttuu palvelimen `~/.ssh/authorized_keys`-tiedostosta |
| `ssh: Host key verification failed` | Käytä `-o StrictHostKeyChecking=accept-new` ja poista tarvittaessa `~/.ssh/known_hosts` |
| `wsl: command not found` | WSL ei ole asennettu, aja `wsl --install` PowerShellissä |
| `rsync: command not found` | `sudo apt install rsync` WSL:ssä (yleensä asennettuna valmiiksi) |
| Tulospalvelu jumittuu | `ConnectTimeout`-lippu puuttuu |
| Selain näyttää puolivalmista | Varmista että `--delay-updates` on mukana |

#### 6. Vaihtoehto ilman WSL:ää

Jos WSL ei ole käytettävissä, voidaan käyttää Windowsin omaa
OpenSSH-clientia (mukana Windows 10/11:ssä) ja erikseen asennettua
rsync-pakettia (esim. cwRsync tai MSYS2). Tällöin `siirto.cmd`:ssä
kutsutaan `rsync.exe`:tä suoraan ja polut ovat Windows-muodossa
(`C:\kisa\html\`) ja SSH-avain tallennetaan hakemistoon `%USERPROFILE%\.ssh\`.
WSL on kuitenkin suositeltu, koska rsync ja ssh toimivat siellä
natiivisti ilman erillisiä asennuksia.

#### 7. Turvallisuushuomioita

- Säilytä palvelimen SSH-avainta vain siinä koneessa, jossa sitä tarvitaan.
  Jos kisakone katoaa, poista kyseinen julkinen avain palvelimen
  `authorized_keys`-tiedostosta.
- Avaimen oikeuksia kannattaa rajoittaa `authorized_keys`-tiedostossa
  esim. määrittelyillä `command="rsync --server ..."` ja
  `from="kisakone.ip"`; ks. `man sshd` kohta *AUTHORIZED_KEYS FILE FORMAT*.
