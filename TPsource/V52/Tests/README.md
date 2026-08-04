# Yksikkötestit

Testikehyksenä on [doctest](https://github.com/doctest/doctest) 2.4.11 (MIT),
vendoroituna tiedostona `doctest.h`. Lisenssi on `doctest-LICENSE.txt`.

## Ajaminen

**Visual Studio 2022:** avaa `..\VS\Tests\TpTest.sln` ja paina F5. Testiajuri on
tavallinen konsolisovellus, joka palauttaa != 0 jos yksikin testi epäonnistuu.

**Komentoriviltä (Developer Command Prompt):**

```
msbuild TPsource\V52\VS\Tests\TpTest.sln /p:Configuration=Release /p:Platform=Win32
TPsource\V52\VS\Tests\Release\TpTest.exe
```

**Linux / WSL (nopein kehityssilmukka):**

```
cd TPsource/V52 && ./Tests/run.sh
```

Testit ajetaan automaattisesti jokaisessa PR:ssä, ks. `.github/workflows/build.yml`.

## Miten testattavaa koodia kirjoitetaan tähän projektiin

Suurin osa ohjelmasta on käännösyksiköissä, jotka `#include`aavat `windows.h`:n
tai VCL:n ja lukevat globaaleja. Niitä ei voi linkittää testeihin vetämättä
mukaan koko ohjelmaa ja tietokantaa.

Käytetty tapa on erottaa päätöslogiikka omaan pieneen käännösyksikköönsä, joka
saa kaiken tarvitsemansa parametreina:

- `Tp/SITulkinta.cpp` sisältää SportIdent-kortin tavupuskurin tulkintalogiikan
  (`tulkSI`), ei riipu globaaleista eikä Windowsista/VCL:sta
- `Tp/TpLaitteet.cpp` sisältää ohuen adapterin (`lue_SI`:n sisällä), joka
  poimii globaalin `t0`:n ja kopioi tuloksen `san_type`-unioniin

Testit kohdistuvat aina siihen puhtaaseen osaan. Adapteri jää testaamatta, mutta
siinä ei ole enää mitään mitä voisi saada väärin.

Linux-käännös CI:ssä ei ole vain nopeusoptimointi: se hajoaa heti jos puhtaaseen
käännösyksikköön lisätään Windows- tai globaaliriippuvuus, eli se pitää
testattavuuden yllä automaattisesti.

Kun lisäät uuden testitiedoston, lisää se `ClCompile`-listaan tiedostoon
`..\VS\Tests\TpTest.vcxproj` ja lähdelistaan tiedostossa `Tests/run.sh`.

## Merkistökoodaus

Muu projekti on ISO-8859-1:tä/UTF-8:aa sekaisin tiedostosta riippuen, mutta
`Tests/`-hakemiston ja `Tp/SITulkinta.*`:n lähdetiedostot ovat **pelkkää
ASCIIta**: kommenteissa ei käytetä skandeja ja merkkijonoliteraaleissa ne
kirjoitetaan `\u`-escapeina, jos niitä tarvitaan.

Näin samat tiedostot kääntyvät MSVC:llä, g++:lla ja C++Builderilla ilman
merkistö-optioita, ja doctestin tulosteet näkyvät oikein sekä Windowsin
konsolissa että GitHub Actionsin lokissa.
