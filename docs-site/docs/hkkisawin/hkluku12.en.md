# Chapter 12. Using a MySQL database

## 12 Using a MySQL database

### 12.1 General

A connection to a MySQL database can be used either so that all competitor data are
written to a MySQL database or read from a MySQL database, or so that
the program immediately saves every change to the competitor data also
to the database server.

The database support is based on the dbExpress database interface. The program delivery includes
all the files needed to create the connection. If dbExpress is already in use on the computer for other purposes,
compatibility problems may arise through the dbxdrivers.ini file.
The program installs the supplied file in the same directory as HkKisaWin, but
a dbxdrivers.ini installed with another program may override this file. The program
components used are from the 32-bit MySQL version 5.1, but they also connect to some
other MySQL versions (at least version 5.5).

The connection to the database can be opened using the menu item "Files/MySQL" or using the parameters

|  |  |
| --- | --- |
| SQLHOST=host | Defines the name of the MySQL server (default 'localhost') |
| SQLDATABASE=database | Defines the name of the MySQL database (default 'kilp') |
| SQLUSER=user | Defines the MySQL database user (default 'kilp') |
| SQLPASSWORD=salasana | Defines the password of the MySQL database user (default 'hkkisawin') |
| SQLSTART | Requests that the MySQL database be opened automatically when the program starts |
| SQLTALLENNUS | Requests that the MySQL database be opened so that all changes to the competitor data are saved automatically |

When immediate saving of changes is in use, the program by default secures the data transfer with the file SQLjonoVarm.dat.
From this file the program determines which competitor data may have been left unsent when the program was closed.

Transferring all competitor data to the database is requested with "Files/Write to transfer file" and
reading from the database with "Entries/Get from file". When data concerning the same entry are transferred to
the database again, the new data replace the old.

### 12.2 Database structure

Competitor data are saved in three database tables: osanotot, osottopv and tulos. The SQL commands
that create these tables are included in the distribution file kilpsql.txt, which is installed in the competition data folder. The keys of all data
include the competition code, defined in the basic competition properties, and the position of the competitor record
in the file KILP.DAT, which does not change unless the order is changed, for example by writing to a CSV file
and reading it back in a different order. (Deleted records may change positions when doing so.)

It is essential that each competition
saved in the same database is given a different code. This way you can save
in one database, for example, the participants and results of all fitness orienteering events
of the season. At present the program does not allow you to leave
out, for example, vacancies, so if desired they must be removed afterwards,
for example with a suitable SQL command.
