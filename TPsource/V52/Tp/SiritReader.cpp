// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
//
// SiritReader-toteutus ja RFID-lukijatehdas.
//
// Ohut delegoiva kaare: metodit kutsuvat suoraan olemassa olevia
// SIRIT-funktioita, joiden runkoja ei muuteta. Nain SIRIT (FX9500) toimii
// tasmalleen kuten ennenkin, mutta kutsut kulkevat IRfidReader-rajapinnan
// kautta - tama on laajennuspiste tulevalle FX9600 (ZEBRA=) -tuelle.

#include <stdio.h>
#include <stdlib.h>

#ifndef MAXOSUUSLUKU
#include "HkDeclare.h"
#else
#include "VDeclare.h"
#endif
#include "TpLaitteet.h"
#include "IRfidReader.h"
#include "SiritReader.h"
#include "ZebraReader.h"

#include <wincom.h>

#ifdef LAJUNEN

// SiritSync ei ole TpLaitteet.h:ssa (sen prototyyppi on siella kommentoituna),
// joten esitellaan se tassa. Maaritelma on TpLaitteet.cpp:ssa.
void SiritSync(bool set_time);

// --- SiritReader: delegointi olemassa oleviin SIRIT-funktioihin ---

int SiritReader::openConnection(int r_no, bool reconnect)
{
	return openSirit2(r_no, reconnect);
}

bool SiritReader::isConnected(int r_no)
{
	return TCPyht_on(hComm[cn_regnly[r_no]]) != 0;
}

void SiritReader::readTags(int r_no)
{
	lue_regnly(r_no);
}

void SiritReader::sync(bool setTime)
{
	SiritSync(setTime);
}

void SiritReader::readCmd(int r_no)
{
	lue_SiritCmd(r_no);
}

int SiritReader::parseTime(INT32 *t, san_type *vastaus, aikatp *ut,
	INT *jono, int r_no)
{
	return siritaika(t, vastaus, ut, jono, r_no);
}

#endif // LAJUNEN

// --- Tehdas ---

// Valimuistitetut lukijaoliot r_no:n mukaan. Prosessin elinkaaren mittaiset.
static IRfidReader *g_rfidReaders[NREGNLY] = { 0 };

// Palauttaa r_no:lle sopivan lukijan regnly[r_no]-arvon perusteella ja luo sen
// tarvittaessa. Tahan lisataan myohemmin LID_ZEBRA -> ZebraReader.
IRfidReader *getRfidReader(int r_no)
{
	if (r_no < 0 || r_no >= NREGNLY)
		return 0;
	if (g_rfidReaders[r_no] == 0) {
		switch (regnly[r_no]) {
#ifdef LAJUNEN
		case LID_SIRIT:
			g_rfidReaders[r_no] = new SiritReader();
			break;
		case LID_ZEBRA:
			g_rfidReaders[r_no] = new ZebraReader();
			break;
#endif
		default:
			return 0;
		}
	}
	return g_rfidReaders[r_no];
}
