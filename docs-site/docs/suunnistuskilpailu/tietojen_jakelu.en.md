# Distributing the data

Preparation of the competition typically takes place on one
computer. The files containing the data the competition needs are

- KilpSrj.xml is created when the competition is created. It contains
  the general competition settings and the class data.

  - KILP.DAT contains all competitor data

    - radat1.xml contains the course
      descriptions and the punch codes of the controls (the number 1 in the name refers to the first or
      only stage; the file of the second stage is rada2.xml )
    - as an option being phased out, RADAT.LST is a previously
      used file format that contains the course descriptions

      - as an option being phased out, LEIMAT.LST is a previously
        used file format that contains a list of the codes of the punches
        at the various controls- EMIT.DAT contains relevant data only once
      Emit cards are being read at punch checking

These files must be identical on all
computers taking part in the results service, and they are needed in two different
folders if two results-service programs, or two copies of the same program, are running
on the same computer.

In addition, configuration files are needed, which
define the tasks of the different computers and their connections to each other.
The configuration files usually differ between computers.

Other files are created when the programs are used

- COMFILE.DAT is created to ensure reliable
  data transfer (in a multi-stage competition COMFILE1.DAT, COMFILE2.DAT
  etc.)

  - timing files, such as AJAT.LST, are created if timing functions are used
    (also LAJAT.LST, AJAT1.LST, AJAT.LS2
    etc.)

    - log files are created if they have been requested.

When the competition starts, every folder in use
must contain identical copies of the files KilpSrj.xml and KILP.DAT. Likewise, the course file (radat1
.xml or equivalent) must be on all computers that handle
punch data in one way or another. An empty file EMIT.DAT may be included, but it is always created when needed.
The file COMFILE.DAT must be deleted when the file
KILP.DAT is distributed to the different computers, because COMFILE.DAT
may then contain unsent messages that cause
errors when the programs are used.

A good approach is to make command files that
first delete potentially problematic files and then copy
the new files KilpSrj.xml, KILP.DAT and radat1.xml
to the folders in use over the network. This requires that the connections between the computers
used for data transfer are allowed by defining the folder paths in question
as shared on the network. The command file could contain, for example

```
del \\192.168.1.11\Kisat\kisa1\*.dat
del \\192.168.1.11\Kisat\kisa1\*.lst
copy c:\kisat\kisa1\KilpSrj.xml \\192.168.1.11\Kisat\kisa1\
copy c:\kisat\kisa1\KILP.DAT \\192.168.1.11\Kisat\kisa1\
copy c:\kisat\kisa1\radat1.xml \\192.168.1.11\Kisat\kisa1\
```

This assumes that on the punch-checking computer 192.168.1.11 the folder C:\kisat has been shared
under the name Kisat, allowing writing to the folder and its subfolders,
and that the competition data is in the subfolder kisa1 of this folder both on the distributing and on the
receiving computer. It is also assumed that no file with the extension \*.dat or \*.lst needs to be kept.

The commands listed above are suitable for use from the server, in which case corresponding
lines for all the other competition folders can be included in the same file. Alternatively, the corresponding commands can be carried out so that only the server
shares the folder Kisat, and the other computers fetch the data with such a
command when they are started for the competition.

Separately, you must make sure that the necessary configuration files are available and that, where needed, the startup settings and the windows to open
have been saved from HkKisaWin before the competition starts.
