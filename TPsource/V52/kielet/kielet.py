#!/usr/bin/env python3
# Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
# Copyright (C) 2015 Pekka Pirila
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Kieliversioiden työkalu (ks. kielet/README.md).

  python kielet/kielet.py poimi      lähdekoodista ja lomakkeista -> kielet/tulospalvelu.pot
  python kielet/kielet.py paivita    tulospalvelu.pot -> en.po ja sv.po (säilyttää käännökset)
  python kielet/kielet.py tarkista   tarkistaa käännökset (muotoilukoodit, pikanäppäimet, &)
  python kielet/kielet.py tilasto    käännettyjen määrät

Aja hakemistosta TPsource/V52 tai mistä tahansa; polut lasketaan tämän tiedoston paikasta.
Vaatii Python 3.8+, ei ulkoisia kirjastoja.
"""

import os
import re
import sys
from collections import OrderedDict

V52 = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KIELIHAK = os.path.join(V52, 'kielet')
POT = os.path.join(KIELIHAK, 'tulospalvelu.pot')
KIELET = ['en', 'sv']

# Hakemistot, joista tekstejä poimitaan
LAHDEHAK = ['Hk', 'Juk', 'HkConsole', 'JukConsole', 'Tp', 'tputilv2', 'cbHk', 'ViestiWin', 'cbTp', 'com']

# Funktiot, joiden merkkijonoparametri on käännösavain: nimi -> (parametrin indeksi, konteksti-indeksi)
KAANNOSFUNKTIOT = {
    'TR': (0, None),
    'TRA': (0, None),
    'TRL': (1, None),
    'TRCL': (2, 1),
    'N_': (0, None),
}
# Konsolin valintakehotteet, jotka kääntyvät funktion sisällä: nimi -> (kehote, sallitut merkit)
VALINTAFUNKTIOT = {
    'wselectopt': (0, 1),
    'selectopt': (0, 1),
    'wselectxy': (2, 3),
    'selectxy': (2, 3),
}

# Lomakkeiden käännettävät ominaisuudet (ks. cbTp/cbKielet.cpp)
DFM_OMINAISUUDET = {'Caption', 'Hint', 'EditLabel.Caption', 'Items.Strings', 'Title', 'Filter',
                    'Lines.Strings', 'Title.Caption'}


# ---------------------------------------------------------------------------
# Lähdetiedostojen lukeminen
# ---------------------------------------------------------------------------

def lue_lahde(polku):
    """Lukee lähdetiedoston niin kuin kääntäjä sen tulkitsee: BOM -> UTF-8, muuten Windows-1252."""
    b = open(polku, 'rb').read()
    if b.startswith(b'\xef\xbb\xbf'):
        return b[3:].decode('utf-8'), 'utf-8-sig'
    try:
        b.decode('utf-8')
        enc = 'utf-8' if any(x >= 0x80 for x in b) else 'ascii'
    except UnicodeDecodeError:
        enc = 'cp1252'
    # Ilman BOMia kääntäjät lukevat tiedoston Windows-1252:na; tekstien avaimet lasketaan samoin.
    return b.decode('cp1252', errors='replace'), enc


ESCAPET = {'n': '\n', 't': '\t', 'r': '\r', 'a': '\a', 'b': '\b', 'f': '\f', 'v': '\v',
           '\\': '\\', '"': '"', "'": "'", '?': '?', '0': '\0'}


def pura_c(raaka):
    """Purkaa C-merkkijonovakion sisällön (ilman lainausmerkkejä) Python-merkkijonoksi."""
    out = []
    i = 0
    while i < len(raaka):
        c = raaka[i]
        if c != '\\':
            out.append(c)
            i += 1
            continue
        i += 1
        if i >= len(raaka):
            break
        c = raaka[i]
        if c == 'x':
            j = i + 1
            while j < len(raaka) and raaka[j] in '0123456789abcdefABCDEF':
                j += 1
            out.append(chr(int(raaka[i+1:j] or '0', 16)))
            i = j
        elif c in '01234567':
            j = i
            while j < len(raaka) and j < i + 3 and raaka[j] in '01234567':
                j += 1
            out.append(chr(int(raaka[i:j], 8)))
            i = j
        else:
            out.append(ESCAPET.get(c, c))
            i += 1
    return ''.join(out)


TOKEN_RE = re.compile(r'''
    (?P<kommentti>//[^\n]*|/\*.*?\*/)
  | (?P<esik>^[ \t]*\#(?:[^\n\\]|\\.|\\\n)*)
  | (?P<str>(?:L|u8|u|U)?"(?:[^"\\\n]|\\.)*")
  | (?P<chr>(?:L)?'(?:[^'\\\n]|\\.)*')
  | (?P<ident>[A-Za-z_][A-Za-z0-9_]*)
  | (?P<num>\d[\w.]*)
  | (?P<rivi>\n)
  | (?P<tyhja>[ \t\r\f\v]+)
  | (?P<p>->|::|.)
''', re.S | re.M | re.X)


class Token:
    __slots__ = ('laji', 'arvo', 'alku', 'loppu', 'rivi')

    def __init__(self, laji, arvo, alku, loppu, rivi):
        self.laji, self.arvo, self.alku, self.loppu, self.rivi = laji, arvo, alku, loppu, rivi

    def __repr__(self):
        return f'{self.laji}:{self.arvo!r}@{self.rivi}'


def tokenit(teksti):
    """C++-tokenit ilman kommentteja, esikääntäjärivejä ja tyhjää."""
    rivi = 1
    tulos = []
    for m in TOKEN_RE.finditer(teksti):
        laji = m.lastgroup
        arvo = m.group()
        if laji in ('str', 'chr', 'ident', 'num', 'p'):
            tulos.append(Token(laji, arvo, m.start(), m.end(), rivi))
        rivi += arvo.count('\n')
    return tulos


def str_ryhma(tok, i):
    """Peräkkäiset merkkijonovakiot kohdasta i: (loppuindeksi, teksti, onko leveä)."""
    osat = []
    leve = False
    j = i
    while j < len(tok) and tok[j].laji == 'str':
        a = tok[j].arvo
        if a.startswith('L'):
            leve = True
            a = a[1:]
        elif a.startswith('u8'):
            a = a[2:]
        elif a[0] in 'uU':
            a = a[1:]
        osat.append(pura_c(a[1:-1]))
        j += 1
    return j, ''.join(osat), leve


def kutsun_parametrit(tok, i):
    """tok[i] on '(' -> lista parametreista tokenien indeksiväleinä [(alku, loppu)], ja sulkevan sulun indeksi."""
    syv = 0
    params = []
    alku = i + 1
    j = i
    while j < len(tok):
        t = tok[j]
        if t.laji == 'p' and t.arvo in '([{':
            syv += 1
        elif t.laji == 'p' and t.arvo in ')]}':
            syv -= 1
            if syv == 0:
                if j > alku or params:
                    params.append((alku, j))
                return params, j
        elif t.laji == 'p' and t.arvo == ',' and syv == 1:
            params.append((alku, j))
            alku = j + 1
        j += 1
    return params, j


def lahdetiedostot():
    for hak in LAHDEHAK:
        d = os.path.join(V52, hak)
        if not os.path.isdir(d):
            continue
        for nimi in sorted(os.listdir(d)):
            if nimi.lower().endswith(('.cpp', '.h', '.c')):
                yield os.path.join(d, nimi)


def suhteellinen(polku):
    return os.path.relpath(polku, V52).replace('\\', '/')


# ---------------------------------------------------------------------------
# Poiminta
# ---------------------------------------------------------------------------

class Luettelo:
    """Poimitut tekstit: avain (konteksti, msgid) -> {'viitteet': [...], 'huom': set()}"""

    def __init__(self):
        self.tekstit = OrderedDict()

    def lisaa(self, msgid, viite, ktx=None, huom=None):
        if not msgid or not re.search(r'[A-Za-zÅÄÖåäöÜü]', msgid):
            return
        k = (ktx, msgid)
        e = self.tekstit.setdefault(k, {'viitteet': [], 'huom': []})
        if viite not in e['viitteet']:
            e['viitteet'].append(viite)
        if huom and huom not in e['huom']:
            e['huom'].append(huom)


def poimi_lahde(polku, luettelo, varoitukset):
    teksti, enc = lue_lahde(polku)
    tok = tokenit(teksti)
    rel = suhteellinen(polku)
    for i, t in enumerate(tok):
        if t.laji != 'ident' or i + 1 >= len(tok) or tok[i+1].arvo != '(':
            continue
        if i > 0 and tok[i-1].arvo in ('.', '->', '::') and t.arvo not in ('TR',):
            continue
        nimi = t.arvo
        if nimi in KAANNOSFUNKTIOT:
            ind, kind = KAANNOSFUNKTIOT[nimi]
        elif nimi in VALINTAFUNKTIOT:
            ind, kind = VALINTAFUNKTIOT[nimi][0], None
        else:
            continue
        params, _ = kutsun_parametrit(tok, i + 1)
        if ind >= len(params):
            continue
        a, l = params[ind]
        if tok[a].laji != 'str':
            continue
        j, msgid, _ = str_ryhma(tok, a)
        if j != l:
            continue
        ktx = None
        if kind is not None and kind < len(params):
            ka, kl = params[kind]
            if tok[ka].laji == 'str':
                _, ktx, _ = str_ryhma(tok, ka)
        huom = None
        if nimi in ('N_', 'TR', 'TRA') and (pikanappaimet(msgid) or '(K/E)' in msgid):
            # Muuttujaan koottu valintakehote (wcscpy/swprintf -> wselectopt(muuttuja))
            alku = teksti.rfind('\n', 0, t.alku) + 1
            mk = re.search(r'\b(wcscpy|wcsncpy|strcpy|swprintf|sprintf|wcscat|strcat)\s*\(\s*(\w+)',
                           teksti[alku:t.alku])
            if mk:
                jatko = '\n'.join(teksti[t.loppu:].split('\n')[:15])
                if re.search(r'\b(w?selectopt|w?selectxy|Wselectopt)\s*\(\s*' + re.escape(mk.group(2)) + r'\b', jatko):
                    # N_: wselectopt kääntää ja vaihtaa näppäimet. TR: teksti on jo muotoiltu, kun
                    # wselectopt saa sen, joten näppäimiä ei voi vaihtaa: käännöksessä samat kirjaimet.
                    huom = 'valinta' if nimi == 'N_' else 'valinta-kiintea: pidä suomenkieliset pikanäppäimet'
        if nimi in VALINTAFUNKTIOT:
            ta, tl = params[VALINTAFUNKTIOT[nimi][1]] if VALINTAFUNKTIOT[nimi][1] < len(params) else (0, 0)
            if tl > ta and tok[ta].laji == 'str':
                _, term, _ = str_ryhma(tok, ta)
                term = ''.join(c for c in term if c.isalnum())
                huom = 'valinta: ' + term
            else:
                huom = 'valinta'
        if enc == 'utf-8' and any(ord(c) >= 0x80 for c in msgid):
            varoitukset.append(f'{rel}:{tok[a].rivi}: UTF-8-tiedostossa ilman BOMia ei-ASCII-teksti: {msgid!r}')
        luettelo.lisaa(msgid, rel, ktx, huom)


# --- Lomakkeet (.dfm) ---

DFM_OBJ = re.compile(r'^\s*(object|inherited|inline)\s+(\w+)\s*:\s*(\w+)', re.M)
DFM_PROP = re.compile(r'^[ \t]*([\w.]+)[ \t]*=[ \t]*', re.M)
DFM_TOKEN = re.compile(r"""'(?:[^'\n]|'')*'|\#\d+|\+|\(|\)|\n|[ \t\r]+|[^\s'#+()]+""")


def lue_dfm(polku):
    with open(polku, 'rb') as tiedosto:
        b = tiedosto.read()
    if b.startswith(b'\xef\xbb\xbf'):
        return b[3:].decode('utf-8')
    try:
        return b.decode('utf-8')
    except UnicodeDecodeError:
        return b.decode('cp1252', errors='replace')


def dfm_arvo(teksti, pos):
    """Lukee ominaisuuden arvon kohdasta pos. Palauttaa (merkkijonolista tai None, loppukohta).
    Skalaari: ['teksti'], lista ( ... ): kaikki merkkijonoalkiot, muu arvo: None."""
    alkiot = []
    nyk = None          # käsiteltävä merkkijono
    lista = False
    jatkuu = False      # edellinen rivi päättyi +:aan
    ei_merkkijono = False
    for m in DFM_TOKEN.finditer(teksti, pos):
        t = m.group()
        if t[0] in ' \t\r':
            continue
        if t == '(' and not lista and nyk is None and not alkiot:
            lista = True
            continue
        if t[0] == "'" or t[0] == '#':
            osa = chr(int(t[1:])) if t[0] == '#' else t[1:-1].replace("''", "'")
            nyk = osa if nyk is None else nyk + osa
            jatkuu = False
            continue
        if t == '+':
            jatkuu = True
            continue
        if t == '\n':
            # Pitka Caption voi alkaa vasta '='-rivia seuraavalta rivilta.
            if jatkuu or (nyk is None and not alkiot):
                continue
            if nyk is not None:
                alkiot.append(nyk)
                nyk = None
            if not lista:
                return (None if ei_merkkijono else alkiot), m.end()
            continue
        if t == ')' and lista:
            if nyk is not None:
                alkiot.append(nyk)
            return (None if ei_merkkijono else alkiot), m.end()
        # Muu arvo (luku, tunniste, {binääri}, ...)
        ei_merkkijono = True
        if not lista:
            # skalaari, joka ei ole merkkijono: ohitetaan rivin loppuun
            loppu = teksti.find('\n', m.end())
            return None, (len(teksti) if loppu < 0 else loppu + 1)
    if nyk is not None:
        alkiot.append(nyk)
    return (None if ei_merkkijono else alkiot), len(teksti)


def dfm_tekstit(polku):
    """Tuottaa (komponenttipolku 'Lomake.Komponentti', luokka, ominaisuus, teksti, rivi)."""
    teksti = lue_dfm(polku).replace('\r\n', '\n')
    pino = []
    pos = 0
    while pos < len(teksti):
        loppu = teksti.find('\n', pos)
        if loppu < 0:
            loppu = len(teksti)
        rivi = teksti[pos:loppu]
        rivino = teksti.count('\n', 0, pos) + 1
        m = DFM_OBJ.match(rivi)
        if m:
            pino.append((m.group(2), m.group(3)))
            pos = loppu + 1
            continue
        if rivi.strip() == 'end':
            if pino:
                pino.pop()
            pos = loppu + 1
            continue
        m = DFM_PROP.match(rivi)
        if not m:
            pos = loppu + 1
            continue
        arvot, pos = dfm_arvo(teksti, pos + m.end())
        if arvot and m.group(1) in DFM_OMINAISUUDET and pino:
            for a in arvot:
                yield '.'.join(p[0] for p in pino), pino[-1][1], m.group(1), a, rivino


def poimi_dfm(polku, luettelo):
    rel = suhteellinen(polku)
    for obj, luokka, prop, teksti, rivi in dfm_tekstit(polku):
        if prop == 'Filter':
            # Vain kuvaukset, ei tiedostomaskeja: 'Tekstitiedostot (*.txt)|*.txt|...'
            osat = teksti.split('|')
            for k in range(0, len(osat), 2):
                luettelo.lisaa(osat[k], rel, huom='suodatin')
            continue
        luettelo.lisaa(teksti, rel)


def poimi(argv):
    luettelo = Luettelo()
    varoitukset = []
    for polku in lahdetiedostot():
        poimi_lahde(polku, luettelo, varoitukset)
    for hak in ('cbHk', 'ViestiWin', 'cbTp'):
        d = os.path.join(V52, hak)
        for nimi in sorted(os.listdir(d)):
            if nimi.lower().endswith('.dfm'):
                poimi_dfm(os.path.join(d, nimi), luettelo)
    kirjoita_po(POT, luettelo_poksi(luettelo), otsikko_pot())
    for v in varoitukset:
        print('VAROITUS', v)
    print(f'{len(luettelo.tekstit)} tekstiä -> {suhteellinen(POT)}')


# ---------------------------------------------------------------------------
# .po-tiedostot
# ---------------------------------------------------------------------------

class Merkinta:
    def __init__(self):
        self.ktx = None
        self.msgid = ''
        self.msgstr = ''
        self.viitteet = []
        self.huom = []        # #. -kommentit
        self.kaantajalle = []  # # -kommentit
        self.liput = []
        self.vanhentunut = False

    @property
    def avain(self):
        return (self.ktx, self.msgid)


def luettelo_poksi(luettelo):
    tulos = []
    for (ktx, msgid), e in luettelo.tekstit.items():
        m = Merkinta()
        m.ktx, m.msgid = ktx, msgid
        m.viitteet = e['viitteet']
        m.huom = e['huom']
        if re.search(r'%[-+ #0]*\d*(?:\.\d+)?[hlLwI]*[diouxXeEfgGcsSp]', msgid):
            m.liput.append('c-format')
        tulos.append(m)
    return tulos


def po_lainaus(s):
    s = s.replace('\\', '\\\\').replace('"', '\\"').replace('\t', '\\t').replace('\r', '\\r')
    s = ''.join(c if ord(c) >= 32 or c == '\n' else '\\x%02x' % ord(c) for c in s)
    if '\n' in s.rstrip('\n') or len(s) > 70:
        osat = re.split(r'(?<=\n)', s)
        osat = [o.replace('\n', '\\n') for o in osat if o]
        return '""\n' + '\n'.join(f'"{o}"' for o in osat)
    return '"' + s.replace('\n', '\\n') + '"'


def kirjoita_po(polku, merkinnat, otsikko):
    rivit = [otsikko.rstrip('\n'), '']
    vanhat = []
    for m in merkinnat:
        if m.vanhentunut:
            vanhat.append(m)
            continue
        for k in m.kaantajalle:
            rivit.append('# ' + k if k else '#')
        for h in m.huom:
            rivit.append('#. ' + h)
        if m.viitteet:
            for k in range(0, len(m.viitteet), 4):
                rivit.append('#: ' + ' '.join(v.replace(' ', '_') for v in m.viitteet[k:k+4]))
        if m.liput:
            rivit.append('#, ' + ', '.join(m.liput))
        if m.ktx is not None:
            rivit.append('msgctxt ' + po_lainaus(m.ktx))
        rivit.append('msgid ' + po_lainaus(m.msgid))
        rivit.append('msgstr ' + po_lainaus(m.msgstr))
        rivit.append('')
    for m in vanhat:
        if m.ktx is not None:
            rivit.append('#~ msgctxt ' + po_lainaus(m.ktx).replace('\n', '\n#~ '))
        rivit.append('#~ msgid ' + po_lainaus(m.msgid).replace('\n', '\n#~ '))
        rivit.append('#~ msgstr ' + po_lainaus(m.msgstr).replace('\n', '\n#~ '))
        rivit.append('')
    with open(polku, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(rivit))


def otsikko_pot():
    return ('# Tulospalveluohjelman käännettävät tekstit.\n'
            '# Tämä tiedosto tuotetaan komennolla: python kielet/kielet.py poimi\n'
            '# Älä muokkaa käsin; käännökset ovat tiedostoissa en.po ja sv.po.\n'
            'msgid ""\n'
            'msgstr ""\n'
            '"Project-Id-Version: tulospalvelu\\n"\n'
            '"MIME-Version: 1.0\\n"\n'
            '"Content-Type: text/plain; charset=UTF-8\\n"\n'
            '"Content-Transfer-Encoding: 8bit\\n"\n'
            '"Language: fi\\n"\n')


def otsikko_po(kieli):
    nimi = {'en': 'englanti', 'sv': 'ruotsi'}[kieli]
    return (f'# Tulospalveluohjelman käännökset: {nimi} ({kieli}).\n'
            '# Avaimena (msgid) on ohjelman suomenkielinen teksti. Tyhjä msgstr = näytetään suomeksi.\n'
            '# Päivitys lähdekoodista: python kielet/kielet.py poimi && python kielet/kielet.py paivita\n'
            '# Tarkistus: python kielet/kielet.py tarkista\n'
            'msgid ""\n'
            'msgstr ""\n'
            '"Project-Id-Version: tulospalvelu\\n"\n'
            '"MIME-Version: 1.0\\n"\n'
            '"Content-Type: text/plain; charset=UTF-8\\n"\n'
            '"Content-Transfer-Encoding: 8bit\\n"\n'
            f'"Language: {kieli}\\n"\n')


def lue_po(polku):
    """Lukee .po-tiedoston merkinnät (ohittaa otsikkomerkinnän)."""
    merkinnat = []
    if not os.path.exists(polku):
        return merkinnat
    m = Merkinta()
    kentta = None

    def valmis():
        nonlocal m
        if m.msgid or m.ktx is not None:
            merkinnat.append(m)
        m = Merkinta()

    def lainaus(s):
        s = s.strip()
        return pura_c(s[1:-1]) if s.startswith('"') and s.endswith('"') else ''

    for r in open(polku, encoding='utf-8-sig'):
        r = r.rstrip('\r\n')
        vanha = r.startswith('#~')
        if vanha:
            r = r[2:].lstrip()
        if not r.strip():
            if kentta == 'msgstr':
                valmis()
            kentta = None
            continue
        if kentta == 'msgstr' and (r.startswith('#') or r.startswith('msgctxt') or
                                   (r.startswith('msgid') and not r.startswith('msgid_plural'))):
            valmis()
            kentta = None
        if vanha:
            m.vanhentunut = True
        if r.startswith('#:'):
            m.viitteet += r[2:].split()
        elif r.startswith('#.'):
            m.huom.append(r[2:].strip())
        elif r.startswith('#,'):
            m.liput += [x.strip() for x in r[2:].split(',') if x.strip()]
        elif r.startswith('#'):
            m.kaantajalle.append(r[1:].strip())
        elif r.startswith('msgctxt'):
            kentta = 'msgctxt'
            m.ktx = lainaus(r[7:])
        elif r.startswith('msgid_plural'):
            kentta = 'ohita'
        elif r.startswith('msgid'):
            kentta = 'msgid'
            m.msgid = lainaus(r[5:])
        elif r.startswith('msgstr'):
            kentta = 'msgstr'
            m.msgstr = lainaus(r[r.index(' '):]) if not r.startswith('msgstr[') or r.startswith('msgstr[0]') else m.msgstr
        elif r.startswith('"'):
            if kentta == 'msgctxt':
                m.ktx += lainaus(r)
            elif kentta == 'msgid':
                m.msgid += lainaus(r)
            elif kentta == 'msgstr':
                m.msgstr += lainaus(r)
    if kentta == 'msgstr':
        valmis()
    return [x for x in merkinnat if x.msgid]


def paivita(argv):
    pot = lue_po(POT)
    for kieli in KIELET:
        polku = os.path.join(KIELIHAK, kieli + '.po')
        vanhat = OrderedDict((m.avain, m) for m in lue_po(polku))
        uudet = []
        for p in pot:
            m = Merkinta()
            m.ktx, m.msgid, m.viitteet, m.huom = p.ktx, p.msgid, p.viitteet, p.huom
            m.liput = [l for l in p.liput if l != 'fuzzy']
            v = vanhat.pop(p.avain, None)
            if v is not None:
                m.msgstr = v.msgstr
                m.kaantajalle = v.kaantajalle
                if 'fuzzy' in v.liput:
                    m.liput.append('fuzzy')
            uudet.append(m)
        for v in vanhat.values():
            if v.msgstr:
                v.vanhentunut = True
                v.viitteet = []
                uudet.append(v)
        kirjoita_po(polku, uudet, otsikko_po(kieli))
        kaannetty = sum(1 for m in uudet if m.msgstr and not m.vanhentunut)
        print(f'{kieli}.po: {kaannetty}/{len(pot)} käännetty')


# ---------------------------------------------------------------------------
# Tarkistus
# ---------------------------------------------------------------------------

MUOTOILU = re.compile(r'%(?:%|[-+ #0]*(?:\d+|\*)?(?:\.(?:\d+|\*))?[hlLwIjzt]*[diouxXeEfgGaAcsSpn])')


def pikanappaimet(s):
    """Sama logiikka kuin Kielet.cpp:n pikanappaimet()."""
    tulos = []
    for i, c in enumerate(s):
        if not c.isupper():
            continue
        ed = s[i-1] if i > 0 else ' '
        seur = s[i+1] if i + 1 < len(s) else ''
        if (seur == ')' and not ed.isupper()) or (ed in '(/' and seur == '/'):
            tulos.append(c)
    return tulos


def tarkista(argv):
    virheita = 0
    for kieli in KIELET:
        polku = os.path.join(KIELIHAK, kieli + '.po')
        for m in lue_po(polku):
            if not m.msgstr or m.vanhentunut:
                continue
            ongelmat = []
            if [x for x in MUOTOILU.findall(m.msgid) if x != '%%'] != \
                    [x for x in MUOTOILU.findall(m.msgstr) if x != '%%']:
                ongelmat.append('muotoilukoodit eroavat')
            for h in m.huom:
                if h.startswith('valinta-kiintea'):
                    # Näppäimet ovat suomenkieliset: käännöksessä saa näkyä vain ne (esim. "S: samma")
                    vieraat = set(pikanappaimet(m.msgstr)) - set(pikanappaimet(m.msgid))
                    if vieraat or ('(K/E)' in m.msgid and re.search(r'\((J/N|Y/N)\)', m.msgstr)):
                        ongelmat.append('näppäimiä ei vaihdeta: käännöksessä vain suomenkieliset näppäimet')
                elif h.startswith('valinta'):
                    a, k = pikanappaimet(m.msgid), pikanappaimet(m.msgstr)
                    term = h.split(':', 1)[1].strip() if ':' in h else ''
                    if len(a) != len(k):
                        ongelmat.append(f'pikanäppäimiä {len(k)}, suomessa {len(a)}')
                    else:
                        kaan = [k[a.index(c)] if c in a else c for c in term]
                        if len(set(kaan)) != len(kaan):
                            ongelmat.append('pikanäppäimet eivät ole yksikäsitteisiä: ' + ''.join(kaan))
            if '&' in m.msgid.replace('&&', '') and '&' not in m.msgstr.replace('&&', ''):
                ongelmat.append('&-pikanäppäin puuttuu')
            if m.msgid.count('\n') != m.msgstr.count('\n'):
                ongelmat.append('rivinvaihtojen määrä eroaa')
            if len(m.msgid) != len(m.msgstr) and \
                    (m.msgid[:1].isspace(), m.msgid[-1:].isspace()) != (m.msgstr[:1].isspace(), m.msgstr[-1:].isspace()):
                ongelmat.append('alku- tai loppuvälilyönnit eroavat')
            for o in ongelmat:
                virheita += 1
                print(f'{kieli}.po: {m.viitteet[0] if m.viitteet else "?"}: {o}: {m.msgid!r} -> {m.msgstr!r}')
    print(f'{virheita} ongelmaa')
    return 1 if virheita else 0


def tilasto(argv):
    pot = lue_po(POT)
    print(f'tulospalvelu.pot: {len(pot)} tekstiä')
    for kieli in KIELET:
        po = [m for m in lue_po(os.path.join(KIELIHAK, kieli + '.po')) if not m.vanhentunut]
        k = sum(1 for m in po if m.msgstr)
        print(f'{kieli}.po: {k}/{len(po)} käännetty')


# ---------------------------------------------------------------------------
# Käärintä: merkkijonovakiot TR()-kutsuun käyttöliittymän tulostuskohdissa
# ---------------------------------------------------------------------------

# Kutsut, joiden kaikki merkkijonoparametrit näkyvät käyttäjälle
UI_KUTSUT = {'MessageBoxW', 'MessageBox', 'ShowMessage', 'writeerror_w', 'writeerror', 'writeerrorkysy',
             'writewarning_w', 'vidspwmsg', 'vidspmsg', 'viwrrect', 'viwrrectw', 'inputint_prompt',
             'inputstr_prompt', 'inputaika_prompt', 'select3', 'InputBox', 'InputQuery', 'MessageDlg',
             'perr_w', 'perr'}
# Kokoelmien metodit (Items->Add(...), Lines->Add(...)), kun edellä on jokin näistä
UI_KOKOELMAT = {'Items', 'Lines', 'Strings'}
UI_KOKOELMAMETODIT = {'Add', 'Insert'}
# Muotoilu- ja kopiointifunktiot: käännetään vain käyttöliittymätiedostoissa ja vain tekstin näköiset
UI_MUOTOILU = {'swprintf', 'swprintf_s', '_snwprintf', 'snwprintf', 'wsprintf', 'sprintf', 'wcscpy', 'wcsncpy',
               'wcscat', 'wcsncat', 'strcpy', 'strcat', 'strncpy', 'strncat', 'wcscpy_s', 'wcscat_s'}
# Sijoitukset ominaisuuksiin: X->Caption = L"..."
UI_OMINAISUUDET = {'Caption', 'Hint', 'Title', 'SimpleText'}
# Kääreet, joiden läpi katsotaan ulompaan kutsuun
NEUTRAALIT = {'UnicodeString', 'AnsiString', 'String', 'WideString', 'wchar_t', 'char', 'LPWSTR', 'LPCWSTR',
              'const'}
KAANNOSKUTSUT = {'TR', 'TRA', 'TRL', 'TRCL', 'KT', 'N_'}
# Tulosteita kirjoittavat kutsut: funktiossa, jossa näitä on, muotoilufunktioiden tekstit
# kuuluvat tulosteeseen (tulosteen kieli), eivät käyttöliittymään
TULOSTEKUTSUT = {'putfld', 'Putfld', 'writehtml', 'put_wantitag', 'put_wtag', 'WriteLine', 'sendln',
                 'sendwline', 'fwprintf', 'writeline', 'fprintf', 'fputs', 'fputws', 'paaots_on', 'wrt_st',
                 'put_wxml_s', 'put_wxml_d', 'initpage', 'endpage', 'newline', 'putfldw'}


VERTAILUKUTSUT = {'wcscmp', 'wcsncmp', 'wmemcmp', 'wmemcmpU', 'wcscmpU', '_wcsicmp', '_wcsnicmp', 'wcsicmp',
                  'strcmp', 'strncmp', 'memcmp', 'stricmp', '_stricmp', 'strnicmp', 'wcsstr', 'strstr', 'Pos',
                  'CompareText', 'CompareStr', 'AnsiCompareText', 'SameText'}


def on_nimike(s):
    """Lomakkeen ominaisuuksien ja listojen tekstit: kirjaimia, ei avainsanan tai tiedostorivin näköinen."""
    if not re.search(r'[A-Za-zÄÖÅäöå]{2}', s) or not re.search(r'[a-zäöå]', s) and ' ' not in s.strip():
        return False        # ei kirjaimia tai yksi isokirjaiminen sana (avainsana)
    if '=' in s or s.lstrip().startswith(('//', '<', '#')) or '://' in s:
        return False
    return True


def nayttaa_tekstilta(s):
    """Heuristiikka muotoilu- ja kopiointifunktioiden vakioille: onko käyttäjälle näkyvää tekstiä."""
    if not re.search(r'[A-Za-zÄÖÅäöå]{2}', s):
        return False
    if re.search(r'<[A-Za-z/!]', s) or re.search(r'\w=\s*$', s) or re.search(r'\w="', s):
        return False        # html/xml tai asetusavain
    if '://' in s or '\t' in s:
        return False        # osoite tai sarkaimin erotettu tiedostorivi
    if re.search(r'\w=[%\w"]', s) or re.match(r'\s*%\w+:\s', s):
        return False        # laitteen tai tiedoston avain=arvo, lokirivi "%d: ..."
    if '\\' in s or re.search(r'\.(xml|txt|csv|html?|lst|cfg|dat|kil|ini|log|pdf|xlsx?|json|css|js)$', s, re.I):
        return False        # tiedostopolku
    puhdas = re.sub(r'%[-+ #0]*\d*(?:\.\d+)?[hlLwI]*[a-zA-Z]', ' ', s).strip()
    if re.fullmatch(r'[\w.\-\\/:*?]+\.[A-Za-z0-9]{1,4}', puhdas):
        return False        # tiedostonimi
    if re.search(r'[a-zäöå]{2}', puhdas) and (' ' in puhdas or re.match(r'[A-ZÄÖÅ]', puhdas)):
        return True
    if re.search(r'[A-ZÄÖÅ]{2,}\s+[A-ZÄÖÅ]{2,}', puhdas):
        return True         # isoin kirjaimin kirjoitettu ilmoitus
    return False


def _valilla_esik(teksti, a, b):
    """Onko kohtien a..b välissä muuta kuin tyhjää tai kommentteja (esim. #if)."""
    vali = teksti[a:b]
    vali = re.sub(r'//[^\n]*|/\*.*?\*/', '', vali, flags=re.S)
    return vali.strip() != ''


def kaari_teksti(teksti, ui_tiedosto):
    """Palauttaa (uusi teksti, käärittyjen määrä, ohitetut). Teksti on tavu per merkki (latin-1)."""
    tok = tokenit(teksti)
    n = len(tok)
    # Sulkupino: jokaiselle tokenille sitä ympäröivä avaava sulku
    ymparoiva = [None] * n
    pino = []
    for i, t in enumerate(tok):
        ymparoiva[i] = pino[-1] if pino else None
        if t.laji == 'p' and t.arvo in '([{':
            pino.append(i)
        elif t.laji == 'p' and t.arvo in ')]}':
            if pino:
                pino.pop()

    def kutsun_nimi(sulku):
        """'(' -indeksistä kutsun nimi ja sitä edeltävät tokenit."""
        if sulku is None or tok[sulku].arvo != '(' or sulku == 0:
            return None
        e = tok[sulku-1]
        return e.arvo if e.laji == 'ident' else None

    # Ylimmän tason lohkot (funktiot) ja niistä ne, jotka kirjoittavat tulosteita
    lohko = [None] * n
    tulostelohkot = set()
    syv = 0
    nyk = None
    for k, t in enumerate(tok):
        if t.arvo == '{':
            if syv == 0:
                nyk = k
            syv += 1
        elif t.arvo == '}':
            syv -= 1
            if syv <= 0:
                syv = 0
        lohko[k] = nyk if syv > 0 else None
        if t.laji == 'ident' and t.arvo in TULOSTEKUTSUT and syv > 0 and k + 1 < n and tok[k+1].arvo == '(':
            tulostelohkot.add(nyk)

    muutokset = []
    ohitetut = []
    kaaritut = set()
    vertailut = []
    i = 0
    while i < n:
        t = tok[i]
        if t.laji != 'str':
            i += 1
            continue
        loppu, sisalto, leve = str_ryhma(tok, i)
        # Ryhmän osien välissä ei saa olla esikääntäjäehtoja
        if any(_valilla_esik(teksti, tok[k].loppu, tok[k+1].alku) for k in range(i, loppu - 1)):
            ohitetut.append((t.rivi, 'esikääntäjä', sisalto))
            i = loppu
            continue
        if not re.search(r'[A-Za-zÄÖÅäöå]{2}', sisalto) or tok[i].arvo.startswith(('u8', 'u"', 'U"')):
            i = loppu
            continue
        seuraava = tok[loppu].arvo if loppu < n else ''
        edellinen = tok[i-1].arvo if i > 0 else ''
        kaari = False
        # Etsi lähin nimetty kutsu neutraalien kääreiden ja ryhmittelysulkujen läpi
        s = ymparoiva[i]
        nimi = None
        while s is not None and tok[s].arvo == '(':
            nimi = kutsun_nimi(s)
            if nimi is None or nimi in NEUTRAALIT:
                s = ymparoiva[s]
                nimi = None
                continue
            break
        if seuraava == '[' or edellinen == 'sizeof':
            pass
        elif nimi in KAANNOSKUTSUT:
            pass
        elif nimi in UI_KUTSUT:
            kaari = True
        elif nimi in UI_KOKOELMAMETODIT and s is not None and s >= 3 and tok[s-2].arvo in ('->', '.') \
                and tok[s-3].arvo in UI_KOKOELMAT:
            kaari = on_nimike(sisalto) and lohko[i] not in tulostelohkot
        elif nimi in UI_MUOTOILU and ui_tiedosto:
            # Vain kohde/muotoiluparametri: ei ensimmäinen parametri (puskuri)
            params, _ = kutsun_parametrit(tok, s)
            onko_eka = params and params[0][0] <= i < params[0][1]
            kaari = not onko_eka and nayttaa_tekstilta(sisalto) and lohko[i] not in tulostelohkot
        elif nimi == 'sprintf' or (nimi is None and s is None):
            pass
        if not kaari and (s is None or tok[s].arvo == '{') and ui_tiedosto is not None:
            # Sijoitus: etsi lauseen '=' samalla tasolla
            k = i - 1
            while k >= 0 and tok[k].arvo not in (';', '{', '}', '=') and \
                    not (tok[k].laji == 'p' and tok[k].arvo in '()' and ymparoiva[k] != ymparoiva[i]):
                k -= 1
            if k >= 1 and tok[k].arvo == '=' and tok[k-1].arvo not in ('=', '!', '<', '>'):
                vasen = tok[k-1]
                if vasen.laji == 'ident' and vasen.arvo in UI_OMINAISUUDET:
                    kaari = on_nimike(sisalto)
                elif vasen.arvo == ']':
                    # Cells[c][r] = ...
                    m = k - 1
                    syv = 0
                    while m >= 0:
                        if tok[m].arvo == ']':
                            syv += 1
                        elif tok[m].arvo == '[':
                            syv -= 1
                            if syv == 0 and (m == 0 or tok[m-1].arvo != ']'):
                                break
                        m -= 1
                    if m >= 1 and tok[m-1].arvo == 'Cells':
                        kaari = on_nimike(sisalto)
        # == ja != ovat tokeneina kaksi merkkiä
        vertailuop = (i >= 2 and tok[i-1].arvo == '=' and tok[i-2].arvo in ('=', '!')) or \
            (loppu + 1 < n and tok[loppu].arvo in ('=', '!') and tok[loppu+1].arvo == '=')
        if kaari:
            muutokset.append((tok[i].alku, tok[loppu-1].loppu, 'TR' if leve else 'TRA', sisalto))
            kaaritut.add(sisalto)
        elif nimi in VERTAILUKUTSUT or vertailuop:
            vertailut.append((t.rivi, sisalto))
        i = loppu
    # Tekstiä, jota samassa tiedostossa verrataan, ei käännetä: se on koodi eikä pelkkä näyttöteksti
    verratut = set(v[1] for v in vertailut)
    for rivi, sisalto in vertailut:
        if sisalto in kaaritut:
            ohitetut.append((rivi, 'verrataan', sisalto))
    muutokset = [m for m in muutokset if m[3] not in verratut]
    uusi = teksti
    for a, b, f, _ in reversed(muutokset):
        uusi = uusi[:a] + f + '(' + uusi[a:b] + ')' + uusi[b:]
    return uusi, len(muutokset), ohitetut


def kaari(argv):
    """kaari [--ui|--ei-ui] [--nayta] tiedostot..."""
    ui = True
    nayta = '--nayta' in argv
    if '--ei-ui' in argv:
        ui = False
    tiedostot = [a for a in argv if not a.startswith('--')]
    yht = 0
    for polku in tiedostot:
        b = open(polku, 'rb').read()
        bom = b.startswith(b'\xef\xbb\xbf')
        teksti = b[3:].decode('latin-1') if bom else b.decode('latin-1')
        uusi, lkm, ohitetut = kaari_teksti(teksti, ui)
        for rivi, syy, s in ohitetut:
            print(f'  ohitettu {polku}:{rivi} ({syy}): {s[:60]!r}')
        if lkm:
            if nayta:
                import difflib
                for r in difflib.unified_diff(teksti.split('\n'), uusi.split('\n'), lineterm='', n=0):
                    if r.startswith(('+', '-')) and not r.startswith(('+++', '---')):
                        print(r.encode('latin-1').decode('cp1252', errors='replace')[:200])
            else:
                open(polku, 'wb').write((b'\xef\xbb\xbf' if bom else b'') + uusi.encode('latin-1'))
        print(f'{polku}: {lkm}')
        yht += lkm
    print(f'yhteensä {yht}')


KOMENNOT = {'poimi': poimi, 'paivita': paivita, 'tarkista': tarkista, 'tilasto': tilasto, 'kaari': kaari}

if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    if len(sys.argv) < 2 or sys.argv[1] not in KOMENNOT:
        print(__doc__)
        sys.exit(2)
    sys.exit(KOMENNOT[sys.argv[1]](sys.argv[2:]) or 0)
