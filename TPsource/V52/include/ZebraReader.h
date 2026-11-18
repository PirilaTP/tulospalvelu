// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
//
// ZebraReader: IRfidReader-toteutus Zebra FX9600 -lukijalle.
//
// FX9600 puhuu LLRP-protokollaa (Low Level Reader Protocol, TCP-portti 5084).
// Toteutus rakennetaan projektin oman raa'an TCP-kerroksen (wincom.h / Winsock)
// paalle ILMAN ulkoisia kirjastoja. Lukija valitaan konfiguraation
// ZEBRA=-parametrilla (vrt. FX9500: SIRIT=).
//
// TILA (Osa A): yhteyden avaus/sulku ja luokan runko toteutettu. Varsinainen
// LLRP-sanomavirta (ROSpec-maaritys, RO_ACCESS_REPORT-parsinta, KEEPALIVE) ja
// tagien muunnos aikatapahtumiksi toteutetaan Osa B:ssa.
//
// Kaannetaan vain LAJUNEN-kokoonpanossa, kuten muukin RFID-lukijakoodi.

#ifndef ZEBRAREADER_H
#define ZEBRAREADER_H

#ifdef LAJUNEN

#include "IRfidReader.h"

class ZebraReader : public IRfidReader {
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

#endif // ZEBRAREADER_H
