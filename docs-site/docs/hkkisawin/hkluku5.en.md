# Chapter 5. Timing functions

## 5 Timing functions

HkKisaWin can use
a finish clock or chip systems capable of timing,
such as Emit punches, the emiTag system and some
RFID devices, but for the time being I recommend using the program
mainly for monitoring and, for the sake of smooth operation, handling the actual
timing with HkMaali. The instructions given in this chapter apply
largely to HkMaali as well.

In the timing functions of HkKisaWin you can easily attach times to competitors, but
HkMaali includes many special recording functions that HkKisaWin does not have.

The timing functions are controlled by many
startup parameters, which are listed in [appendix
1](hkliite1.md). This
chapter describes the use of the most important parameters.

Further instructions on the Alge Timy finish clock are in [appendix 7](liite_7._algen_timy_usb_portissa.md) and
on using the EmiTag system in [appendix 9](liite_9._emitag-laitteiden_kaytto.md) .

The timing functions of the program use a separate
file, typically named AJAT1.LST, where
1 refers to the first competition stage. The last character may also vary
when times are taken into several queues.

Parameters common to all timing methods include,
among others,

```
      AJAT=/S  
LÄHAIKA1
```

of which the first requests that the timing data be kept
without a confirmation question and the second that the timing data be sent to connection 1.
Timing data does not mean the result obtained by the competitor, which is always transferred when
the connection is open for sending, but a separate table that is used,
for example, for monitoring and processing times received from the finish
clock.
