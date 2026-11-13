# Appendix 5. Fitness orienteering model 3

## Appendix 5. Fitness orienteering model 3

### A5.1 Basic description of the operating mode

Fitness orienteering operating mode 3 is enabled by giving
the program the startup parameter

`KUNTOSUUNNISTUS=3`

If pre-reading is also used, the pre-reading program must be given the parameter

`ESILUENTA=KUNTO`

You then get to fitness orienteering mode by opening the program in results service mode and selecting
*Results service/Emit reading* or *Entries / Entries* from the main menu, depending on
the task handled at that point. Alternatively, you can define the program to open
directly in results service mode and to open the Emit reading form on startup.

In operating mode 3 the whole consists of the following tasks:

- Preparing the files- Entering participant data in the competition office- Assigning Emit codes to participants and marking participants
      as starters.- Punch check, which also produces the results.

### A5.2 Preparations for operating mode 3

The preparation includes the following steps:

- Defining the competition
  - The file folder to be used is selected and created either outside the program or in the function
    *Competition creation and basic properties*- The classes of the competition are created and named either in the above-mentioned
      function or by copying a suitable earlier KilpSrj.xml file
      to the competition's file folder before defining the basic competition data. The classes must
      include an extra vacancy class, in which all data are placed during
      preparation. The program tries to create a suitable
      vacancy class automatically.- In the basic properties you should define the title data and give the competition an identification code that
        differs from other competitions.  
  - Defining the courses in the program's menu item *Preparation / Course data*. For each class you must
    define a course whose name is the same as the class name. As the reset advance,
    define *Reset at start time*, and for the finish you must select
    *Punching unit on finish line (not online*). Defining a course is also
    recommended for the vacancy class to eliminate disturbing error messages.
    The course of the vacancy class includes at least one control that is not in the forest and that is given
    a control code that is not in use either, so that nobody can get an accepted
    performance in the vacancy class.
      
      
    - Creating the configuration file. It is simplest to use the default name
      `Laskenta.cfg` as the file name. The file must include at least the following lines:  
        
      - On the office computer
          
         `KUNTOSUUNNISTUS=3`
          
          
        - On the pre-reading computer
            
           `ESILUENTA=KUNTO
            
          LUKIJA=n`
            
            
          - On the punch check computer
              
             `KUNTOSUUNNISTUS=3
              
            LUKIJA=n
              
            EMITALKU=aaa`  
      where the number in the parameter `LUKIJA=n` must be the number of the serial port used, i.e. `LUKIJA=1`
      if `COM1` is used. The parameter EMITALKU
      is needed when there are several punch check points. Each point must
      use a different starting value aaa. The values used should be chosen from the range 1000
      - 30000 in order to avoid the value 200 of faulty cards and the number ranges otherwise
      used by cards.
        
        
      In addition, the parameters required for data transfer must be given.
        
        
      - Records are created for each participant, following the number ranges of the possible bib numbers
        in use, while the program is in preparation mode, with *Entries / Add vacants*.
        The records are then marked as Not started. In addition, extra vacancies are created; a small number of them
        is needed in basic operation, and they also serve as spare places if such are ever needed.
        These vacancies are created outside the number range in use and marked as vacant.
        It is advisable to leave the start time open for everyone.
          
          
        - A person database may be prepared for use in the competition office on the basis of earlier participants.
          The file is named `henkilot.csv`.
          The first line of the file is
            
            
          `KilpId;Sukunimi;Etunimi;Seura;Maa;Joukkue;Sarja;Badge`  
            
          and each other line contains, according to that header line and separated by semicolons, at least the following data
          (missing data are left out, but there must be the correct number of semicolons):
            
          - The competitor's identification code, which is used for identifying
            persons and which is saved in the licence data field. Essential
            for continuous maintenance of the file.- Last name, at most 24 characters- First name, at most 24 characters- Club or organisation, at most 31 characters- Country. Not usually used, 3 characters- Team. Not usually used, at most 15
                      characters- Class. Not essential. Used as the default if
                        it is the same as in the class data. Makes the work a little easier.- Emit card code. An essential aid when the card is used by one or at most a few persons.
                          Should not be saved unless the person uses the same card repeatedly.  
          - A club database is prepared if you want to use one. Each line of the database contains the following
            data separated by tabs, semicolons or spaces:
            - District code. A numeric value, entered as 0 unless
              there is another value.- Abbreviation. An abbreviation of at most 15 characters that
                does not contain spaces or semicolons. Not usually used, but some
                text must be present.- Club name. At most 31 characters.  
            - The files
              `KilpSrj.xml ja KILP.DAT` are copied to each computer into the competition's
              working directory, where a configuration file adapted for each computer is also placed.
              The file `radat1.xml` is also copied to the punch check computer.
              The files `henkilot.csv ja Seurat.csv` are placed
              on the competition office computer, if they have been prepared.
                
                
              - If you want the program to start automatically in operating mode
                - start the program in the desired mode- save the windows with *Files / Save windows*- in *Files / Startup settings*
                      - Select the configuration file- Include in it the line `ikkunat`, to which the file name is added
                          if the windows were saved in a file other than `ikkunat.xml`. Any changes must
                          be saved.- Select *Open directly in results service* and save the choices.- When you want to stop the automatic startup, select
                              *Open to main menu* to be saved

### A5.3 Operation during the event

### A5.3.1 Competition office

Start the program. Switch to results service mode and select the directory
and configuration file in use. Then select
*Entries / Entries* from the menu.

The competitors' data are entered by selecting the record to be processed by bib number and
recording at least the name and, if desired, the club. The class can also be entered, but
it is not necessary. The competitor is left as *Not started*. If it is possible to use
a person database and perhaps also a club list, this speeds up the work considerably.

### A5.3.2 Pre-reading

Start the program. Switch to results service mode and select the directory
and configuration file in use, unless these operations have been automated as described
above. Then select *Results service / Emit reading* from the menu,
unless this also happens automatically. When a participant arrives, the card is read
and the competitor's number is entered, which can also be done by reading a barcode. When *Enter* is pressed
in the number field, the competitor is marked as open and the time of day of the event is recorded
in the start time field. The start time serves only as information for monitoring who is still in the forest.

The registration can be cancelled if this is necessary for some reason.

If the card is already in use, the program points this out and lets you either
leave the card bound to the previously registered participant or release it for new use.

### A5.3.3 Punch check

When a normally processed competitor arrives at the punch check, the program calculates their time
and shows the punches. If the punches correspond to some course, the program determines the class on the basis of the course.

If the competitor's card was not read at pre-reading, the data are recorded in a vacancy place, from which they must be
moved to the participant concerned using the button *Change competitor or edit punch data*,
then entering the number or searching by name, and finally confirming the entry with the buttons *Move punches
to selected competitor* and *Save the move*. After using the first-mentioned button,
the competitor's data are shown on the punch check form but have not yet been saved.

### A5.3.4 Viewing results

Results can be viewed in the main menu item *Monitoring / Standings*.

Entry data can also be viewed and edited with *Entries / Entries*,
which opens a table.

In the tables of both of the above options you can click a participant to get more information.
The information displayed is not identical, but in some respects complementary.

### A5.4 Operation after the event

### A5.4.1 Results lists

Results lists to an HTML file or on paper can be produced with *Outputs*, where
you must select the classes to be output and the type of results list desired.

### A5.4.2 Maintaining the databases

The person database file can be supplemented on the office computer by opening the function in which it is shown and then selecting
*Edit / Add competitors to database*.

If a MySQL database is in use, the entry data of the event can be transferred there with
*Files / Write to transfer file*. More instructions in chapter 11.
