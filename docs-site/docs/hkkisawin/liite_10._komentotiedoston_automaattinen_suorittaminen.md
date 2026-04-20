# Liite 10. Komentotiedoston automaattinen suorittaminen

## Liite 10. Komentotiedoston automaattinen suorittaminen

Huom. Ohjelmasta voidaan nyt käynnistää tiedostojen
siirto myös [sisäisenä
toimintona](9.7_automaattinen_tiedostotulostus.md) 
ilman komentotiedostoon turvautumista

### A10.1  Komennon määrittely

Kun ohjelmassa on käynnistetty tulosten automaattinen
kirjoittaminen määrävälein tiedostoon ([luku 9.7](9.7_automaattinen_tiedostotulostus.md)), voidaan
aina tiedostojen kirjoittamisen jälkeen käynnistää ulkoinen komento. Tämä
toimintatapa käynnistetään joko automaattista tulostusta ohjaavalla kaavakkeella tai parametrilla

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

### A10.2  Html-tiedostojen automaattinen ftp-siirto

Tyypillinen käyttötarkoitus ulkoisen komennon
automaattiselle suorittamiselle on ohjelman automaattisesti luomien
html-tiedostojen siirtäminen ftp-protokollaa käyttäen www-palvelimelle. Tämä
toimintatapa voidaan käynnistää seuraavasti. Parametreilla

```
HTML=c:\kisa\html\tulokset.htm/60  
KOMENTO=c:\kisa\html\ftpsiirto.cmd
```

käynnistetään tiedoston c:\kisa\html\tulokset.htm
automaattinen kirjoittaminen 60 s välein sekä aina tämän tiedostojen
kirjoittamisen jälkeen samassa hakemistossa oleva komentotiedosto ftpsiirto.cmd

Usein kannattaa lisätä parametriin HTML täydennys /S ilmaisemaan,
että sarjat tulostetaan erikseen ja käyttää tulostettavat sarjat ja pisteet
määrittelevää tiedostoa AUTOFILE.LST
. Asiasta lisää ohjelmien yleisissä
ohjeissa.

Tiedoston ftpsiirto.cmd sisältö voi olla esimerkiksi

```
if NOT exist c:\kisa\html\*.htm goto loppu
ftp -v -i -n -s:c:\kisa\html\ftpsiirto.txt
del c:\kisa\html\*.htm
:loppu
```

Tiedosto ftpsiirto.txt sisältää puolestaan ohjelmaa ftp
ohjaavan skriptin, jonka sisältö voisi olla

```
open wwwpalvelin.tarjoaja.fi
user username password
cd public_html/kisa
binary
mput c:\kisa\html\*.htm
quit
```

missä kaksi ensimmäistä riviä määrittelevät käytettävän
ftp-palvelimen, käyttäjätunnuksen ja salasanan, kolmas rivi pyytää siirtymään
palvelimen hakemistoon public\_html/kisa
ja viides rivi käynnistää tiedostojen siirron.

### A10.3  Secure ftp:n (ohjelman sftp2) käyttö

Monet palvelimet eivät salli ftp-tiedonsiirtoa
tietoturvasyistä, mutta sallivat suojatun secure ftp:n käytön. Tämä voidaan
hoitaa käyttäen komentoriviohjelmaa sftp2. Ohjelman
sftp2 käyttöä vaikeuttaa kaksi seikkaa:

- SSH-ohjelmat, joista sftp2 on yksi, vaativat yleensä, että käyttäjä
  antaa näppäimistöltä salasanan, mikä ei sovi yhteen tiedonsiirron
  automatisoinnin kanssa. Tämä rajoitus voidaan ratkaista ottamalla käyttöön
  julkiseen avaimeen perustuva käyttäjän tunnistus ja määrittelemällä julkiseen
  avaimeen liittyvä salafraasi tyhjäksi.

  - sftp2 ei salli
    näyttötulostuksen ohjaamista näkymättömiin. Tästä syystä ei ohjelmaa voida
    käynnistää samassa ikkunassa, jossa tulospalveluohjelma toimii. Tämä rajoitus
    voidaan kiertää avaamalla sftp2 uuteen ikkunaan
    komennolla start.

Ohjelmaa sftp2 voidaan käyttää
esimerkiksi käynnistämällä tulospalveluohjelmasta komentotiedosto
sftpsiirto.cmd, jonka sisältö on

```
start /min sftp2 -B sftpsiirto.txt user@palvelin.tarjoaja.fi
```

Parametri /min ei ole välttämätön, mutta se estää uuden
ikkunan aukeamisen häiritsevästi näytölle.

Tiedosto sftpsiirto.txt
sisältää ohjelmalle sftp2
lähetettävät komennot, jotka voivat olla esimerkiksi

```
cd public_html/kisa
mput c:\kisa\html\*.htm
quit
```