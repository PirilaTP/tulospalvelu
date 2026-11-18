# Bilaga 10. Automatisk körning av kommandofil

## Bilaga 10. Automatisk körning av kommandofil

### A10.1  Definition av kommandot

När automatisk skrivning av resultat till fil med jämna
mellanrum har startats i programmet ([kapitel 9.7](9.7_automaattinen_tiedostotulostus.md)), kan
ett externt kommando startas varje gång filerna har skrivits. Detta
arbetssätt aktiveras antingen i formuläret som styr den automatiska utskriften eller med parametern

```
KOMENTO=suoritettava komento
```

Kommandot som ska köras kan vara nästan vilket
kommando som helst som inte kräver åtgärder av användaren när det körs och som inte skriver
något på skärmen annat än via utdataströmmen "standard output". Denna
utdataström dirigeras automatiskt så att den inte syns. Även denna begränsning kan
tas bort genom att starta programmet i ett nytt fönster med kommandot
START.

I de flesta fall lönar det sig troligen att samla de kommandon som ska köras
i en kommandofil (BAT- eller CMD-fil) och ange namnet på denna
kommandofil i programmets parameter.

### A10.2  Automatisk överföring av html-filer med rsync över SSH (rekommendation)

Ett modernt och säkert sätt att automatiskt överföra de
html-resultatfiler som skapats till en egen webbserver är `rsync` över SSH-förbindelse.
`rsync` överför endast ändrade filer, så överföringen går snabbt och sparar
nätverket, och flaggan `--delay-updates` byter de enskilda filerna
från tillfälliga namn till slutliga namn först i slutet av överföringen. På så sätt
ser åskådarna normalt inte en halvfärdig html-fil. Autentisering med SSH:s publika
nyckel ersätter lösenordet, så överföringen lämpar sig även för automatisk
körning.

Denna anvisning ersätter de tidigare exemplen med `ftp` och `sftp2`, som inte längre
rekommenderas för moderna servrar.

#### Förutsättningar

- Windows 10/11 (22H2 eller nyare)
- WSL2 + Ubuntu (eller annan distribution), installation med `wsl --install` i PowerShell
  som administratör
- SSH-åtkomst till målservern (t.ex. `user@kisapalvelin.fi`) och
  skrivrättighet till dess katalog (t.ex. `/var/www/html/kisa/`)

#### 1. Skapa ett SSH-nyckelpar och registrera det på servern

Öppna en WSL-terminal (kommandot `wsl` från kommandoraden). Skapa en nyckel utan
lösenfras, så att resultatprogrammet kan köra överföringen
automatiskt:

```bash
ssh-keygen -t ed25519 -N "" -f ~/.ssh/id_ed25519_kisa
ssh-copy-id -i ~/.ssh/id_ed25519_kisa.pub user@kisapalvelin.fi
```

Testa att inloggningen lyckas utan lösenord:

```bash
ssh -i ~/.ssh/id_ed25519_kisa user@kisapalvelin.fi echo ok
```

Första gången måste du godkänna serverns värdnyckel (`yes`).

#### 2. Prova rsync-kommandot manuellt

Resultatprogrammet skriver html-filerna i regel till en egen katalog, såsom
`C:\kisa\www\`. Samma sökväg syns i WSL i formen `/mnt/c/kisa/www/`.
Prova först manuellt från WSL-terminalen:

```bash
rsync -av --delete-after --delay-updates \
  -e "ssh -i /home/KÄYTTÄJÄ/.ssh/id_ed25519_kisa" \
  /mnt/c/kisa/www/ \
  user@kisapalvelin.fi:/var/www/html/kisa/
```

Flaggornas betydelse:

- `-a` = rekursivt arkivläge (behåller tidsstämplar)
- `-v` = visar överförda filer (för testning; tas bort i det slutliga skriptet)
- `--delete-after` = tar bort filer från servern som inte längre finns
  i källan, först när överföringen har lyckats
- `--delay-updates` = överför filerna först med tillfälliga namn och byter dem
  till slutliga namn först i slutet av överföringen

När filerna dyker upp på servern fungerar SSH-autentiseringen och rsync.

#### 3. Windows-kommandofilen siirto.cmd

Skapa filen som resultatprogrammet startar. Kommandot får inte
skriva något på skärmen eller fråga något (se A10.1). Därför dirigeras utdata
till en loggfil. Spara skriptet någon annanstans än i den html-katalog som publiceras,
t.ex. med namnet `C:\kisa\siirto\siirto.cmd`:

```cmd
@echo off
REM Överföring av resultattjänstens HTML-filer till servern med rsync via WSL.
REM Anropas automatiskt från HkKisaWins parameter KOMENTO.

set LOG=C:\kisa\siirto\siirto.log

wsl -- rsync -a --delete-after --delay-updates ^
  -e "ssh -i /home/KÄYTTÄJÄ/.ssh/id_ed25519_kisa -o StrictHostKeyChecking=accept-new -o ConnectTimeout=10" ^
  /mnt/c/kisa/www/ ^
  user@kisapalvelin.fi:/var/www/html/kisa/ ^
  >> "%LOG%" 2>&1

exit /b %ERRORLEVEL%
```

Att observera:

- `wsl -- komento` kör kommandot i standarddistributionen för WSL utan nytt fönster.
- Tecknen `^` fortsätter raden i en .cmd-fil.
- `ConnectTimeout=10` säkerställer att kommandot inte blir hängande om nätverket
  är nere.
- Överföringsloggen kan följas från PowerShell med kommandot
  `Get-Content C:\kisa\siirto\siirto.log -Wait`.
- Håll `siirto.cmd` och `siirto.log` i en annan katalog än de html-filer
  som publiceras, så att de inte speglas till servern.

#### 4. Koppling till resultatprogrammet

Lägg till i tävlingens `.cfg`-fil (t.ex. `laskenta.cfg`):

```
HTML=c:\kisa\www\tulokset.htm/60/S
KOMENTO=c:\kisa\siirto\siirto.cmd
```

`HTML=...` skriver html-filerna med 60 sekunders mellanrum klassvis
(`/S`) och `KOMENTO=...` kör filen `siirto.cmd` varje gång efter detta.
Vilka klasser och mellantidspunkter som skrivs ut kan begränsas närmare med filen
`AUTOFILE.LST` (se [kapitel 9.7](9.7_automaattinen_tiedostotulostus.md)).

#### 5. Felsökning

| Symptom | Kontrollera |
|------|----------|
| Ingenting överförs | Kör `siirto.cmd` manuellt från kommandoraden och kontrollera `siirto.log` |
| `ssh: permission denied` | Den publika nyckeln saknas i serverns fil `~/.ssh/authorized_keys` |
| `ssh: Host key verification failed` | Använd `-o StrictHostKeyChecking=accept-new` och ta vid behov bort `~/.ssh/known_hosts` |
| `wsl: command not found` | WSL är inte installerat, kör `wsl --install` i PowerShell |
| `rsync: command not found` | `sudo apt install rsync` i WSL (i regel förinstallerat) |
| Resultattjänsten hänger sig | Flaggan `ConnectTimeout` saknas |
| Webbläsaren visar halvfärdigt innehåll | Kontrollera att `--delay-updates` är med |

#### 6. Alternativ utan WSL

Om WSL inte är tillgängligt kan Windows egen
OpenSSH-klient (ingår i Windows 10/11) och ett separat installerat
rsync-paket (t.ex. cwRsync eller MSYS2) användas. Då anropas
`rsync.exe` direkt i `siirto.cmd` och sökvägarna är i Windows-format
(`C:\kisa\html\`), och SSH-nyckeln sparas i katalogen `%USERPROFILE%\.ssh\`.
WSL rekommenderas dock, eftersom rsync och ssh fungerar där
nativt utan separata installationer.

#### 7. Säkerhetsaspekter

- Förvara SSH-nyckeln till servern endast på den dator där den behövs.
  Om tävlingsdatorn försvinner, ta bort den aktuella publika nyckeln från serverns
  fil `authorized_keys`.
- Nyckelns rättigheter bör begränsas i filen `authorized_keys`,
  t.ex. med definitionerna `command="rsync --server ..."` och
  `from="kisakone.ip"`; se `man sshd`, avsnittet *AUTHORIZED_KEYS FILE FORMAT*.
