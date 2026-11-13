# Distributing the data

Preparation of the competition typically takes place on one
computer. The files containing the data the competition needs are

- KilpSrj.xml is created when the competition is created. It contains
  the general competition settings and the class data.

  - KILP.DAT contains all competitor data

These files must be identical on all
computers taking part in the results service, and they are needed in two different
folders if two results-service programs, or two copies of the same program, are running
on the same computer.

In addition, the configuration files described in the previous chapter are needed;
they define the tasks of the different computers and their
connections to each other. The configuration files differ between computers.

Other files are created when the programs are used

- COMFILE.DAT is created to ensure reliable
  data transfer

  - timing files, such as AJAT1.LST and LAJAT.LST, are created if
    timing functions are used

    - log files are created if they have been requested.

When the competition starts, every folder in use
must contain identical copies of the files KilpSrj.xml and KILP.DAT
. The file COMFILE.DAT must be deleted when the file
KILP.DAT is distributed to the different computers, because COMFILE.DAT
may then contain unsent messages that cause errors when the programs are used.

A good approach is to make command files that first delete potentially problematic files and then copy
the new files KilpSrj.xml and KILP.DAT to the folders in use
over the network. This requires that the connections between the computers used for data transfer are allowed by defining the folder paths in question
as shared on the network. The command file could contain, for example

```
del \\192.168.1.13\Kisat\kisa1\*.dat
del \\192.168.1.13\Kisat\kisa1\*.lst
copy c:\kisat\kisa1\KilpSrj.xml \\192.168.1.13\Kisat\kisa1\
copy c:\kisat\kisa1\KILP.DAT \\192.168.1.13\Kisat\kisa1\
del \\192.168.1.12\Kisat\kisa1\*.dat
copy c:\kisat\kisa1\KilpSrj.xml \\192.168.1.12\Kisat\kisa1\
copy c:\kisat\kisa1\KILP.DAT \\192.168.1.12\Kisat\kisa1\
```

This assumes that on the punch-checking computers
192.168.1.13 (finish computer) and 192.168.1.12 (announcer's computer) the folder
C:\kisat has been shared on the network under the name Kisat, allowing writing to the folder and its subfolders,
and that the competition data is in the subfolder kisa1 of this folder both on the
distributing and on the receiving computer. It is also assumed that no file with the extension \*.dat needs to be kept.

The commands listed above
are suitable for use from the office computer. Alternatively, the corresponding
commands can be carried out so that only the office computer shares the folder Kisat, and the other computers
fetch the data with a corresponding command when they are started for the competition.

Separately, you must make sure that the necessary
configuration files are available and that, where needed, the startup settings and the windows to open
have been saved from HkKisaWin before the competition starts.
