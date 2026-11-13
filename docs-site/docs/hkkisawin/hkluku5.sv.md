# Kapitel 5. Tidtagningsfunktioner

## 5 Tidtagningsfunktioner

Programmet HkKisaWin kan användas med
en målklocka eller tagg-system som klarar tidtagning,
såsom Emit-stämplare, emiTag-systemet och vissa
RFID-enheter, men tills vidare rekommenderar jag att programmet används
främst för uppföljning och att själva tidtagningen av smidighetsskäl
sköts med programmet HkMaali. Anvisningarna i detta kapitel gäller
till stor del även programmet HkMaali.

I tidtagningsfunktionerna i HkKisaWin kan tider enkelt kopplas till tävlande, men
programmet HkMaali innehåller ett stort antal specialfunktioner för registrering som inte finns i HkKisaWin.

Tidtagningsfunktionerna styrs med många
startparametrar, som förtecknas i [bilaga
1](hkliite1.md). I detta
kapitel beskrivs användningen av de viktigaste parametrarna.

Ytterligare anvisningar om målklockan Alge Timy finns i [bilaga 7](liite_7._algen_timy_usb_portissa.md) och
om användningen av EmiTag-systemet i [bilaga 9](liite_9._emitag-laitteiden_kaytto.md) .

Programmets tidtagningsfunktioner använder en separat
fil, som typiskt heter AJAT1.LST, där
1 syftar på tävlingens första etapp. Även det sista tecknet kan variera
när tider tas i flera köer.

Parametrar som är gemensamma för alla tidtagningssätt är
bl.a.

```
      AJAT=/S  
LÄHAIKA1
```

av vilka den första begär att tidtagningsdata ska behållas
utan bekräftelsefråga och den senare att tidtagningsdata ska skickas till förbindelse 1.
Tidtagningsdata avser inte den tävlandes resultat, som alltid överförs när
förbindelsen är öppen för sändning, utan en separat tabell som används
till exempel för att följa upp och behandla tider som kommit
från målklockan.
