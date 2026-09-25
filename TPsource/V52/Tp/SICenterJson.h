// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
// Copyright (C) 2015 Pekka Pirila

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// One punch from the SportIdent Center REST API's /api/rest/v1/punches
// response. Field names and types match the API's documented JSON schema
// directly: id (integer), card (string), time (ms since epoch, local),
// code (control code), type ("Unknown"|"Control"|"Start"|"Finish"|"Check"|
// "Clear"), modem (string), receptionTimeUtc (ms since epoch, UTC).
// This translation unit is deliberately independent of global variables,
// the database, and Windows/VCL, so it can be compiled for unit tests
// (see Tests/SICenterJsonTest.cpp).

#ifndef SICENTERJSON_DEFINED
#define SICENTERJSON_DEFINED

#define SI_PUNCH_TYPE_LEN 10
#define SI_PUNCH_CARD_LEN 24
#define SI_PUNCH_MODEM_LEN 24

// Layout pinned with pack(8): tputil.h sets "#pragma option -a1" (1-byte
// alignment) for everything after it under the classic Borland compiler
// (bcc32). HkIV.cpp/VIv.cpp include tputil.h, SICenterJson.cpp does not,
// so without the pin the two sides would disagree on field offsets and on
// the element size of the punches[] array (see also SITulkinta.h).
#pragma pack(push, 8)
typedef struct {
	long id;
	char card[SI_PUNCH_CARD_LEN];
	long long time;
	int code;
	char type[SI_PUNCH_TYPE_LEN];
	char modem[SI_PUNCH_MODEM_LEN];
	long long receptionTimeUtc;
} SIPunchTp;
#pragma pack(pop)

// Parses a JSON array (the SportIdent Center REST API's /punches response)
// into out[0..maxout-1]. Unknown keys and object fields not known to this
// struct are silently skipped. Returns the number of punches parsed
// (capped at maxout), or -1 if json is not a valid JSON array (e.g.
// missing opening '['). An empty array "[]" returns 0.
int parseSIPunches(const char *json, SIPunchTp *out, int maxout);

// Searches for the code in rastikoodi[0..rastiluku-1]. Returns the
// 0-based index of the found element, or -1 if the code is not found.
int koodi2piste(const int *rastikoodi, int rastiluku, int koodi);

// Sentinel return values for siResolvePunch(), distinct from any valid
// piste index (always >= 1, matching set_tulos()/setMaali()'s existing
// piste+1 convention for numbered splits).
#define SI_PUNCH_START    -1
#define SI_PUNCH_FINISH    0
#define SI_PUNCH_NOTFOUND -2

// Resolves what a SIGPRS punch represents, using the same precedence
// HkIV.cpp/VIv.cpp's siParsePunch() applies for the "Control"/"Unknown"
// case: per SportIdent's own documentation, a punch of type "Unknown" (and,
// defensively, any other non-Start/Finish type) can actually be a Start or
// Finish, so the real meaning has to be derived from the control code
// rather than trusting the type string alone.
//
// isFinishCode is the caller's own maalirasti(rt, code)-equivalent result
// (whether code matches the competitor's course's Finish control, including
// any kuvio/pattern-code aliasing) - that logic depends on course data this
// module has no access to, so it's supplied rather than recomputed here.
//
// rastikoodi[0..rastiluku-1] is the list the split number is looked up
// from: the callers pass the series' split-point codes (Sarjat[].va_koodi,
// valuku entries), so entry i is split i+1 - not the course's control list,
// whose positions are not split numbers.
//
// Returns SI_PUNCH_START, SI_PUNCH_FINISH, a piste index >= 1 (already
// including the +1 that set_tulos()/setMaali() expect for numbered splits),
// or SI_PUNCH_NOTFOUND if code isn't the start, the finish or a split point.
int siResolvePunch(const char *type, int code, int sistartkoodi, int isFinishCode,
	const int *rastikoodi, int rastiluku);

// Whether siParsePunch may store a Center punch's time tm into a finish or
// split slot that currently holds ed (both in the program's internal clock
// units; TMAALI0 = no time yet). Same rule as tall_etulos: only an empty
// slot, or a time within uusinaika of the stored one (uusinaika 0 = never
// replace). So a delayed GPRS punch can't replace a photocell or manually
// corrected time, and of repeated punches the first one stays. Not used
// for start punches (the start field also holds the drawn start time).
bool siAikaSaaTallentaa(long ed, long tm, int uusinaika);

#endif
