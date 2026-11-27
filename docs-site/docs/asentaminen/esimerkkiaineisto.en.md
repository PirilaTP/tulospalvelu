# Sample data

The installer creates the folders *HkKisaWinData* and
*HkMaaliData* (page [Folder structure](kansiorakenne.md)), which
contain material intended for getting to know the programs. The material corresponds to a small
orienteering competition as follows:

- *HkMaaliData* contains the data and
  settings for punch checking carried out with the program
  *HkMaali*, except that no connection to a card reader has been defined.
  You can start exploring it by clicking the shortcut to the program
  *HkMaali* installed in this folder.

  - *HkKisaWinData* contains the data of the same competition
    for viewing with the program *HkKisaWin*. To view it,
    start *HkKisaWin* from its own shortcut, which the
    installer places both on the desktop and in the Start menu. Then,
    using the *Open competition for results service* button of the program, browse to the
    folder *HkKisaWinData* and open the file
    *Laskenta.cfg.*

    - When both programs are open, a data transfer connection
      also opens between them.

The resulting situation closely corresponds to one where
one computer carries out punch checking and another performs some
other tasks. In this example both functions just run on the same computer.

Both folders contain the files

- KILP.DAT, which contains all competitor data and results

  - KilpSrj.xml, which contains the competition and class settings

    - EMIT.DAT, which contains the data read from the Emit cards

      - LEIMAT.LST, which contains a list of the codes of the punches
        at the controls

        - RADAT.LST, which contains the course descriptions
          (control lists and some other data)

          - seurat.csv, which contains a list of orienteering
            clubs and club abbreviations

            - COMFILE.DAT, which is created whenever data transfer
              is enabled and is used to ensure reliable data transfer

              - Laskenta.cfg is a configuration file that describes
                the tasks and settings of the program using that folder

The last two files differ between the folders;
the others have identical contents.

In addition, the folder *HkMaaliData* contains a shortcut
to the program *HkMaali*, and the folder *HkKisaWinData* contains a sample
list of persons and an empty configuration file *ilmoitt.cfg* intended
for the preparation phase.

The file *Laskenta.cfg* in the folder *HkMaaliData*
contains the lines

```
Kone=MA
Emit
yhteys1=udp:0/localhost:y2
lähemit1
```

The first line gives the two-letter identifier of the computer,
which is shown on the computer at the other end of the data transfer. The second line says that
the program is prepared to handle Emit punch-checking data. The third line says
that connection no 1 uses the udp protocol to connect to connection no 2 of
another program instance running on the same computer. The character '0' before the slash means
that the connection uses the default port of connection 1, 15901. The fourth line says that
the punch data is also transferred through connection 1.

The file *Laskenta.cfg* in the folder *HkKisaWinData*
contains the lines

```
Kone=WI
Emit
yhteys2=udp
lähemit2
```

The contents are otherwise the same, but at this end of the connection only its number needs to be given, because the program can deduce the other details when the other program
connects
to it.
