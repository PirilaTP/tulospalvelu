# Configuration files

The configuration files presented here are based on
the following choices:

- **Option
  2** described earlier on the page *Tasks of the
  computers* is used .

  - The IP addresses of the computers are:

    - 192.168.1.10  Office

      - 192.168.1.11  Punch checking

        - 192.168.1.12  Announcer- The default printer of the office computer
      is used for printing

      - The card reader is connected to the punch-checking
        computer with a USB adapter, which has been given the port COM5.

        - The complaints desk functions are handled in the office

          - All computers use the folder C:\Kisat\Kisa1 for the competition

            - On the office computer, HkKisaWin uses
              the subfolder C:\Kisat\Kisa1\HkKisaWin and
              HkMaali uses C:\Kisat\Kisa1\HkMaali

              - The announcer does not follow the Emit split times
                obtained from punch checking

The following configuration files contain comment lines starting with //
that explain the meaning of the next line

**Configuring the office computer**

Folder C:\Kisat\Kisa1\HkMaali

- description

  - uses the program
    HkMaali

    - acts as the hub of the data transfer,
      i.e. the "server"

      - also handles automatic printing

        - the program is started from a desktop
          shortcut whose start-in folder is set to C:\Kisat\Kisa1\HkMaali, and
          the parameter CFG=SR.CFG has been written after the name of the program
          to start, HkMaali.exe- contents of the configuration file SR.CFG

```
// 2-character identifier to help interpret the data transfer connections
kone=sr
// Title of the console window to open
ikkunaots=Server
// Connection to the punch-checking computer. Also transfers the punch data
yhteys1=udp:0/192.168.1.11:y1
lähemit1
// One-way connection to the announcer's computer. Does not transfer punch data
yhteys2=udpo:0/192.168.1.12:y1
// Connection to the WinKisaHk program on the office computer. Also transfers the punch data
yhteys3=udp:0/localhost::y4
lähemit3
// The program uses the default printer
lista
// The program prints results lists automatically
auto=L/40/60
//  The program automatically prints a report of the punches that led to a proposed disqualification
comautorap=h
```

Folder C:\Kisat\Kisa1\HkKisaWin

- description

  - uses the program HkKisaWin

    - handles the various office tasks,
      including the needs of the complaints desk

      - the program is started from a desktop
        shortcut, which is the basic shortcut for starting HkKisaWin- starting in this competition has been automated
          in HkKisaWin's menu item *Files /
          Startup settings* during earlier use (the information is saved
          in the file init.cfg, but you
          do not need to deal with this directly).- in this case to.cfg has been selected as the configuration file- contents of the configuration file to.cfg

```
// 2-character identifier to help interpret the data transfer connections
kone= to 
// Connection to the HkMaali program on the office computer. Also transfers the punch data
yhteys4= udp
lähemit4
```

**Configuring the punch-checking computer**

- description

  - uses the program
    HkKisaWin

    - handles punch checking, which
      also produces the finish times

      - the program is started from a desktop shortcut, which is the basic shortcut for starting HkKisaWin- starting in this competition has been automated in HkKisaWin's menu item *Files / Startup settings* during earlier use (the information is saved in the file init.cfg, but you do not need to deal with this directly).- in this case em.cfg has been selected as the configuration file- during earlier use, the state of the windows to open
              has been saved with the menu item *Files / Save windows* while
              the punch-checking window was in the desired place on the screen (the selection is saved
              in the file oletus.ikk)- contents of the configuration file em.cfg

```
// 2-character identifier to help interpret the data transfer connections
kone=em
//  The program opens the previously selected windows at startup
ikkunat
// Connection to the HkMaali program on the office computer. Also transfers the punch data
yhteys1=udp
lähemit1
//  Card reader connected to port COM5 (via a UDP/serial port adapter)
lukija=5
```

**Configuring the announcer's computer**

- description

  - uses the program
    HkKisaWin

    - the announcer
      follows the competition with the latest events display and a window
      showing the standings of two
      selected classes

      - the program is started from a desktop shortcut, which is the basic shortcut for starting HkKisaWin- starting in this competition has been automated in HkKisaWin's menu item *Files / Startup settings* during earlier use (the information is saved in the file init.cfg, but you do not need to deal with this directly).- in this case ku.cfg has been selected as the configuration file- during earlier use, the state of the windows to open
              has been saved with the menu item *Files / Save windows* while
              the windows were in the desired places on the screen (the selection is saved
              in the file oletus.ikk)- contents of the configuration file ku.cfg

```
// 2-character identifier to help interpret the data transfer connections
kone=ku 
// The program opens the previously selected windows at startup
ikkunat  
// Connection to the HkMaali program on the office computer.
yhteys1=udp
```
