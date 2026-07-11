// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
//
// Yhteinen rajapinta RFID-lukijoille.
//
// Tausta: ohjelma tukee Zebra FX9500 -lukijaa (koodissa nimellä SIRIT). Tuki
// uudemmalle FX9600:lle (ZEBRA=) lisataan myohemmin omana toteutuksenaan taman
// saman rajapinnan taakse. Talla ensimmaisella askeleella SIRIT-logiikkaa EI
// muuteta - olemassa olevat funktiot (openSirit2, SiritSync, lue_SiritCmd,
// siritaika) sailyvat ennallaan, ja SiritReader vain delegoi niihin.
//
// HUOM includejarjestys: tama otsikko kayttaa tyyppeja san_type ja aikatp,
// jotka maaritellaan sovelluskohtaisesti (HkDef.h / VDef.h). Sisallyta tama
// otsikko VASTA sovelluksen Declare/Def-otsikoiden ja TpLaitteet.h:n jalkeen,
// aivan kuten TpLaitteet.h itse olettaa nama tyypit jo maaritellyiksi.

#ifndef IRFIDREADER_H
#define IRFIDREADER_H

// Abstrakti lukijarajapinta. Metodit ottavat r_no-indeksin, koska ohjelman
// nykyinen tilamalli pohjautuu r_no:lla indeksoituihin globaaleihin
// taulukoihin; tata mallia ei tassa vaiheessa muuteta.
class IRfidReader {
public:
	virtual ~IRfidReader() {}

	// Avaa lukijan TCP-yhteyden. reconnect=true sulkee ensin vanhat yhteydet.
	// Palauttaa 0 onnistuessaan, muuten virhekoodin (vrt. openSirit2).
	virtual int openConnection(int r_no, bool reconnect) = 0;

	// Onko lukijan yhteys auki (vrt. TCPyht_on).
	virtual bool isConnected(int r_no) = 0;

	// Lukee ja käsittelee lukijalta saapuvat tagitapahtumat (vrt. lue_regnly).
	virtual void readTags(int r_no) = 0;

	// Synkronoi lukijan kellon. setTime=true asettaa ajan, false vain kysyy.
	virtual void sync(bool setTime) = 0;

	// Lukee lukijan komentokanavan vastauspuskurin (vrt. lue_SiritCmd).
	virtual void readCmd(int r_no) = 0;

	// Jasentaa lukijalta saadun sanoman aikatapahtumaksi (vrt. siritaika).
	// Palauttaa 0 onnistuessaan tai 1 jos sanomaa ei kasitella.
	virtual int parseTime(INT32 *t, san_type *vastaus, aikatp *ut,
		INT *jono, int r_no) = 0;
};

// Tehdas: palauttaa r_no:lle sopivan lukijatoteutuksen regnly[r_no]-arvon
// perusteella (nyt vain SIRIT). Instanssit luodaan tarvittaessa ja
// valimuistitetaan. Palauttaa NULL, jos r_no:lle ei ole RFID-lukijaa.
// Tahan lisataan myohemmin ZEBRA (FX9600) -haara.
IRfidReader *getRfidReader(int r_no);

#endif // IRFIDREADER_H
