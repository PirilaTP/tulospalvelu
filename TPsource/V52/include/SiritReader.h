// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
//
// SiritReader: IRfidReader-toteutus Zebra FX9500 (SIRIT) -lukijalle.
//
// Tama on ohut kaare: jokainen metodi delegoi suoraan olemassa olevaan
// SIRIT-funktioon (openSirit2, SiritSync, lue_SiritCmd, siritaika).
// SIRIT:n toimintaa EI muuteta - vain reititetaan kutsut rajapinnan kautta,
// jotta FX9600 voidaan myohemmin lisata rinnalle samaan rajapintaan.
//
// Kaannetaan vain LAJUNEN-kokoonpanossa, koska delegoitavat SIRIT-funktiot
// ovat olemassa vain silloin.

#ifndef SIRITREADER_H
#define SIRITREADER_H

#ifdef LAJUNEN

#include "IRfidReader.h"

class SiritReader : public IRfidReader {
public:
	virtual int openConnection(int r_no, bool reconnect);
	virtual bool isConnected(int r_no);
	virtual void readTags(int r_no);
	virtual void sync(bool setTime);
	virtual void readCmd(int r_no);
	virtual int parseTime(INT32 *t, san_type *vastaus, aikatp *ut,
		INT *jono, int r_no);
};

#endif // LAJUNEN

#endif // SIRITREADER_H
