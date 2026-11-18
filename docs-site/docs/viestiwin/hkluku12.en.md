# Chapter 12. Using a MySQL database

## 12 Using a MySQL database

### 12.1 General

A connection to a MySQL database is available both so that all competitor data is either
written to the MySQL database or read from the MySQL database, and so that
the program saves every change to the competitor data immediately also
to the database server.

The database support is based on the dbExpress database interface. The program distribution includes
all files needed to create the connection. If dbExpress is already otherwise in use on the computer,
compatibility problems may arise through the file dbxdrivers.ini.
The program installs the included file in the same directory as HkKisaWin, but
a dbxdrivers.ini installed with another program may override this file. The program
components used are from the 32-bit MySQL version 5.1, but they also connect to some
other MySQL versions (at least version 5.5).

The connection to the database can be opened with the menu selection "Files/MySQL" or with the parameters

|  |  |
| --- | --- |
| SQLHOST=host | Defines the name of the MySQL server (default 'localhost') |
| SQLDATABASE=database | Defines the name of the MySQL database (default 'kilp') |
| SQLUSER=user | Defines the MySQL database user (default 'kilp') |
| SQLPASSWORD=salasana | Defines the password of the MySQL database user (default 'hkkisawin') |
| SQLSTART | Requests opening the MySQL database automatically when the program starts |
| SQLTALLENNUS | Requests opening the MySQL database so that all changes to the competitor data are saved automatically |

When immediate saving of changes is in use, the program by default backs up the data transfer with the file SQLjonoVarm.dat.
From this file the program determines which competitor data may have been left unsent when the program was closed.

Transferring all competitor data to the database is requested with the selection "Files/Write to transfer file" and
reading from the database with the selection "Entries/Get from file". When data concerning the same entry is transferred again
to the database, the new data replaces the old.

### 12.2 Database structure

Competitor data is stored in three database tables: vosanotot, osuudet
and vtulos. The SQL commands that create these tables are included
in the distribution file vkilpsql.txt and are installed in the competition data folder.

The keys of all data include the competition code, which is defined
in the competition's basic properties, and the position of the competitor record in the file
KILP.DAT, which is unchanging unless the order is changed, for example by
writing to a CSV file and reading back from it in a different order.
(Deleted records can change positions when doing so.)

It is essential that every competition saved in the same database is given a different code.
In this way, for example, the entries and results of all fitness orienteering events of a season
can be stored in one database.
