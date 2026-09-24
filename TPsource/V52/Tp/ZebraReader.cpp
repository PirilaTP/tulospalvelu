// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
//
// ZebraReader-toteutus: Zebra FX9600 -lukija LLRP-protokollalla (portti 5084),
// projektin oman raa'an TCP-kerroksen (wincom.h) paalla, ilman ulkoisia
// kirjastoja. Lukija valitaan konfiguraation ZEBRA=-parametrilla.
//
// Toimintaperiaate:
//  - openConnection avaa TCP-yhteyden ja kaynnistaa jatkuvan inventaarion
//    LLRP-sanomilla (DELETE_ROSPEC, ADD_ROSPEC, ENABLE_ROSPEC, START_ROSPEC,
//    ENABLE_EVENTS_AND_REPORTS).
//  - readTags lukee LLRP-sanomat, vastaa KEEPALIVE:en ja jasentaa
//    RO_ACCESS_REPORT-sanomista tagit (EPC + antenni + aikaleima).
//  - Kukin tagi muotoillaan samaan tekstimuotoon kuin FX9500/SIRIT
//    ("tag_id=0x.. first=.. antenna=..") ja syotetaan tall_regnly:lle, jolloin
//    EPC -> kilpailijanumero -muunnos ja ajankasittely toimivat samalla
//    koodilla kuin SIRIT (parseTime delegoi siritaika:lle).
//
// Huom: tagien aikaleima otetaan lukijan FirstSeenTimestampUTC-kentasta, joten
// FX9600:n kellon tulee olla oikeassa ajassa (NTP). Jos aikaleimaa ei saada,
// kaytetaan PC:n paikallista kelloa.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef MAXOSUUSLUKU
#include "HkDeclare.h"
#else
#include "VDeclare.h"
#endif
#include "TpLaitteet.h"
#include "IRfidReader.h"
#include "ZebraReader.h"

#include <wincom.h>

#ifdef LAJUNEN

// LLRP:n vakioportti, jos konfiguraatiossa ei anneta porttia (ZEBRA=TCP:osoite).
#define LLRP_PORT 5084

// GPI-portti, johon ulkoinen kytkin on johdotettu (FX9600:n laitekonfig).
// Oletus GPI1; asetettavissa konfiguraation ZEBRAGPI=-parametrilla (HkInit.cpp/VInit.cpp).
int ZebraGpiPort = 1;

// GPO-portti, johon summeri on johdotettu (FX9600:n laitekonfig). Uusi tagi
// kaynnistaa GPO1-summeripulssin (paalle GPO_PULSE_MS ms). Toistaiseksi kiintea.
#define GPO_PORT 1

// GPOWriteData-parametrin LLRP-tyyppinumero (kulkee SET_READER_CONFIG:n sisalla).
// Verifioitu kanonisesti: llrp-1x0-def.xml rivi 1289 (typeNum="219").
#define LLRP_GPO_WRITE_DATA 219

// GPO-summeripulssin kesto millisekunteina: uusi tagi sytyttaa summerin, joka
// sammutetaan ei-blokkaavasti taman ajan kuluttua (readTags-silmukan lopussa).
#define GPO_PULSE_MS 500

// LLRP-sanomatyypit (10-bittinen tyyppikentta)
#define LLRP_ADD_ROSPEC                20
#define LLRP_DELETE_ROSPEC             21
#define LLRP_START_ROSPEC              22
#define LLRP_ENABLE_ROSPEC             24
#define LLRP_RO_ACCESS_REPORT          61
#define LLRP_KEEPALIVE                 62
#define LLRP_READER_EVENT_NOTIFICATION 63
#define LLRP_KEEPALIVE_ACK             72
#define LLRP_ENABLE_EVENTS_AND_REPORTS 64
#define LLRP_SET_READER_CONFIG          3
#define LLRP_GET_READER_CONFIG          2
#define LLRP_GET_READER_CONFIG_RESPONSE 12
// FX9600-diagnostiikkalokitus (kytkettävissä loki-lipulla): komentovastausten tyypit
#define LLRP_ADD_ROSPEC_RESPONSE       30
#define LLRP_DELETE_ROSPEC_RESPONSE    31
#define LLRP_ENABLE_ROSPEC_RESPONSE    34
#define LLRP_GET_ROSPECS               26
#define LLRP_GET_ROSPECS_RESPONSE      36

// LLRP-parametrityypit
#define P_ANTENNA_ID            1   // TV, 2 tavua
#define P_FIRST_SEEN_UTC        2   // TV, 8 tavua (mikrosekuntia UTC)
#define P_LAST_SEEN_UTC         4   // TV, 8 tavua (mikrosekuntia UTC)
#define P_PEAK_RSSI             6   // TV, 1 tavu
#define P_CHANNEL_INDEX         7   // TV, 2 tavua
#define P_ROSPEC_ID             9   // TV, 4 tavua
#define P_INV_PARAM_SPEC_ID    10   // TV, 2 tavua
#define P_EPC_96               13   // TV, 12 tavua
#define P_SPEC_INDEX           14   // TV, 2 tavua
#define P_ACCESS_SPEC_ID       16   // TV, 4 tavua
#define P_EPC_DATA            241   // TLV (bittimaara + EPC-tavut)
#define P_TAG_REPORT_DATA     240   // TLV
#define P_READER_EVENT_NOTIF_DATA 246   // TLV (yhteydenavausilmoituksen sisalto)
#define P_UTC_TIMESTAMP       128   // TLV (u64 mikrosekuntia UTC-epookista)
#define P_UPTIME              129   // TLV (u64 mikrosekuntia kaynnistyksesta)
#define P_GPI_PORT_CURRENT_STATE 225   // TLV (GPIPortNum + Config + State)

#define RXSZ 16384

// --- LLRP-sanomien rakennus (big-endian) ---

typedef struct { unsigned char b[600]; int n; } LB;

static void p8(LB *L, unsigned v)  { L->b[L->n++] = (unsigned char)v; }
static void p16(LB *L, unsigned v) { p8(L, v >> 8); p8(L, v); }
static void p32(LB *L, unsigned long v) { p8(L, (unsigned)(v >> 24)); p8(L, (unsigned)(v >> 16)); p8(L, (unsigned)(v >> 8)); p8(L, (unsigned)v); }

// Aloittaa TLV-parametrin (tyyppi + paikanvaraus pituudelle); palauttaa alkukohdan.
static int beginP(LB *L, unsigned type) { int pos = L->n; p16(L, type); p16(L, 0); return pos; }
// Paivittaa TLV-parametrin pituuden (koko parametri otsikoineen).
static void endP(LB *L, int pos) { int len = L->n - pos; L->b[pos+2] = (unsigned char)(len >> 8); L->b[pos+3] = (unsigned char)len; }

static unsigned long g_msgid = 1;

// Lahettaa LLRP-sanoman (otsikko + hyotykuorma) yhtena lohkona.
static int sendLLRP(int cn, unsigned type, const unsigned char *payload, int plen)
{
	unsigned char msg[700];
	int nsent;
	unsigned ver_type = (1u << 10) | (type & 0x3FF);  // versio 1
	unsigned long total = 10 + plen;

	msg[0] = (unsigned char)(ver_type >> 8);
	msg[1] = (unsigned char)ver_type;
	msg[2] = (unsigned char)(total >> 24);
	msg[3] = (unsigned char)(total >> 16);
	msg[4] = (unsigned char)(total >> 8);
	msg[5] = (unsigned char)total;
	msg[6] = (unsigned char)(g_msgid >> 24);
	msg[7] = (unsigned char)(g_msgid >> 16);
	msg[8] = (unsigned char)(g_msgid >> 8);
	msg[9] = (unsigned char)g_msgid;
	g_msgid++;
	if (plen > 0)
		memcpy(msg + 10, payload, plen);
	return wrt_st_TCP(hComm[cn], (int)total, (char *)msg, &nsent);
}

// Rakentaa ADD_ROSPEC-hyotykuorman: jatkuva inventaario kaikilla antenneilla,
// raportoi jokaisen tagin EPC + antenni + ensimmaisen havainnon aikaleima.
static void buildAddRospec(LB *L)
{
	L->n = 0;
	int ro = beginP(L, 177);          // ROSpec
		p32(L, 1);                    // ROSpecID = 1
		p8(L, 0);                     // Priority
		p8(L, 0);                     // CurrentState = Disabled
		int bound = beginP(L, 178);   // ROBoundarySpec
			int st = beginP(L, 179);      // ROSpecStartTrigger
				p8(L, 3);                 // GPI: nouseva reuna (HIGH) kaynnistaa
				int sg = beginP(L, 181);  // GPITriggerValue
					p16(L, ZebraGpiPort);     // GPIPortNum
					p8(L, 0x80);          // GPIEvent = true (HIGH), bitti 7
					p32(L, 0);            // Timeout = 0 (ei aikakatkaisua)
				endP(L, sg);
			endP(L, st);
			int sp = beginP(L, 182);      // ROSpecStopTrigger
				p8(L, 2);                 // GPI_With_Timeout: LOW pysayttaa
				p32(L, 0);                // DurationTriggerValue = 0 (ei kestopysaytysta)
				int pg = beginP(L, 181);  // GPITriggerValue
					p16(L, ZebraGpiPort);     // GPIPortNum
					p8(L, 0x00);          // GPIEvent = false (LOW)
					p32(L, 0);            // Timeout = 0
				endP(L, pg);
			endP(L, sp);
		endP(L, bound);
		int ai = beginP(L, 183);      // AISpec
			p16(L, 1);                // AntennaCount = 1
			p16(L, 0);                // AntennaID = 0 -> kaikki antennit
			int aist = beginP(L, 184);// AISpecStopTrigger
				p8(L, 0);             // Null
				p32(L, 0);            // Duration
			endP(L, aist);
			int inv = beginP(L, 186); // InventoryParameterSpec
				p16(L, 1);            // InventoryParameterSpecID
				p8(L, 1);             // ProtocolID = EPCGlobalClass1Gen2
				// Gen2-inventoinnin hallinta: Session 0, jotta kentassa oleva tagi
				// luetaan jatkuvasti ja tiheasti (presence-luenta) eika se vaikene
				// Session 1 -lipun persistenssin (0.5-5 s) takia.
				// HUOM: FX9600 hylkaa eksplisiittisen single-target-asetuksen
				// (TagInventoryStateAware + SingulationAction -> A_Invalid), joten
				// kohteen (target A/B) hallinta jaa lukijalle. Session 0 yksin
				// korjaa patkittaisen luennan.
				int ac = beginP(L, 222);          // AntennaConfiguration
					p16(L, 0);                    // AntennaID = 0 -> kaikki antennit
					int ic = beginP(L, 330);      // C1G2InventoryCommand
						p8(L, 0x00);              // TagInventoryStateAware = 0 (lukija hallitsee kohteen)
						int sc = beginP(L, 336);  // C1G2SingulationControl
							p8(L, 0x00);          // Session = 0 (bitit 7-6)
							p16(L, 4);            // TagPopulation (arvio, vahan tageja)
							p32(L, 0);            // TagTransitTime = 0
						endP(L, sc);
					endP(L, ic);
				endP(L, ac);
			endP(L, inv);
		endP(L, ai);
		int rep = beginP(L, 237);     // ROReportSpec
			p8(L, 2);                 // ROReportTrigger = Upon_N_Tags_Or_End_Of_ROSpec
			p16(L, 1);                // N = 1 (raportoi joka tagi heti)
			int tcs = beginP(L, 238); // TagReportContentSelector
				// EnableAntennaID (bit12) | EnableFirstSeenTimestamp (bit9)
				// | EnableLastSeenTimestamp (bit8) -> tarvitaan takareunaan
				p16(L, 0x1300);
			endP(L, tcs);
		endP(L, rep);
	endP(L, ro);
}

// Rakentaa SET_READER_CONFIG-hyotykuorman, joka ottaa GPI-portin kayttoon, jotta
// ROSpecin GPI-start/stop-triggerit voivat laueta. Ilman tata GPI-portti on
// lukijassa oletuksena pois kaytosta eika trigger laukea.
static void buildEnableGpi(LB *L)
{
	L->n = 0;
	p8(L, 0x00);                  // ResetToFactoryDefault = false (bitti 7), johtava tavu
	int gp = beginP(L, 225);      // GPIPortCurrentState
		p16(L, ZebraGpiPort);         // GPIPortNum
		p8(L, 0x80);              // Config = enabled (bitti 7); reserved(7) = 0
		p8(L, 0);                 // State (GPIPortState; lukija ohittaa SET:ssa)
	endP(L, gp);
}

// Asettaa GPO-portin (summeri) tilan LLRP:lla: on=true -> GPO1 paalle, false -> pois.
// Lahettaa SET_READER_CONFIG:n (tyyppi 3), jonka hyotykuormassa on VAIN GPOWriteData
// (tyyppi 219). KRIITTISTA: ei sisalla GPIPortCurrentState:a, jottei buildEnableGpi:n
// tekema GPI-portin kayttoonotto nollaudu (muuten kytkin lakkaisi laukaisemasta luentaa).
static void setGpo(int cn, bool on)
{
	LB L;
	L.n = 0;
	p8(&L, 0x00);                       // ResetToFactoryDefault = false (johtava tavu)
	int gp = beginP(&L, LLRP_GPO_WRITE_DATA);   // GPOWriteData (219)
		p16(&L, GPO_PORT);              // GPOPortNumber (u16)
		p8(&L, on ? 0x80 : 0x00);      // GPOData = bitti 7 (0x80=paalla); 7 bittia reserved=0
	endP(&L, gp);
	sendLLRP(cn, LLRP_SET_READER_CONFIG, L.b, L.n);
}

// Etsii TLV-parametrin (maaritelty alempana); kaytetaan GPI-tilan jasennykseen.
static const unsigned char *findTLV(const unsigned char *buf, int len,
	unsigned want, int *vlen);

// === FX9600-DIAGNOSTIIKKALOKITUS (kytkettävissä loki-lipulla) ===
// Paljasti IoT Connector -vian kevät 2026, jätetty paikalleen pysyväksi työkaluksi.
// Nama funktiot vain HAVANNOIVAT (lukevat ja kirjaavat lokiin), eivat muuta
// luennan toimintaa. Vaativat etta loki on paalla (loki != 0). Eivat kosketa
// SIRIT-polkua. Tarkoitus: paljastaa onnistuuko vanhan ROSpecin poisto ja onko
// lukijaan jaanyt ylimaaraisia (esim. periodisia) ROSpeceja.

// Lukee LLRP-vastauksia ~600 ms ja kirjaa annetun vastaustyypin LLRPStatus-
// koodin lokiin (0 = OK). Vastaa KEEPALIVE:en. Diagnostiikka kohtaan (1).
static void zdiagWaitStatus(int cn, unsigned wantType, const wchar_t *label)
{
	if (!loki) return;
	unsigned char rb[4096];
	int rblen = 0, got = 0;
	for (int s = 0; s < 12 && !got; s++) {       // ~12 x 50 ms = 600 ms
		int nr = 0;
		if (rblen < (int)sizeof(rb) - 512) {
			read_TCP(hComm[cn], (int)sizeof(rb) - rblen, (char *)(rb + rblen), &nr);
			if (nr > 0) rblen += nr;
			}
		while (rblen >= 10) {
			unsigned t = ((rb[0] & 0x03) << 8) | rb[1];
			unsigned long ml = ((unsigned long)rb[2] << 24) | ((unsigned long)rb[3] << 16) |
				((unsigned long)rb[4] << 8) | rb[5];
			if (ml < 10 || ml > (unsigned long)sizeof(rb)) { rblen = 0; break; }
			if ((unsigned long)rblen < ml) break;
			if (t == LLRP_KEEPALIVE)
				sendLLRP(cn, LLRP_KEEPALIVE_ACK, 0, 0);
			else if (t == wantType) {
				int vl = 0;
				// LLRPStatus on TLV-parametri 287; arvo-osa: StatusCode(u16)+ByteCount(u16)+teksti
				const unsigned char *st = findTLV(rb + 10, (int)(ml - 10), 287, &vl);
				wchar_t lg[160];
				if (st && vl >= 2) {
					unsigned code = (st[0] << 8) | st[1];
					swprintf(lg, L"%d: FX9600 %ls -> LLRPStatus=%u (%ls)",
						mstimer(), label, code, code == 0 ? L"OK" : L"VIRHE");
					}
				else
					swprintf(lg, L"%d: FX9600 %ls -> ei LLRPStatusia vastauksessa",
						mstimer(), label);
				wkirjloki(lg);
				got = 1;
				}
			memmove(rb, rb + ml, rblen - (int)ml);
			rblen -= (int)ml;
			}
		if (!got) Sleep(50);
		}
	if (!got) {
		wchar_t lg[120];
		swprintf(lg, L"%d: FX9600 %ls -> ei vastausta 600 ms:ssa", mstimer(), label);
		wkirjloki(lg);
		}
}

// Kysyy GET_ROSPECS ja kirjaa kuinka monta ROSpecia lukijassa on seka kunkin
// ROSpecID:n ja CurrentState:n (0=Disabled, 1=Inactive, 2=Active). Diagnostiikka
// kohtaan (2): paljastaa lukijaan jaaneet/ylimaaraiset ROSpecit (esim. periodinen
// vanha spec ohjelman GPI-ROSpecin rinnalla).
static void zdiagGetRospecs(int cn)
{
	if (!loki) return;
	sendLLRP(cn, LLRP_GET_ROSPECS, 0, 0);
	unsigned char rb[8192];
	int rblen = 0, got = 0;
	for (int s = 0; s < 12 && !got; s++) {       // ~12 x 50 ms = 600 ms
		int nr = 0;
		if (rblen < (int)sizeof(rb) - 1024) {
			read_TCP(hComm[cn], (int)sizeof(rb) - rblen, (char *)(rb + rblen), &nr);
			if (nr > 0) rblen += nr;
			}
		while (rblen >= 10) {
			unsigned t = ((rb[0] & 0x03) << 8) | rb[1];
			unsigned long ml = ((unsigned long)rb[2] << 24) | ((unsigned long)rb[3] << 16) |
				((unsigned long)rb[4] << 8) | rb[5];
			if (ml < 10 || ml > (unsigned long)sizeof(rb)) { rblen = 0; break; }
			if ((unsigned long)rblen < ml) break;
			if (t == LLRP_KEEPALIVE)
				sendLLRP(cn, LLRP_KEEPALIVE_ACK, 0, 0);
			else if (t == LLRP_GET_ROSPECS_RESPONSE) {
				// Top-tason parametrit: LLRPStatus (287) + 0..n ROSpec (177).
				// ROSpec arvo-osa alkaa: ROSpecID(u32) + Priority(u8) + CurrentState(u8) + ...
				const unsigned char *p = rb + 10;
				int plen = (int)(ml - 10), i = 0, nspec = 0;
				while (i + 4 <= plen) {
					if (p[i] & 0x80) break;       // TV-parametri ei kuulu tanne
					unsigned pt = ((p[i] & 0x03) << 8) | p[i+1];
					int pl = (p[i+2] << 8) | p[i+3];
					if (pl < 4 || i + pl > plen) break;
					if (pt == 177 && pl >= 10) {
						unsigned long rid = ((unsigned long)p[i+4] << 24) | ((unsigned long)p[i+5] << 16) |
							((unsigned long)p[i+6] << 8) | p[i+7];
						unsigned state = p[i+9];
						wchar_t lg[150];
						swprintf(lg, L"%d: FX9600 GET_ROSPECS: ROSpecID=%lu CurrentState=%u",
							mstimer(), rid, state);
						wkirjloki(lg);
						nspec++;
						}
					i += pl;
					}
				wchar_t lg[120];
				swprintf(lg, L"%d: FX9600 GET_ROSPECS: lukijassa %d ROSpec(ia)",
					mstimer(), nspec);
				wkirjloki(lg);
				got = 1;
				}
			memmove(rb, rb + ml, rblen - (int)ml);
			rblen -= (int)ml;
			}
		if (!got) Sleep(50);
		}
	if (!got)
		wkirjloki(L"FX9600 GET_ROSPECS: ei vastausta 600 ms:ssa");
}
// === FX9600-DIAGNOSTIIKKALOKITUKSEN LOPPU ===

// Asettaa GPI-triggeroidyn inventaarion lukijaan: GPI HIGH (nouseva reuna)
// kaynnistaa, LOW pysayttaa. LISAKSI tarkistetaan GPI:n nykytila ROSpecin
// asetuksen jalkeen: jos kytkin on JO HIGH (ei nousevaa reunaa tulossa),
// luenta kaynnistetaan kerran START_ROSPECilla. Jos LOW, jaadaan odottamaan reunaa.
static void startInventory(int cn)
{
	unsigned char id[4];
	LB L;

	id[0] = 0; id[1] = 0; id[2] = 0; id[3] = 0;
	sendLLRP(cn, LLRP_DELETE_ROSPEC, id, 4);   // poista kaikki ROSpecit
	Sleep(150);
	zdiagWaitStatus(cn, LLRP_DELETE_ROSPEC_RESPONSE, L"DELETE_ROSPEC");  // DIAGNOSTIIKKALOKITUS
	buildEnableGpi(&L);                        // ota GPI-portti kayttoon ennen ROSpecia
	sendLLRP(cn, LLRP_SET_READER_CONFIG, L.b, L.n);
	Sleep(150);
	buildAddRospec(&L);
	sendLLRP(cn, LLRP_ADD_ROSPEC, L.b, L.n);
	Sleep(200);
	zdiagWaitStatus(cn, LLRP_ADD_ROSPEC_RESPONSE, L"ADD_ROSPEC");        // DIAGNOSTIIKKALOKITUS
	id[3] = 1;
	sendLLRP(cn, LLRP_ENABLE_ROSPEC, id, 4);   // ROSpecID = 1: aseteltu, odottaa GPI HIGH
	Sleep(150);
	zdiagWaitStatus(cn, LLRP_ENABLE_ROSPEC_RESPONSE, L"ENABLE_ROSPEC");  // DIAGNOSTIIKKALOKITUS

	// --- GPI-tilan tarkistus + ehdollinen START_ROSPEC ---
	// Kysytaan GPI-portin nykytila (GET_READER_CONFIG, RequestedData=9). Jos kytkin
	// on JO HIGH ROSpecia asetettaessa, nousevaa reunaa ei tule eika luenta kaynnisty;
	// silloin kaynnistetaan luenta kerran START_ROSPECilla. Jos LOW, jaadaan
	// odottamaan nousevaa reunaa (kytkin pois = RF pois). GPI-stop-trigger jaa voimaan.
	{
		unsigned char q[7];
		q[0] = 0; q[1] = 0;        // AntennaID = 0
		q[2] = 9;                  // RequestedData = GPIPortCurrentState
		q[3] = 0; q[4] = ZebraGpiPort; // GPIPortNum = 1
		q[5] = 0; q[6] = 0;        // GPOPortNum = 0
		sendLLRP(cn, LLRP_GET_READER_CONFIG, q, 7);

		unsigned char rb[4096];
		int rblen = 0, state = -1, gotResp = 0;
		for (int s = 0; s < 10 && !gotResp; s++) {   // ~10 x 50 ms = 500 ms
			int nr = 0;
			if (rblen < (int)sizeof(rb) - 512) {
				read_TCP(hComm[cn], (int)sizeof(rb) - rblen, (char *)(rb + rblen), &nr);
				if (nr > 0) rblen += nr;
				}
			while (rblen >= 10) {
				unsigned t = ((rb[0] & 0x03) << 8) | rb[1];
				unsigned long ml = ((unsigned long)rb[2] << 24) | ((unsigned long)rb[3] << 16) |
					((unsigned long)rb[4] << 8) | rb[5];
				if (ml < 10 || ml > (unsigned long)sizeof(rb)) { rblen = 0; break; }
				if ((unsigned long)rblen < ml) break;
				if (t == LLRP_KEEPALIVE)
					sendLLRP(cn, LLRP_KEEPALIVE_ACK, 0, 0);
				else if (t == LLRP_GET_READER_CONFIG_RESPONSE) {
					int vl = 0;
					const unsigned char *gp = findTLV(rb + 10, (int)(ml - 10),
						P_GPI_PORT_CURRENT_STATE, &vl);
					if (gp && vl >= 4) state = gp[3];   // GPIPortNum(2)+Config(1)+State(1)
					gotResp = 1;
					}
				memmove(rb, rb + ml, rblen - (int)ml);
				rblen -= (int)ml;
				}
			if (!gotResp) Sleep(50);
			}

		if (loki) {                        // DIAGNOSTIIKKALOKITUS: miksi START_ROSPEC jai lahettamatta
			wchar_t lg[140];
			if (!gotResp)
				swprintf(lg, L"%d: FX9600 GPI tilakysely -> ei vastausta 500 ms:ssa", mstimer());
			else if (state < 0)
				swprintf(lg, L"%d: FX9600 GPI tilakysely -> vastaus ilman GPIPortCurrentState-parametria", mstimer());
			else
				swprintf(lg, L"%d: FX9600 GPI%d tila=%ls -> START_ROSPEC %ls",
					mstimer(), ZebraGpiPort, state == 1 ? L"HIGH" : L"LOW", state == 1 ? L"lahetetty" : L"ei lahetetty");
			wkirjloki(lg);
			}

		if (state == 1) {                  // GPI High -> kaynnista luenta kerran
			id[3] = 1;
			sendLLRP(cn, LLRP_START_ROSPEC, id, 4);
			}
	}

	// DIAGNOSTIIKKALOKITUS: kysy paljonko ROSpeceja lukijassa on lopullisen
	// asetuksen jalkeen (pitaisi olla tasan 1 kpl, ROSpecID=1). >1 -> jaanyt vanha.
	zdiagGetRospecs(cn);

	sendLLRP(cn, LLRP_ENABLE_EVENTS_AND_REPORTS, 0, 0);
	Sleep(50);
}

// --- Tagiraporttien jasennys ---

// Muotoilee tavut heksamerkkijonoksi (isot kirjaimet).
static void tohex(const unsigned char *src, int n, char *dst, int dstsz)
{
	int i;
	if (n * 2 >= dstsz)
		n = (dstsz - 1) / 2;
	for (i = 0; i < n; i++)
		sprintf(dst + 2*i, "%02X", src[i]);
	dst[2*n] = 0;
}

// ZEBRADEPART: aika (ms), jonka tagi saa olla nakymatta ennen kuin se tulkitaan
// poistuneeksi kentasta (takareuna). Luetaan HkInit.cpp:ssa; oletus 700 ms.
int ZebraDepart = 700;

// ZEBRADEPARTCLEANUP: aika (ms), jonka jalkeen departed-slotti vapautetaan uudelleen-
// kayttolle. Arvo 0 = automaatti (10 x ZebraDepart). Luetaan HkInit.cpp:ssa.
// Saato: lyhyt arvo sallii saman tagin uuden ohituksen nopeammin (riski: viela
// kentassa olevan tagin slotti vapautuu liian pian -> toinen depart). Pitka arvo
// estaa varmemmin toistuvan depart-syklin mutta blokkaa saman tagin uuden ohituksen.
// HUOM: sama slotin elinkaari (ZebraDepart + tama cleanup) saataa MOLEMPIA reunoja.
// Etureunalla (E) se maaraa, kuinka pian sama tagi voi tuottaa uuden arriven
// poistuttuaan kentasta; takareunalla (T) saman tagin uuden departin.
int ZebraDepartCleanup = 0;

// Muotoilee tagitapahtuman SIRIT-tekstimuotoon ja syottaa tall_regnly:lle.
// depart=false -> "arrive ... first=" (etureuna), depart=true -> "depart ... last=" (takareuna).
static void emitTag(int r_no, const char *epchex, unsigned antenna, int haveAnt,
	unsigned long long utc, int haveUtc, bool depart)
{
	int yyyy, mon, dd, hh, mm, ss, ms;
	if (haveUtc) {
		time_t secs = (time_t)(utc / 1000000ULL);
		struct tm lt;
		ms = (int)((utc / 1000ULL) % 1000ULL);
		localtime_s(&lt, &secs);
		yyyy = lt.tm_year + 1900; mon = lt.tm_mon + 1; dd = lt.tm_mday;
		hh = lt.tm_hour; mm = lt.tm_min; ss = lt.tm_sec;
		}
	else {
		SYSTEMTIME stm;
		GetLocalTime(&stm);
		yyyy = stm.wYear; mon = stm.wMonth; dd = stm.wDay;
		hh = stm.wHour; mm = stm.wMinute; ss = stm.wSecond; ms = stm.wMilliseconds;
		}
	san_type vast;
	// Sama tekstimuoto kuin FX9500/SIRIT, jotta siritaika jasentaa numeron ja ajan.
	sprintf(vast.bytes,
		"event.tag.%s tag_id=0x%s %s=%04d-%02d-%02dT%02d:%02d:%02d.%03d antenna=%u\r",
		depart ? "depart" : "arrive", epchex, depart ? "last" : "first",
		yyyy, mon, dd, hh, mm, ss, ms, haveAnt ? antenna : 1);
	tall_regnly(&vast, r_no);
}

// --- Aktiiviset tagit etureunan (arrive) ja takareunan (poistuman) tunnistusta varten ---
#define ZEBRA_MAXACTIVE 256
typedef struct {
	int used;
	int arrived;      // 1 kun arrive on jo emittoitu talle lahsaolojaksolle (etureuna)
	int departed;     // 1 kun depart on jo emittoitu/lahsaolo paattynyt talle jaksolle
	int departedMs;   // mstimer() depart-emittoinnin / lahsaolon paattymisen hetkella
	char epc[80];
	unsigned antenna;
	int haveAnt;
	unsigned long long ts;   // viimeisin (suurin) LastSeen-aika
	int haveTs;
	int lastMs;              // mstimer() viimeisimmasta havainnosta
} ActiveTag;
static ActiveTag g_active[NREGNLY][ZEBRA_MAXACTIVE];

// Tyhjentaa aktiivitagi-taulukon (kutsutaan yhteytta avattaessa, myos reconnect).
static void clearActive(int r_no)
{
	int i;
	for (i = 0; i < ZEBRA_MAXACTIVE; i++)
		g_active[r_no][i].used = 0;
}

// Kirjaa havainnon yhteiseen g_active-taulukkoon ja emittoi etureunan arriven
// VAIN kerran per lahsaolojakso (slotin luontihetkella, FirstSeen-ajalla). Sama
// taulukko ja sama departed/cleanup-kuvio hoitaa seka etureunan (arrived) etta
// takareunan (departed) "emittoi kerran per lahsaolo" -vaimennuksen.
//  - wantFront != 0 (E/M): emittoi arrive uudelle slotille ja merkitsee arrived.
//  - ts paivitetaan suurimmaksi (uusin havainto) takareunan depart-aikaa varten.
//  - jos tagi on jo departed-tilassa, paivitys ohitetaan, jottei sama
//    lahsaolojakso kaynnistaisi uutta sykli a (slotti vapautuu cleanupissa).
// Takareunan depart-emissio tehdaan checkDeparturessa (portitettu siritreuna & 1).
static void activeSee(int r_no, const char *epc, unsigned antenna, int haveAnt,
	unsigned long long firstUTC, int haveFirst,
	unsigned long long lastUTC, int haveLast, int wantFront)
{
	int i, vapaa = -1;
	unsigned long long ts = haveLast ? lastUTC : firstUTC;
	int haveTs = haveLast || haveFirst;
	for (i = 0; i < ZEBRA_MAXACTIVE; i++) {
		if (g_active[r_no][i].used) {
			if (strcmp(g_active[r_no][i].epc, epc) == 0) {
				if (g_active[r_no][i].departed) {
					// Tagi luetaan depart-emittoinnin/lahsaolon paattymisen jalkeen:
					// ei paiviteta lastMs:aa, jottei checkDepartures kaynnistaisi
					// uutta syklia -> slotti vapautuu cleanupissa.
					return;
					}
				// Tagi jo aktiivinen: arrive on jo emittoitu (vaimennus) -> ei uutta
				// emittia. Paivitetaan vain havaintotiedot takareunaa varten.
				if (haveTs && (!g_active[r_no][i].haveTs || ts > g_active[r_no][i].ts)) {
					g_active[r_no][i].ts = ts;
					g_active[r_no][i].haveTs = 1;
					}
				g_active[r_no][i].antenna = antenna;
				g_active[r_no][i].haveAnt = haveAnt;
				g_active[r_no][i].lastMs = mstimer();
				return;
				}
			}
		else if (vapaa < 0)
			vapaa = i;
		}
	if (vapaa >= 0) {
		g_active[r_no][vapaa].used = 1;
		g_active[r_no][vapaa].arrived = 0;
		g_active[r_no][vapaa].departed = 0;
		g_active[r_no][vapaa].departedMs = 0;
		strncpy(g_active[r_no][vapaa].epc, epc, sizeof(g_active[r_no][vapaa].epc) - 1);
		g_active[r_no][vapaa].epc[sizeof(g_active[r_no][vapaa].epc) - 1] = 0;
		g_active[r_no][vapaa].antenna = antenna;
		g_active[r_no][vapaa].haveAnt = haveAnt;
		g_active[r_no][vapaa].ts = ts;
		g_active[r_no][vapaa].haveTs = haveTs;
		g_active[r_no][vapaa].lastMs = mstimer();
		// GPO1-summeripulssi: uusi tagi havaittu kenttaan (uusi slotti varattu) ->
		// summeri paalle 1 s. Soi AINA uudesta lahsaolosta riippumatta SIRITREUNA-
		// asetuksesta (myos pelkka T-tila). Tapahtuu vain KERRAN per lahsaolojakso,
		// koska slotin varaus tapahtuu kerran (ei joka luennassa). Maski (maskOhita)
		// on jo suodattanut vieraan tagin ennen activeSee:ta. Jos pulssi on jo paalla
		// (toinen tagi alle 1 s sisalla), vain nollataan ajastin (yksi yhteinen summeri).
		int cn = cn_regnly[r_no];
		if (gpoActive[r_no])
			gpoOnMs[r_no] = mstimer();
		else {
			setGpo(cn, true);
			gpoActive[r_no] = 1;
			gpoOnMs[r_no] = mstimer();
			}
		// Etureuna: emittoi arrive VAIN nyt (uusi lahsaolojakso, ensilukema).
		if (wantFront && !g_active[r_no][vapaa].arrived) {
			emitTag(r_no, epc, antenna, haveAnt, firstUTC, haveFirst, false);
			g_active[r_no][vapaa].arrived = 1;
			}
		}
}

// Palauttaa departed-slotille kaytetyn siivousajan ms:ina.
// ZebraDepartCleanup == 0: kayteta automaattisesti 10 x ZebraDepart.
static int departCleanupMs(void)
{
	return ZebraDepartCleanup > 0 ? ZebraDepartCleanup : 10 * ZebraDepart;
}

// Tarkistaa lahsaolon paattymisen: jos tagia ei ole nahty ZebraDepart ms:iin,
// takareunalla (T/M) emittoidaan poistuma (last=) viimeisella havaintoajalla.
// Etureunalla (pelkka E) ei emittoida departtia, mutta lahsaolo merkitaan silti
// paattyneeksi (departed), jotta slotti vanhenee ja sama tagi voi myohemmin
// tuottaa uuden arriven. Departed-slotti pysyy varattuna departCleanupMs():n
// ajan, jottei sama lahsaolojakso tuota useita tapahtumia (ks. activeSee).
static void checkDepartures(int r_no)
{
	int i, now = mstimer();
	for (i = 0; i < ZEBRA_MAXACTIVE; i++) {
		if (!g_active[r_no][i].used) continue;
		if (g_active[r_no][i].departed) {
			if (now - g_active[r_no][i].departedMs > departCleanupMs())
				g_active[r_no][i].used = 0;
			continue;
			}
		if (now - g_active[r_no][i].lastMs > ZebraDepart) {
			// Vain takareuna (T/M) emittoi poistuman; E-tilassa pelkka merkinta.
			if (siritreuna & 1)
				emitTag(r_no, g_active[r_no][i].epc, g_active[r_no][i].antenna,
					g_active[r_no][i].haveAnt, g_active[r_no][i].ts,
					g_active[r_no][i].haveTs, true);
			g_active[r_no][i].departed = 1;
			g_active[r_no][i].departedMs = now;
			// EI: used = 0 tassa â€” se aiheutti uudelleen-lisays+depart-syklin
			}
		}
}

// SiritMask: maskin konfiguraatio (X/Z/V/P + hex). Maaritelma TpLaitteet.cpp:ssa;
// SIRIT-polkua ei muuteta, tassa kaytetaan vain samaa konfiguraatioarvoa, jotta
// ZEBRA ja SIRIT suodattavat yhdenmukaisesti.
extern char SiritMask[31];

// Maskisuodatus: palauttaa true jos EPC on suodatettava (pudotettava) pois.
// Toisinto TpLaitteet.cpp:n ohitaSirit-funktiosta (joka on static eika siten
// nakyvissa tanne). Logiikan ON oltava identtinen ohitaSirit:n kanssa (samat
// lajit X/Z/V/P), jotta ZEBRA ja SIRIT pudottavat samat tagit. Avulla vieras
// tagi karsiutuu jo tassa, ENNEN emitTag/tall_regnly-kutsua, eika kuormita
// putkea joka luennalla. Syote on heksamuotoinen EPC (kuten ohitaSirit:lle p,
// joka osoittaa "tag_id=0x":n jalkeisiin heksamerkkeihin).
static bool maskOhita(const char *epchex)
{
	int len, laji;
	UINT32 maskval, stval;
	char buf[32] = "0x";

	if ((len = (int)strlen(SiritMask) - 1) < 1 || (laji = stschind(SiritMask[0], "XZVP")) < 0)
		return(false);
	memcpy(buf, epchex, len);
	buf[len] = 0;
	stval = strtoul(buf, 0, 16);
	maskval = strtoul(SiritMask + 1, 0, 16);
	switch (laji) {
		case 0:
			return(maskval == stval);
		case 1:
			return((maskval & stval) != 0);
		case 2:
			return(maskval != stval);
		case 3:
			return((maskval & stval) == 0);
		}
	return(false);
}

static unsigned long long pcUtcMicros(void);   // maaritelty alempana

// Kasittelee yhden TagReportData-parametrin: poimii EPC:n, antennin ja
// aikaleiman, muotoilee SIRIT-tyylisen sanoman ja syottaa tall_regnly:lle.
static void handleTagReport(int r_no, const unsigned char *d, int len)
{
	char epchex[80] = "";
	int haveEpc = 0;
	unsigned antenna = 0;
	int haveAnt = 0;
	unsigned long long firstUTC = 0;
	int haveFirst = 0;
	unsigned long long lastUTC = 0;
	int haveLast = 0;
	int i = 0;

	while (i < len) {
		if (d[i] & 0x80) {            // TV-parametri
			unsigned t = d[i] & 0x7F;
			int sz;
			i++;
			// Ensin paatellaan parametrin koko lukematta viela sen sisaltoa,
			// jotta rajatarkistus voidaan tehda ennen d[i+1]/d[i+k]-lukuja.
			switch (t) {
			case P_ANTENNA_ID:        sz = 2; break;
			case P_PEAK_RSSI:         sz = 1; break;
			case P_CHANNEL_INDEX:     sz = 2; break;
			case P_ROSPEC_ID:         sz = 4; break;
			case P_INV_PARAM_SPEC_ID: sz = 2; break;
			case P_EPC_96:            sz = 12; break;
			case P_SPEC_INDEX:        sz = 2; break;
			case P_ACCESS_SPEC_ID:    sz = 4; break;
			case P_FIRST_SEEN_UTC:    sz = 8; break;  // tyyppi 2: TV, 8 tavua, FirstSeen mikrosekuntia UTC
			case P_LAST_SEEN_UTC:     sz = 8; break;  // tyyppi 4: TV, 8 tavua, LastSeen mikrosekuntia UTC
			case 3:                   // FirstSeenTimestampUptime (8 tavua, ei seinakelloaika)
			case 5:                   // LastSeenTimestampUptime (8 tavua)
				sz = 8;
				break;
			default:
				// Tuntematon TV: kokoa ei voi paatella -> lopetetaan parametrien luku,
				// mutta jo luettu EPC kasitellaan silti (ei pudoteta tagia).
				i = len; sz = 0;
				break;
			}
			if (i + sz > len)
				return;
			switch (t) {
			case P_ANTENNA_ID:
				antenna = (d[i] << 8) | d[i+1]; haveAnt = 1; break;
			case P_EPC_96:
				tohex(d + i, 12, epchex, sizeof(epchex)); haveEpc = 1; break;
			case P_FIRST_SEEN_UTC: {
				int k; firstUTC = 0;
				for (k = 0; k < 8; k++)
					firstUTC = (firstUTC << 8) | d[i+k];
				haveFirst = 1;
				break;
				}
			case P_LAST_SEEN_UTC: {
				int k; lastUTC = 0;
				for (k = 0; k < 8; k++)
					lastUTC = (lastUTC << 8) | d[i+k];
				haveLast = 1;
				break;
				}
			default:
				break;
			}
			i += sz;
			}
		else {                        // TLV-parametri
			if (i + 4 > len)
				return;
			unsigned t = ((d[i] & 0x03) << 8) | d[i+1];
			unsigned l = (d[i+2] << 8) | d[i+3];
			if (l < 4 || i + (int)l > len)
				return;
			if (t == P_EPC_DATA) {
				// EPCData = 4 tavun TLV-otsake + 2 tavun EPCLengthBits + EPC-tavut.
				// Alle 6 tavun mittainen parametri ei voi sisaltaa bittipituutta,
				// joten se ohitetaan (ei pudoteta tagia: mahd. EPC on jo luettu).
				if (l >= 6) {
					unsigned bits = (d[i+4] << 8) | d[i+5];
					int nb = (bits + 7) / 8;
					// Laitteen ilmoittama bittipituus voi olla suurempi kuin
					// parametriin oikeasti mahtuu -> rajataan parametrin sisaltoon.
					if (nb > (int)l - 6)
						nb = (int)l - 6;
					tohex(d + i + 6, nb, epchex, sizeof(epchex));
					haveEpc = 1;
					}
				}
			// Aikaleimat (tyypit 2-5) ovat TV-koodattuja -> kasitellaan TV-haarassa.
			i += (int)l;
			}
		}

	if (!haveEpc)
		return;

	// Paivita lukijan kellopoikkeama (max-suodatus): suurin offset = pienin
	// verkkoviive = tarkin arvio. Aikaleima tulee jo raportissa, ei eri kyselya.
	if (haveFirst) {
		int cand = (int)(((long long)firstUTC - (long long)pcUtcMicros()) / 100000LL);
		if (zebraOffsetState[r_no] != 1 || cand > zebraOffsetDs[r_no]) {
			zebraOffsetDs[r_no] = cand;
			zebraOffsetState[r_no] = 1;
			}
		}

	// Maski: pudota vieras tagi jo tassa, ennen mitaan emittia tai slotin
	// varausta, jottei se kuormita tall_regnly-putkea joka luennalla.
	if (maskOhita(epchex))
		return;

	int wantFront = (siritreuna & 2);   // E tai M

	// Yksi yhteinen kasittely: kirjaa lahsaolo g_active-taulukkoon ja emittoi
	// etureunan arrive VAIN kerran per lahsaolo (activeSee:n luontihaara).
	// Takareunan depart emittoidaan checkDeparturessa kun tagia ei nahda
	// ZebraDepart ms:iin. activeSee kutsutaan AINA (myos pelkka E-tila), jotta
	// etureunan arrived-vaimennus toimii; M-tilassa molemmat reunat toimivat.
	activeSee(r_no, epchex, antenna, haveAnt, firstUTC, haveFirst,
		lastUTC, haveLast, wantFront);
}

// Kasittelee RO_ACCESS_REPORT-sanoman: kay lapi TagReportData-parametrit.
static void handleAccessReport(int r_no, const unsigned char *d, int len)
{
	int i = 0;
	while (i + 4 <= len) {
		if (d[i] & 0x80)              // TV ei kuulu raportin ylatasolle
			break;
		unsigned ptype = ((d[i] & 0x03) << 8) | d[i+1];
		unsigned plen = (d[i+2] << 8) | d[i+3];
		if (plen < 4 || i + (int)plen > len)
			break;
		if (ptype == P_TAG_REPORT_DATA)
			handleTagReport(r_no, d + i + 4, (int)plen - 4);
		i += (int)plen;
		}
}

// --- IRfidReader-rajapinta ---

// Akkumulointipuskuri r_no-kohtaisesti (LLRP-sanomat voivat tulla osina).
static unsigned char g_rx[NREGNLY][RXSZ];
static int g_rxlen[NREGNLY];

// Etsii TLV-parametrilistasta (buf, len) ensimmaisen tyyppia 'want' olevan
// parametrin; palauttaa osoittimen sen ARVO-osaan (otsikon jalkeen) ja *vlen
// sen pituuden, tai 0 jos ei loydy. Kasittelee vain TLV-parametrit (>=128).
static const unsigned char *findTLV(const unsigned char *buf, int len,
	unsigned want, int *vlen)
{
	int i = 0;
	while (i + 4 <= len) {
		if (buf[i] & 0x80)        // TV-parametri: ei pituuskenttaa -> ei osata ohittaa
			break;
		unsigned t = ((buf[i] & 0x03) << 8) | buf[i+1];
		int l = (buf[i+2] << 8) | buf[i+3];
		if (l < 4 || i + l > len)
			break;
		if (t == want) {
			*vlen = l - 4;
			return buf + i + 4;
			}
		i += l;
		}
	return 0;
}

// Jasentaa READER_EVENT_NOTIFICATIONin hyotykuorman (payload, plen): etsii
// ReaderEventNotificationData (246) ja sen sisalta Timestampin -> UTCTimestamp
// (128) tai Uptime (129). Palauttaa 1 jos aikaleima loytyi; *isUptime kertoo
// kumpaa tyyppia se oli, *micros sisaltaa u64-mikrosekunnit.
static int parseReaderEventUTC(const unsigned char *payload, int plen,
	unsigned long long *micros, int *isUptime)
{
	int dlen = 0;
	const unsigned char *d = findTLV(payload, plen, P_READER_EVENT_NOTIF_DATA, &dlen);
	if (!d)
		return 0;
	int tlen = 0;
	*isUptime = 0;
	const unsigned char *ts = findTLV(d, dlen, P_UTC_TIMESTAMP, &tlen);
	if (!ts) {
		ts = findTLV(d, dlen, P_UPTIME, &tlen);
		*isUptime = 1;
		}
	if (!ts || tlen < 8)
		return 0;
	unsigned long long v = 0;
	for (int k = 0; k < 8; k++)
		v = (v << 8) | ts[k];
	*micros = v;
	return 1;
}

// PC:n nykyaika mikrosekunteina UTC-epookista (1970-01-01).
static unsigned long long pcUtcMicros(void)
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);   // 100 ns -tikkia vuodesta 1601 UTC
	unsigned long long t = ((unsigned long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	return t / 10ULL - 11644473600000000ULL;   // -> us, epookki 1970
}

// Laskee lukijan ja PC:n kellon poikkeaman ja tallentaa sen globaaliin
// (zebraOffsetDs = poikkeama 1/10 s, zebraOffsetState = mittauksen tila), jonka
// lukumaarat() nayttaa LUKIJA/KELLO-laatikossa ZEBRA-rivilla juoksevana kellona.
// EI kirjoita ruudulle (alalaita); kirjaa poikkeaman lokiin (jalkikateisdiagnostiikka).
// Kutsutaan yhteytta avattaessa -> asettaa baselinen (reconnect nollaa ja mittaa uudelleen).
static void measureClockOffset(int r_no, int haveUTC, int haveUptime,
	unsigned long long readerUTC)
{
	if (haveUTC) {
		long long delta = (long long)readerUTC - (long long)pcUtcMicros();   // us, +/-
		zebraOffsetDs[r_no] = (int)(delta / 100000LL);    // -> 1/10 s, etumerkki sailyy
		zebraOffsetState[r_no] = 1;
		if (loki) {
			wchar_t lg[100];
			long long ad = delta < 0 ? -delta : delta;
			long long tenths = (ad + 50000LL) / 100000LL; // pyoristus 0.1 s
			swprintf(lg, L"%d: FX9600 kello %ls%lld.%lld s",
				mstimer(), delta < 0 ? L"-" : L"+", tenths / 10, tenths % 10);
			wkirjloki(lg);
			}
		}
	else if (haveUptime) {
		zebraOffsetState[r_no] = 2;
		if (loki)
			wkirjloki(L"FX9600: aikaleima oli Uptime (ei UTC-kelloa)");
		}
	else {
		zebraOffsetState[r_no] = 0;
		if (loki)
			wkirjloki(L"FX9600: avausilmoituksesta ei saatu aikaleimaa");
		}
}

int ZebraReader::openConnection(int r_no, bool reconnect)
{
	int cn = cn_regnly[r_no];
	wchar_t wline[200];

	comtype[cn] = comtpTCP;
	ipparam[MAX_LAHPORTTI + r_no].noreconnect = 1;
	if (ipparam[MAX_LAHPORTTI + r_no].destport == 0)
		ipparam[MAX_LAHPORTTI + r_no].destport = LLRP_PORT;

	if (reconnect)
		closeportTCP(&hComm[cn]);

	if (openlukijaTCP(cn))
		return 1;

	for (int i = 0; i < 5; i++) {
		if (TCPyht_on(hComm[cn]))
			break;
		Sleep(500);
		}
	if (!TCPyht_on(hComm[cn]))
		return 1;

	comopen[cn] = 1;
	aikaTCPstatus = r_no + 1;
	paivita_aikanaytto();
	g_rxlen[r_no] = 0;
	clearActive(r_no);   // tyhjenna aktiivitagit (myos reconnect) -> ei haamupoistumia

	// Lue yhteydenavauksen READER_EVENT_NOTIFICATION ja jasenna lukijan kello,
	// jotta poikkeama PC:hen voidaan nayttaa heti (ennen ensimmaista tagia).
	// Tehdaan ENNEN startInventoryta: ROSpecia ei viela ole, joten puskurissa on
	// vain avausilmoitus (ei tagiraportteja eika komentovastauksia). Sailyttaa
	// ~300 ms asettumisajan. Muut sanomat jaavat puskuriin readTagsille.
	unsigned long long readerUTC = 0;
	int gotTs = 0, isUpt = 0;
	for (int s = 0; s < 6 && !gotTs; s++) {     // ~6 x 50 ms = 300 ms
		int nread = 0;
		if (g_rxlen[r_no] < RXSZ - 2048) {
			read_TCP(hComm[cn], RXSZ - g_rxlen[r_no],
				(char *)(g_rx[r_no] + g_rxlen[r_no]), &nread);
			if (nread > 0)
				g_rxlen[r_no] += nread;
			}
		while (g_rxlen[r_no] >= 10) {
			unsigned char *m = g_rx[r_no];
			unsigned type = ((m[0] & 0x03) << 8) | m[1];
			unsigned long mlen = ((unsigned long)m[2] << 24) | ((unsigned long)m[3] << 16) |
				((unsigned long)m[4] << 8) | m[5];
			if (mlen < 10 || mlen > RXSZ) {
				g_rxlen[r_no] = 0;
				break;
				}
			if ((unsigned long)g_rxlen[r_no] < mlen)
				break;
			if (type == LLRP_READER_EVENT_NOTIFICATION)
				gotTs = parseReaderEventUTC(m + 10, (int)(mlen - 10), &readerUTC, &isUpt);
			memmove(m, m + mlen, g_rxlen[r_no] - (int)mlen);
			g_rxlen[r_no] -= (int)mlen;
			}
		if (!gotTs)
			Sleep(50);
		}
	// Tallenna kellopoikkeama (baseline); lukumaarat() nayttaa juoksevan kellon
	// LUKIJA/KELLO-laatikossa. Yhteystila nakyy jo laatikon varina, joten
	// alalaidan "Zebra-yhteys avattu" -ilmoitusta ei tarvita.
	measureClockOffset(r_no, gotTs && !isUpt, gotTs && isUpt, readerUTC);
	startInventory(cn);

	// Aseta GPO1 (summeri) pois alkutilaksi ja nollaa pulssin tila (myos reconnect).
	gpoActive[r_no] = 0;
	gpoOnMs[r_no] = 0;
	setGpo(cn, false);

	if (loki) {
		swprintf(wline, L"%d: FX9600 (LLRP) inventaario kaynnistetty portissa %d",
			mstimer(), ipparam[MAX_LAHPORTTI + r_no].destport);
		wkirjloki(wline);
		}
	return 0;
}

bool ZebraReader::isConnected(int r_no)
{
	return TCPyht_on(hComm[cn_regnly[r_no]]) != 0;
}

void ZebraReader::readTags(int r_no)
{
	int cn = cn_regnly[r_no];
	int nread = 0;

	// Lue saatavilla olevat tavut akkumulointipuskuriin.
	if (g_rxlen[r_no] < RXSZ - 2048) {
		read_TCP(hComm[cn], RXSZ - g_rxlen[r_no], (char *)(g_rx[r_no] + g_rxlen[r_no]), &nread);
		if (nread > 0)
			g_rxlen[r_no] += nread;
		}

	// Kasittele puskurista taydelliset LLRP-sanomat.
	while (g_rxlen[r_no] >= 10) {
		unsigned char *m = g_rx[r_no];
		unsigned type = ((m[0] & 0x03) << 8) | m[1];
		unsigned long mlen = ((unsigned long)m[2] << 24) | ((unsigned long)m[3] << 16) |
			((unsigned long)m[4] << 8) | m[5];
		if (mlen < 10 || mlen > RXSZ) {   // virheellinen pituus -> tyhjenna puskuri
			g_rxlen[r_no] = 0;
			break;
			}
		if ((unsigned long)g_rxlen[r_no] < mlen)
			break;                        // sanoma ei viela kokonaan saapunut

		if (type == LLRP_RO_ACCESS_REPORT)
			handleAccessReport(r_no, m + 10, (int)(mlen - 10));
		else if (type == LLRP_KEEPALIVE)
			sendLLRP(cn, LLRP_KEEPALIVE_ACK, 0, 0);
		// muut sanomat (tapahtumailmoitukset) ohitetaan

		memmove(m, m + mlen, g_rxlen[r_no] - (int)mlen);
		g_rxlen[r_no] -= (int)mlen;
		}

	// Takareuna (T/M): tarkista poistuneet tagit ja emittoi poistuma-ajat.
	checkDepartures(r_no);

	// GPO1-summeripulssin sammutus ei-blokkaavasti: kun sytytyksesta on kulunut
	// >= GPO_PULSE_MS, summeri pois. Sama mstimer-kuvio kuin checkDepartures.
	if (gpoActive[r_no] && mstimer() - gpoOnMs[r_no] >= GPO_PULSE_MS) {
		setGpo(cn, false);
		gpoActive[r_no] = 0;
		}
}

void ZebraReader::sync(bool setTime)
{
	// FX9600:n kello synkronoidaan ensisijaisesti NTP:lla; LLRP:lla ei ole
	// standardia kellonasetusta. Ei toimenpiteita.
}

void ZebraReader::readCmd(int r_no)
{
	// FX9600 ei kayta SIRIT:n kaltaista erillista komentokanavaa.
}

int ZebraReader::parseTime(INT32 *t, san_type *vastaus, aikatp *ut,
	INT *jono, int r_no)
{
	// readTags muotoilee tagit SIRIT-tekstimuotoon, joten sama jasennin
	// (siritaika) hoitaa EPC -> kilpailijanumero -muunnoksen ja ajan.
	return siritaika(t, vastaus, ut, jono, r_no);
}

#endif // LAJUNEN
