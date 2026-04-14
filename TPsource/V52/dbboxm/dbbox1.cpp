#include <windows.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <boxm.h>
#include <tputil.h>
#include "dbint.h"
#define TRUE 1
#define FALSE 0
#define MAXNBLOCK 1024
#define UNDERSCORE  =  '_'

void lopetus(void);

extern int maaliajat[];
extern short minkilpno, maxkilpno;
//extern int ok;
int __dbbox__ok;
static tarecordbuffer *tarecbuf;
static unsigned int tarecbufsize;
int ainauusirec = 1;

#ifdef _BORLAND_
// Palauttaa kahden kokonaisluvun pienemmän (Borland-versiossa puuttuvan std::min korvaaja).
static int min(int i1, int i2)
{
	return((i1 < i2) ? i1 : i2);
}
#endif

// Pakkaa n kokonaislukua buf-puskurissa 32-bitistä 16-bittiseen muotoon (lossless truncation).
// Käytetään tiedostorakenteen otsikkotietueen tiivistämiseen levylle kirjoitettaessa.
void tacompress(void *buf, int n)
{
	int i;
	unsigned *pint;
	unsigned short *pshort;

	pint = (unsigned *)buf;
	pshort = (unsigned short *) buf;
	for (i = 0; i < n; i++, pshort++, pint++) *pshort = (unsigned short) *pint;
	for (i = 0; i < n; i++, pshort++) *pshort = 0;
}

// Laajentaa n tiivistettyä 16-bittistä arvoa buf-puskurista takaisin 32-bittisiksi.
// Arvo 0xffff tulkitaan 0xffffffff:ksi (vapaan listan loppumerkki).
void taexpand(void *buf, int n)
{
	int i;
	unsigned *pint;
	unsigned short *pshort;

	pint = (unsigned *)buf;
	pshort = (unsigned short *) buf;
	for (i = n-1; i >= 0; i--) {
		if (pshort[i] == 0xffff) pint[i] = 0xffffffff;
		else pint[i] = pshort[i];
	}
}

// Lukee tietueen r tiedostosta datf puskuriin buffer.
// Palauttaa 0 onnistuessaan, -1 virhetilanteessa (perr() kirjaa virheen).
int getrec(datafile *datf, DATAREF r, void *buffer)
{
unsigned long ls, lrecl;

  lrecl = r * datf->recl;
  ls = SetFilePointer(datf->hDatf, lrecl, NULL, FILE_BEGIN);
  if (ls != lrecl) {
	 perr(datf->flnm, r, "getrec", GetLastError());
	 return(-1);
  }
  ReadFile(datf->hDatf, buffer, datf->recl, &ls, NULL);
  if (ls < datf->recl){
	 perr(datf->flnm, r, "getrec", GetLastError());
	 return(-1);
  }
  return(0);
}

// Kirjoittaa puskurin buffer tiedostoon datf tietueelle r.
// Palauttaa 0 onnistuessaan, -1 virhetilanteessa.
int putrec(datafile *datf, DATAREF r, void *buffer)
{
unsigned long ls, lrecl;

  lrecl = r * datf->recl;
  ls = SetFilePointer(datf->hDatf, lrecl, NULL, FILE_BEGIN);
  if (ls != lrecl) {
	 perr(datf->flnm, r, "putrec", GetLastError());
	 return(-1);
  }
  WriteFile(datf->hDatf, buffer, datf->recl, &ls, NULL);
  if (ls < datf->recl){
	 perr(datf->flnm, r, "putrec", GetLastError());
	 return(-1);
  }
  return(0);
}

// Luo uuden tietokantatiedoston fname tietuepituudella reclen ja alustaa datafile-rakenteen datf.
// Varaa tarecbuf-puskurin tarvittaessa; palauttaa 0 onnistuessaan, 1 epäonnistuessaan.
int makefile(datafile *datf, char *fname, unsigned reclen)
{
	if (tarecbufsize && tarecbufsize < reclen) {
		tarecbufsize = 0;
		free(tarecbuf);
		}
	if (tarecbufsize == 0) {
		tarecbufsize = reclen;
		tarecbuf = (tarecordbuffer *) calloc(tarecbufsize, 1);
		}
	else
		memset(tarecbuf, 0, tarecbufsize);
	datf->hDatf = CreateFile(fname, GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);

	if(datf->hDatf == INVALID_HANDLE_VALUE) {
		__dbbox__ok = FALSE;
		perr(fname, 0, "makefile", GetLastError());
	}
	else {
		datf->recl = reclen;
      datf->firstfree = 0xFFFFFFFF;
		datf->numberfree = 0;
		datf->int1 = 0;
		datf->int2 = 0;
		memmove(tarecbuf,&(datf->firstfree),4*sizeof(DATAREF));
		tacompress(tarecbuf, 4);
		putrec(datf,0,tarecbuf);
		datf->numrec = 1;
		datf->flnm = fname;
		__dbbox__ok = TRUE;
	}
	return(!__dbbox__ok);
}

// Kirjoittaa datafile-rakenteen otsikkotiedot (firstfree, numberfree, numrec, maaliajat) tiedostoon.
// Kutsutaan aina kun tietueiden lukumäärä tai vapaan listan tila muuttuu.
static void updatedescr(datafile *datf)
{
	datafile df;

  if (datf->recl < 8) return;
  df.firstfree = datf->firstfree;
  df.numberfree = datf->numberfree;
  df.recl = datf->recl;
  df.numrec = datf->numrec;
  df.int1 = datf->int1;
  df.int2 = datf->numrec;
  memmove(tarecbuf,&df.firstfree,4*sizeof(DATAREF));
  memcpy(&tarecbuf->ii.tm, maaliajat, min(datf->recl-4*sizeof(DATAREF),40));
  tacompress(tarecbuf, 4);
  putrec(datf,0,tarecbuf);
}

// Avaa olemassa olevan tietokantatiedoston fname tietuepituudella reclen.
// Lukee otsikkotietueen, tarkistaa eheyden ja täyttää datf-rakenteen. Palauttaa 0 ok, 1 virhe.
int openfile(datafile *datf, char *fname, unsigned reclen)
   {
   unsigned flen;
   DWORD Err = 0;
   char msg[81];

	__dbbox__ok = FALSE;
	if (tarecbufsize && tarecbufsize < reclen) {
		tarecbufsize = 0;
		free(tarecbuf);
		}
	if (tarecbufsize == 0) {
		tarecbufsize = reclen;
		tarecbuf = (tarecordbuffer *)calloc(tarecbufsize, 1);
		}
	else
		memset(tarecbuf, 0, tarecbufsize);
	datf->hDatf = CreateFile(fname, GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
	if (datf->hDatf == INVALID_HANDLE_VALUE) {
		if ((Err = GetLastError()) != ERROR_FILE_NOT_FOUND) {
			perr(fname, 0, "openfile", Err);
		}
	}
	else {
		if((flen = GetFileSize(datf->hDatf, NULL)) >= reclen) {
			datf->recl = reclen;
			getrec(datf,0,tarecbuf);
			taexpand(tarecbuf, 4);
			memmove(&(datf->firstfree),tarecbuf,4*sizeof(DATAREF));
			memcpy(maaliajat, &tarecbuf->ii.tm, min(reclen-4*sizeof(DATAREF),40));
			datf->numrec = flen / reclen;
			datf->flnm = fname;
			if (datf->int2 == datf->numrec -1)
				datf->int2 = datf->numrec;
			if ((ainauusirec || ((datf->firstfree == 0xffffffff || (datf->numberfree > 0 &&
				(datf->firstfree > 0 && datf->firstfree < datf->numrec)))))
				&& reclen * datf->numrec == flen
				&& datf->int2 == datf->numrec) {
				__dbbox__ok = TRUE;
				}
		}
		if (!__dbbox__ok) {
			CloseHandle(datf->hDatf);
			sprintf(msg, "Tiedoston %s otsikkoalue virheellinen", fname);
			writeerror(msg, 0);
		}
	}
	return(!__dbbox__ok);
}

// Sulkee tietokantatiedoston datf: kirjoittaa otsikkotietueen ja sulkee tiedostokahvan.
void closefile(datafile *datf)
{
  if (datf->recl < 8) return;
  datf->int2 = datf->numrec;
  memmove(tarecbuf,&datf->firstfree,4*sizeof(DATAREF));
  memcpy(&tarecbuf->ii.tm, maaliajat, min(datf->recl-4*sizeof(DATAREF),40));
  tacompress(tarecbuf, 4);
  putrec(datf,0,tarecbuf);
  CloseHandle(datf->hDatf);
}

// Varaa uuden tietuepaikan tiedostosta datf: käyttää vapaan listan alkiota tai lisää tiedoston loppuun.
// Kirjoittaa varatun tietueindeksin *r:ään.
void newrec(datafile *datf, DATAREF *r)
{
  if(ainauusirec || datf->firstfree == 0xffffffff )
  {
    *r = datf->numrec++;
  }
  else
  {
    *r = datf->firstfree;
    getrec(datf,*r,tarecbuf);
    datf->firstfree = tarecbuf->ii.i & 0xffff;
    if (datf->firstfree == 0xffff)
       datf->firstfree = 0xffffffff;
    datf->numberfree--;
  }
}

// Lisää uuden tietueen buffer tiedostoon datf; kirjoittaa otsikon päivityksen (updatedescr).
// Kirjoittaa varatun indeksin *r:ään.
void addrec(datafile *datf, DATAREF *r, void *buffer)
{
  newrec(datf,r);
  putrec(datf,*r,buffer);
  updatedescr(datf);
}

// Lisää uuden tietueen buffer tiedostoon datf ilman otsikon päivitystä (nopeampi kuin addrec).
// Kirjoittaa varatun indeksin *r:ään.
void addrec0(datafile *datf, DATAREF *r, void *buffer)
{
  newrec(datf,r);
  putrec(datf,*r,buffer);
}

// Merkitsee tietueen r poistetuksi lisäämällä sen vapaaseen listaan datf-tiedostossa.
void deleterec(datafile *datf, DATAREF r)
{
   tarecbuf->ii.i = datf->firstfree;
   putrec(datf,r,tarecbuf);
   datf->firstfree = r;
   if (datf->firstfree == 0xffff)
      datf->firstfree = 0xffffffff;
   datf->numberfree++;
}

// Palauttaa tiedoston datf tietueiden kokonaismäärän (ml. otsikko- ja poistetut tietueet).
DATAREF  filelen(datafile *datf)
{
  return(datf->numrec);
}

// Palauttaa aktiivisten (ei poistettujen) tietueiden lukumäärän tiedostossa datf.
DATAREF usedrecs(datafile *datf)
{
  return(datf->numrec - datf->numberfree - 1);
}


