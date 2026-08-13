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

#include <stdio.h>
#include <string.h>
#include "SICenterJson.h"

// Hand-written, minimal JSON parser: handles only this one known schema
// (an array of flat objects with no nested structures, whose values are
// strings or integers). Not a general-purpose JSON library.

static const char *skipWs(const char *p)
{
	while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
		p++;
	return p;
}

// Parses a "..." string from p. Writes the unescaped content into out
// (capped at outsz-1 plus a null terminator). Returns a pointer past the
// closing quote, or NULL if p doesn't start with a quote or the string
// is unterminated.
static const char *parseJsonString(const char *p, char *out, int outsz)
{
	int oi = 0;

	if (*p != '"')
		return NULL;
	p++;
	while (*p && *p != '"') {
		char ch = *p;
		if (ch == '\\') {
			p++;
			if (!*p)
				return NULL;
			switch (*p) {
				case 'n': ch = '\n'; break;
				case 't': ch = '\t'; break;
				case 'r': ch = '\r'; break;
				case 'b': ch = '\b'; break;
				case 'f': ch = '\f'; break;
				case 'u':
					// \uXXXX: this schema's fields (numbers, type names,
					// modem ids) don't contain unicode code points;
					// skip 4 hex digits, write '?'.
					ch = '?';
					if (p[1] && p[2] && p[3] && p[4])
						p += 4;
					break;
				default: ch = *p; break;
				}
			}
		if (oi < outsz-1)
			out[oi++] = ch;
		p++;
		}
	if (*p != '"')
		return NULL;
	if (out)
		out[oi < outsz ? oi : outsz-1] = 0;
	return p+1;
}

// Parses a JSON number from p into *val (truncated to an integer - this
// schema's numeric fields are all documented as integers). Also consumes,
// but discards, a fractional part and/or exponent if present, so a
// surprise float in some field doesn't desync the rest of the parse.
// Returns a pointer past the number, or NULL if p doesn't contain a number.
static const char *parseJsonInt(const char *p, long long *val)
{
	int neg = 0;
	long long v = 0;
	const char *start;

	if (*p == '-') {
		neg = 1;
		p++;
		}
	start = p;
	while (*p >= '0' && *p <= '9') {
		v = v*10 + (*p - '0');
		p++;
		}
	if (p == start)
		return NULL;
	if (*p == '.') {
		p++;
		while (*p >= '0' && *p <= '9')
			p++;
		}
	if (*p == 'e' || *p == 'E') {
		p++;
		if (*p == '+' || *p == '-')
			p++;
		while (*p >= '0' && *p <= '9')
			p++;
		}
	*val = neg ? -v : v;
	return p;
}

// Skips one JSON value of any shape (string, number, true/false/null,
// object, or array) starting at p, without extracting anything from it.
// Used for fields this schema doesn't know about, so an unexpected shape
// there (e.g. a nested object) doesn't fail the whole parse. Returns a
// pointer past the value, or NULL if p isn't a valid JSON value.
static const char *skipJsonValue(const char *p)
{
	char tmp[256];
	long long num;

	if (*p == '"')
		return parseJsonString(p, tmp, sizeof(tmp));
	if (*p == '{') {
		p++;
		p = skipWs(p);
		if (*p == '}')
			return p+1;
		for (;;) {
			p = skipWs(p);
			p = parseJsonString(p, tmp, sizeof(tmp));
			if (!p)
				return NULL;
			p = skipWs(p);
			if (*p != ':')
				return NULL;
			p++;
			p = skipWs(p);
			p = skipJsonValue(p);
			if (!p)
				return NULL;
			p = skipWs(p);
			if (*p == ',') {
				p++;
				continue;
				}
			if (*p == '}')
				return p+1;
			return NULL;
			}
		}
	if (*p == '[') {
		p++;
		p = skipWs(p);
		if (*p == ']')
			return p+1;
		for (;;) {
			p = skipWs(p);
			p = skipJsonValue(p);
			if (!p)
				return NULL;
			p = skipWs(p);
			if (*p == ',') {
				p++;
				continue;
				}
			if (*p == ']')
				return p+1;
			return NULL;
			}
		}
	if (!strncmp(p, "true", 4))
		return p+4;
	if (!strncmp(p, "false", 5))
		return p+5;
	if (!strncmp(p, "null", 4))
		return p+4;
	return parseJsonInt(p, &num);
}

// Parses one { ... } object from p into punch. Returns a pointer past
// the closing '}', or NULL if the object is malformed.
static const char *parseJsonObject(const char *p, SIPunchTp *punch)
{
	char key[32];
	long long num;

	memset(punch, 0, sizeof(*punch));
	p = skipWs(p);
	if (*p != '{')
		return NULL;
	p++;
	p = skipWs(p);
	if (*p == '}')
		return p+1;
	for (;;) {
		p = skipWs(p);
		p = parseJsonString(p, key, sizeof(key));
		if (!p)
			return NULL;
		p = skipWs(p);
		if (*p != ':')
			return NULL;
		p++;
		p = skipWs(p);
		if (*p == '"') {
			char sval[64];
			p = parseJsonString(p, sval, sizeof(sval));
			if (!p)
				return NULL;
			if (!strcmp(key, "card"))
				strncpy(punch->card, sval, sizeof(punch->card)-1);
			else if (!strcmp(key, "type"))
				strncpy(punch->type, sval, sizeof(punch->type)-1);
			else if (!strcmp(key, "modem"))
				strncpy(punch->modem, sval, sizeof(punch->modem)-1);
			// other string fields (if the API expands) are ignored
			}
		else if ((*p >= '0' && *p <= '9') || *p == '-') {
			p = parseJsonInt(p, &num);
			if (!p)
				return NULL;
			if (!strcmp(key, "id"))
				punch->id = (long) num;
			else if (!strcmp(key, "time"))
				punch->time = num;
			else if (!strcmp(key, "code"))
				punch->code = (int) num;
			else if (!strcmp(key, "receptionTimeUtc"))
				punch->receptionTimeUtc = num;
			else if (!strcmp(key, "card"))
				// the live API sometimes sends card as a bare number
				// rather than the documented string
				snprintf(punch->card, sizeof(punch->card), "%lld", num);
			// other numeric fields are ignored
			}
		else {
			// null, true/false, or a nested object/array in a field this
			// schema doesn't know about - skip it without failing the parse
			p = skipJsonValue(p);
			if (!p)
				return NULL;
			}
		p = skipWs(p);
		if (*p == ',') {
			p++;
			continue;
			}
		if (*p == '}')
			return p+1;
		return NULL;
		}
}

int parseSIPunches(const char *json, SIPunchTp *out, int maxout)
{
	const char *p = json;
	int n = 0;

	// skip a UTF-8 byte-order-mark, if present (some JSON encoders emit one)
	if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF)
		p += 3;
	p = skipWs(p);
	if (*p != '[')
		return -1;
	p++;
	p = skipWs(p);
	if (*p == ']')
		return 0;
	for (;;) {
		SIPunchTp tmp;
		const char *next = parseJsonObject(p, (n < maxout) ? out+n : &tmp);
		if (!next)
			return -1;
		p = next;
		n++;
		p = skipWs(p);
		if (*p == ',') {
			p++;
			p = skipWs(p);
			continue;
			}
		if (*p == ']')
			break;
		return -1;
		}
	return (n < maxout) ? n : maxout;
}

int koodi2piste(const int *rastikoodi, int rastiluku, int koodi)
{
	int i;

	for (i = 0; i < rastiluku; i++)
		if (rastikoodi[i] == koodi)
			return i;
	return -1;
}

int siResolvePunch(const char *type, int code, int sistartkoodi, int isFinishCode,
	const int *rastikoodi, int rastiluku)
{
	int piste;

	if (!strcmp(type, "Start"))
		return SI_PUNCH_START;
	if (!strcmp(type, "Finish"))
		return SI_PUNCH_FINISH;
	if (sistartkoodi > 0 && code == sistartkoodi)
		return SI_PUNCH_START;
	if (isFinishCode)
		return SI_PUNCH_FINISH;
	piste = koodi2piste(rastikoodi, rastiluku, code);
	if (piste < 0)
		return SI_PUNCH_NOTFOUND;
	return piste+1;
}
