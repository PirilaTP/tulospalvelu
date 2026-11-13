# Description of the example competitions

### Results service for a ski competition - examples

Pekka Pirilä, 12.2.2015

This guide goes through the tasks involved in organising
a few different kinds of ski competitions.

The competition types covered are:

1. A small event
   run on a single computer

   - A typical national competition with computers at the finish,
     in the results office and with the announcer.

     - Additional features of a pursuit competition compared with the previous one.

       - Cross-country sprint

The last chapter of these instructions is a detailed
list of the steps required for the results service of a very small
competition.

Before starting these steps, the software must be installed, and this
is assumed to have been done according to the installation instructions.

The program is used in three different phases:

- The competition is created and
  the basic settings are made before moving on to the other phases

  - Classes are defined in preparation mode. Entering competitors and the draw are also carried out in this mode,
    unless competitors are added during the event itself.

    - The results service itself is run in results service mode. In this mode you can also
      add competitors who enter during the event.

After making the basic settings, it is best to move straight to preparation mode. You cannot move from preparation mode
to results service mode other than by exiting the program in between.

In case 1 there are a few dozen competitors,
who may enter at the venue. Timing uses the computer's clock.

In case 2 the competition under consideration is expected to have a couple of
hundred competitors. Timing uses a finish clock. Competitors' numbers are entered
on the computer by hand alongside the time received from the clock. A
start gate is also used. The results service itself also uses a second
computer for recording various changes and for producing
outputs. A third computer is with the announcer for following the competition.

For cases 3 and
4, the additional tasks involved in preparing and
running the competition are covered. Otherwise the cases correspond to case 2.

A small event as in case 1 contains at least
the following
steps:

- [Setting up
  the competition and making the basic settings](perusvalinnat.md)

  - [Defining the classes](sarajatiedot.md)
    in preparation mode

    - Running
      the event itself
      in results service mode

      - [Entering competitors](ilmoittautumisten_syotto.md)
        while also giving start times and
        numbers in the function *Entries / Entries*

        - [Timing](ajanotto.md)
           using the computer's
          clock in the function *Results service / Timing*

          - [Producing the results
            lists](tulosluettelot.md)
            for a printer and/or
            a file

Even a small event may also require
recording entries in advance, carrying out a draw, and preparing
configuration settings for the results service phase, for example for using a start clock.
