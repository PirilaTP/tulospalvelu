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

typedef struct {
	long id;
	char card[SI_PUNCH_CARD_LEN];
	long long time;
	int code;
	char type[SI_PUNCH_TYPE_LEN];
	char modem[SI_PUNCH_MODEM_LEN];
	long long receptionTimeUtc;
} SIPunchTp;

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
// Returns SI_PUNCH_START, SI_PUNCH_FINISH, a piste index >= 1 (already
// including the +1 that set_tulos()/setMaali() expect for numbered splits),
// or SI_PUNCH_NOTFOUND if code doesn't resolve to anything on the course.
int siResolvePunch(const char *type, int code, int sistartkoodi, int isFinishCode,
	const int *rastikoodi, int rastiluku);

#endif
