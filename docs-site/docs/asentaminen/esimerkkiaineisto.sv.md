# Exempelmaterial

Installationsprogrammet skapar mapparna *HkKisaWinData* och
*HkMaaliData* (sidan [Mappstruktur](kansiorakenne.md)), som
innehåller material avsett för att bekanta sig med programmen. Materialet motsvarar en liten
orienteringstävling så att

- *HkMaaliData* innehåller de uppgifter och
  inställningar som motsvarar en stämplingskontroll som genomförs med programmet
  *HkMaali*, förutom att ingen anslutning till en läsenhet har definierats.
  Du kan starta granskningen genom att klicka på den genväg till programmet
  *HkMaali* som installerats i denna mapp.

  - *HkKisaWinData* innehåller samma tävlings
    material för granskning med programmet *HkKisaWin*. För granskningen
    ska programmet *HkKisaWin* startas från sin egen genväg, som
    installationsprogrammet placerar både på skrivbordet och i startmenyn. Därefter
    ska du via programmets knapp *Öppna tävlingen för resultattjänst* bläddra fram
    mappen *HkKisaWinData* och öppna filen
    *Laskenta.cfg.*

    - När båda programmen har öppnats upprättas också en
      dataöverföringsförbindelse mellan dem.

Situationen som uppstår motsvarar i hög grad den som uppstår när
stämplingskontroll utförs på en dator och vissa andra uppgifter
utförs på en annan. I detta exempel har båda funktionerna bara genomförts på samma dator.

Båda mapparna innehåller filerna

- KILP.DAT, som innehåller alla deltagaruppgifter och resultat

  - KilpSrj.xml, som innehåller tävlingens och klassernas definitioner

    - EMIT.DAT, som innehåller de uppgifter som lästs från Emit-brickorna

      - LEIMAT.LST, som innehåller en förteckning över koderna för
        stämplingsenheterna vid kontrollerna

        - RADAT.LST, som innehåller banbeskrivningarna
          (kontrollistor och vissa andra uppgifter)

          - seurat.csv, som innehåller en förteckning över orienterings­klubbar
            och klubbförkortningar

            - COMFILE.DAT, som skapas varje gång dataöverföring
              tas i bruk och som används för att säkerställa dataöverföringen

              - Laskenta.cfg är en konfigurationsfil som beskriver
                uppgifterna och inställningarna för det program som använder mappen

De två sista filerna skiljer sig mellan mapparna,
de övriga har identiskt innehåll.

Dessutom finns i mappen *HkMaaliData* en genväg
till programmet *HkMaali* och i mappen *HkKisaWinData* en exempelvis
personförteckning samt en tom konfigurationsfil *ilmoitt.cfg*
avsedd för förberedelsefasen.

Filen *Laskenta.cfg* i mappen *HkMaaliData*
innehåller raderna

```
Kone=MA
Emit
yhteys1=udp:0/localhost:y2
lähemit1
```

Den första raden anger datorns tvåbokstavskod,
som syns på datorn i andra änden av dataöverföringen. Den andra raden anger att
programmet är förberett att hantera uppgifter från Emit-stämplingskontrollen. Den tredje raden anger
att förbindelse nr 1 använder udp-protokollet för att ansluta till förbindelse nr 2 i en
annan programinstans på samma dator. Tecknet '0' före snedstrecket anger
att förbindelsen använder förbindelse 1:s standardport 15901. Den fjärde raden anger att
även stämplingsuppgifterna överförs via förbindelse 1.

Filen *Laskenta.cfg* i mappen *HkKisaWinData*
innehåller raderna

```
Kone=WI
Emit
yhteys2=udp
lähemit2
```

Innehållet är i övrigt detsamma, men om förbindelsen behöver man i denna ände inte ange annat än dess nummer, eftersom programmet kan härleda övriga uppgifter när det andra programmet ansluter
till
det.
