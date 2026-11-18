# Appendix 3. Fitness orienteering model 1

## Appendix 3. Fitness orienteering model 1

### A3.1 Basic description of the operating mode

Fitness orienteering operating mode 1 is enabled by giving
the program the startup parameter

`KUNTOSUUNNISTUS=1`

You then get to fitness orienteering mode by opening the program in results service mode and selecting
"Results service/Emit reading" from the main menu. Alternatively, you can define the program to open
directly in results service mode and to open the Emit reading form on startup.

In operating mode 1 the competitor file does not initially contain data on the participants; instead,
their data are in a separate list of possible participants (person database).
The competitor file must then contain so many vacancy places that all
participants will certainly fit in.

In operating mode 1, registering as a starter and the punch check take place at different readouts.
When a card is read for the first time, the program marks the competitor as started and saves
the Emit code in the participant's data. When the Emit card is read a second time, the result is saved
and the Emit code is changed to a free value from the range of small numbers. Thus the third readout
is again the registration of a new competitor, in exactly the same way as
the first readout. Registration and punch check can take place
on the same or on different computers, provided that there is a data transfer connection between the computers.

The result can, however, be saved already at the first readout, provided that
the punches give an accepted performance for some course. Otherwise a second readout is needed,
and some other card must be read between the readouts.

### A3.2 Preparations for operating mode 1

The preparation includes the following steps:

- Defining the competition
  - The file folder to be used is selected and created either outside the program or in the function
    *Competition creation and basic properties*- The classes of the competition are created and named either in the above-mentioned
      function or by copying a suitable earlier KilpSrj.xml file to the competition's
      file folder before defining the basic competition data. The classes must
      include an extra class for vacancy records. The program tries
      to create a suitable vacancy class automatically.- In the basic properties you should define the title data and give the competition an identification code that
        differs from other competitions.  
  - Defining the courses in the program's menu item *Preparation / Course data*
    .
    For each class you must define a course whose name is the same as the class name. As the reset advance,
    define *Reset at start time*, and for the finish
    you must select *Punching unit on finish line (not online*). Defining a course
    is also recommended for the vacancy class to eliminate disturbing
    error messages. The course of the vacancy class
    includes at least one control that is not in the forest and that is given
    a control code that is not in use either, so that nobody can get an accepted
    performance in the vacancy class.
      
      
    - Creating the configuration file. It is simplest to use the default name
      `Laskenta.cfg` as the file name. The file must include at least the following lines:
        
       `KUNTOSUUNNISTUS=1
        
      LUKIJA=n  
      VAKANTTIALKU=kkk`

        
      where the number in the parameter `LUKIJA=n` must be the number of the serial port used, i.e. `LUKIJA=1`
      if `COM1` is used. The VAKANTTIALKU parameter is needed when there are several
      punch check points.
        
        
      In addition, the parameters required for data transfer must be given if several connected computers are used.
      In that case the vacancy ranges of the different computers must also be defined with the parameter `VAKANTTIALKU=nnnn` so that
      each computer certainly has enough vacancies in its own range.
      On one of the computers nnnn is chosen to correspond to the lower limit
      of the number range in use, and on the others the middle of this range.
      Furthermore, you can give the parameter `SEURATIETO=E` if you do not want to handle club data,
      and the parameter `VAHVISTAAINA=E`
      if you want the program to accept a participant without confirmation when
      only one alternative is offered.   
        
      The parameter KUNTOLÄHTÖ is suitable for use on the computer at the registration
      point if you are sure that only registering participants will come to that point. When this
      parameter is used, all extra information has been removed from the form.
      Through the menu it is possible to enable this mode and to return to the normal
      punch check display.  
        
      The parameter KUNTOMAALI makes the program prepare for
      the possibility that an orienteer arrives at the finish after completing the course even though
      the card was not registered at the start. The program then asks whether this is a start
      or a finish. Without this parameter the program assumes that all those
      whose card has not yet been read and who do not have an
      accepted performance for some course are starters. If an accepted
      performance is found on the card, the program asks in any case whether this is a finisher.
        
        
      The way of using the parameters LÄHEMITx=V and LÄHEMITy=I presented in chapter 6.6.4
      may make it easier to finalise the data collected at the event for control split time printouts.
        
        
      - Vacancy places are created while the program is in preparation mode with *Entries / Add vacants*.
        It is advisable to create plenty of vacancies, and so that after each parameter `VAKANTTIALKU=nnnn`
        there are more vacancies than the maximum number of participants that can register at one computer.
        The vacancies are created as vacant in the class reserved for vacancies, leaving the last name and club empty.
          
          
        The same file containing vacancies can be used
        in several different events if the number of classes and the position
        of the vacancy class in the class list are the same in all of them.   
          
        The number of vacancies
        created automatically by the program is controlled with the parameter VAKANTTILUKU=nnn in the preparation stage's
        configuration file ilmoitt.cfg or in the results service stage's
        configuration file. The parameter LISÄVAKANTTIALKU=nn
        in this file gives the starting point of the vacancy number range. Without this
        parameter the vacancies are added at the end of the number range.
          
          
        - The person database is prepared on the basis of earlier participants. The file is named `henkilot.csv`.
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
            - All files created in the previous steps,
               `KilpSrj.xml, KILP.DAT, radat1.xml, henkilot.csv, Seurat.csv`, are copied to each computer into the competition's
              working directory, where a configuration file adapted for each computer
              is also placed.
                
                
              - If you want the program to start automatically in operating mode
                - start the program in the desired mode- save the windows with *Files / Save windows*- in *Files / Startup settings*
                      - Select the configuration file- Include in it the line `ikkunat`, to which the file name is added
                          if the windows were saved in a file other than `ikkunat.xml`. Any changes must
                          be saved.- Select *Open directly in results service* and save the choices.- When you want to stop the automatic startup, select
                              *Open to main menu* to be saved

### A3.3 Operation during the event

Start the program. Switch to results service mode and select the directory
and configuration file in use, unless these operations have been automated as described
above. Then select *Results service / Emit reading* from the menu, unless
this also happens automatically.

When a participant arrives at the start, the card is read. The following can then happen:

- If the Emit code is found in the person database for one participant, the program asks
  whether the person is correct and saves them as a participant if you answer *Yes*.
  The answer *No* leads to the same branch as a card that is not in the database.
  This confirmation request can be removed with a parameter or from the menu.  
  - If the Emit code is found in the person database for several participants, the program asks you
    to select the correct person using the *up arrow / down arrow* keys. When
    the correct person is selected, press *Enter* and then *Ctrl-Enter*, or
    confirm with the *Save* button.  
    - If the Emit code is not found in the database, the program moves to the last name field, where
      you start typing the name. If the name is found in the database, you can select it when
      the alternatives have been reduced enough, using the *Enter* key or browsing
      before that with the *up arrow / down arrow* keys. When the name is correct, check
      the club and class if desired, and accept with *Ctrl-Enter* or
      the *Save* button.- If the participant's card contains data that correspond to
        an accepted performance on some course in use, the program asks whether
        this is a registration or a finish.- If a competitor arrives at the finish without prior registration and with an error
          in the punches, the card must be read again after some other card has been read in between.
          To keep the card from switching off before the new readout, it must be punched soon after the first readout
          with some unit other than the finish unit.

When a previously registered participant arrives at the finish, the card is read. The program determines
the result immediately, shows the punches and deduces the course from the punches. Usually nothing else needs to be
done, but the data can be edited if they are not correct. When editing,
the *Save* button appears as a sign that the changes must be saved,
either with the button or with *Ctrl-Enter*.

Previously read results can be displayed by browsing with the navigation bar or by using
the *Find participant* button and the search options that appear.
Data brought to the screen this way can also be changed and the changes saved.

### A3.3.1 Viewing results

Results can be viewed in the main menu item *Monitoring / Standings*.

Entry data can also be viewed and edited with *Entries / Entries*,
which opens a table.

In the tables of both of the above options you can click a participant to get more information.
The information displayed is not identical, but in some respects complementary.

### A3.4 Operation after the event

### A3.4.1 Results lists

Results lists to an HTML file or on paper can be produced with *Outputs*, where
you must select the classes to be output and the type of results list desired.

### A3.4.2 Maintaining the databases

The person database file can be supplemented by opening the function in which it is shown and then selecting
*Edit / Add competitors to database*.

If a MySQL database is in use, the entry data of the event can be transferred there with
*Files / Write to transfer file*. More instructions in chapter 11.
