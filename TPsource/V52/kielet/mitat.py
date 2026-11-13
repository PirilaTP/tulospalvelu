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

"""Käännettyjen tekstien mahtuminen lomakkeille (kielet.py mittaa).

Mittaa jokaisen lomakkeen (.dfm) tekstin käännöksen leveyden kontrollin omalla fontilla
ja vertaa sitä tilaan, joka tekstille on lomakkeella. Lomakkeet on suunniteltu 96 DPI:lle,
joten kaikki mitat ovat pikseleitä 96 DPI:llä.

Käytettävissä oleva tila on vähintään suomenkielisen tekstin leveys: suomenkielinen
lomake on hyväksytty sellaisenaan, joten mittausmallin epätarkkuus ei tuota vääriä
hälytyksiä tekstistä, joka ei ole suomea pidempi.

Vaatii Pillow-kirjaston (pip install pillow) ja Windowsin fontit (C:\\Windows\\Fonts).
"""

import os

TOL = 2  # px, sallittu ylitys

FONTTIHAK = os.path.join(os.environ.get('WINDIR', r'C:\Windows'), 'Fonts')
FONTTITIEDOSTOT = {
    ('arial', False): 'arial.ttf', ('arial', True): 'arialbd.ttf',
    ('tahoma', False): 'tahoma.ttf', ('tahoma', True): 'tahomabd.ttf',
    ('segoe ui', False): 'segoeui.ttf', ('segoe ui', True): 'segoeuib.ttf',
    ('verdana', False): 'verdana.ttf', ('verdana', True): 'verdanab.ttf',
    ('courier new', False): 'cour.ttf', ('courier new', True): 'courbd.ttf',
    ('times new roman', False): 'times.ttf', ('times new roman', True): 'timesbd.ttf',
}
# Ms Sans Serif ym. bittikarttafontit korvataan Tahomalla (Windowsin oma korvaus)

# Kontrollit, joiden Caption mitataan
MITATTAVAT = ('TLabel', 'TCheckBox', 'TRadioButton', 'TButton', 'TBitBtn',
              'TGroupBox', 'TRadioGroup', 'TPanel')
# Kontrollit, jotka cbKielet.cpp:n sovitaLeveys() voi leventää vapaaseen tilaan
LEVENNETTAVAT = ('TCheckBox', 'TRadioButton', 'TButton', 'TBitBtn')

LOMAKEHAK = ['cbHk', 'ViestiWin', 'cbTp']


class Mittari:
    def __init__(self):
        try:
            from PIL import ImageFont
        except ImportError:
            raise SystemExit('kielet.py mittaa vaatii Pillow-kirjaston: pip install pillow')
        self.ImageFont = ImageFont
        self.fontit = {}

    def fontti(self, nimi, px, liha):
        tied = FONTTITIEDOSTOT.get((nimi.lower(), liha)) or FONTTITIEDOSTOT[('tahoma', liha)]
        avain = (tied, px)
        if avain not in self.fontit:
            polku = os.path.join(FONTTIHAK, tied)
            if not os.path.exists(polku):
                raise SystemExit('Fonttia ei löydy: %s' % polku)
            self.fontit[avain] = self.ImageFont.truetype(polku, px)
        return self.fontit[avain]

    def leveys(self, teksti, fontti):
        # &-pikanäppäinmerkki ei näy, && näkyy yhtenä &-merkkinä
        s = teksti.replace('&&', '\0').replace('&', '').replace('\0', '&')
        return max((fontti.getlength(rivi) for rivi in s.split('\n')), default=0)


class Komponentti:
    def __init__(self, nimi, luokka, isa):
        self.nimi, self.luokka, self.isa = nimi, luokka, isa
        self.om = {}
        self.lapset = []

    def p(self, k, oletus=None):
        return self.om.get(k, oletus)

    def luku(self, k, oletus=0):
        try:
            return int(self.om.get(k, oletus))
        except (TypeError, ValueError):
            return oletus

    def geometria(self):
        return self.luku('Left'), self.luku('Top'), self.luku('Width'), self.luku('Height')

    def fontti(self):
        """(nimi, pikselikorkeus, lihavoitu) ParentFont-ketjua seuraten."""
        k = self
        nimi = px = liha = None
        while k is not None:
            if nimi is None and 'Font.Name' in k.om:
                nimi = k.om['Font.Name'].strip("'")
            if px is None and 'Font.Height' in k.om:
                px = abs(int(k.om['Font.Height']))
            if px is None and 'Font.Size' in k.om:
                px = round(abs(int(k.om['Font.Size'])) * 96 / 72)
            if liha is None and 'Font.Style' in k.om:
                liha = 'fsBold' in k.om['Font.Style']
            if k.p('ParentFont') == 'False':
                break
            k = k.isa
        return nimi or 'Tahoma', px or 11, bool(liha)

    def asiakasleveys(self):
        if self.isa is None:
            return self.luku('ClientWidth', self.luku('Width', 800))
        w = self.luku('Width', 0)
        if 'Width' not in self.om or self.p('Align') in ('alClient', 'alTop', 'alBottom'):
            w = self.isa.asiakasleveys()
        if self.luokka in ('TTabSheet', 'TPageControl'):
            return w - 8
        if self.luokka in ('TGroupBox', 'TRadioGroup'):
            return w - 4
        return w

    def sisarukset(self):
        if self.isa is None:
            return []
        return [s for s in self.isa.lapset
                if s is not self and 'Left' in s.om and s.p('Visible') != 'False']


def lue_lomake(polku, kielet):
    """Jäsentää .dfm-tiedoston komponenttipuuksi."""
    teksti = kielet.lue_dfm(polku).replace('\r\n', '\n')
    juuri = None
    pino = []
    pos = 0
    while pos < len(teksti):
        loppu = teksti.find('\n', pos)
        loppu = len(teksti) if loppu < 0 else loppu
        rivi = teksti[pos:loppu]
        m = kielet.DFM_OBJ.match(rivi)
        if m:
            k = Komponentti(m.group(2), m.group(3), pino[-1] if pino else None)
            if pino:
                pino[-1].lapset.append(k)
            else:
                juuri = k
            pino.append(k)
            pos = loppu + 1
            continue
        if rivi.strip() == 'end':
            if pino:
                pino.pop()
            pos = loppu + 1
            continue
        m = kielet.DFM_PROP.match(rivi)
        if not m or not pino:
            pos = loppu + 1
            continue
        avain = m.group(1)
        if avain in ('Caption', 'Items.Strings'):
            arvot, pos = kielet.dfm_arvo(teksti, pos + m.end())
            if arvot is not None:
                pino[-1].om[avain] = arvot if avain == 'Items.Strings' else ''.join(arvot)
            continue
        arvo = rivi[m.end():].strip()
        if arvo.startswith('[') and not arvo.endswith(']'):
            while not arvo.endswith(']') and loppu < len(teksti):
                pos = loppu + 1
                loppu = teksti.find('\n', pos)
                loppu = len(teksti) if loppu < 0 else loppu
                arvo += ' ' + teksti[pos:loppu].strip()
        pino[-1].om[avain] = arvo
        pos = loppu + 1
    return juuri


def kaikki(k):
    yield k
    for c in k.lapset:
        yield from kaikki(c)


def tilaa_oikealla(k, leveys):
    """Kuinka leveäksi k voi kasvaa osumatta isän reunaan tai oikealla olevaan sisarukseen."""
    l, t, w, h = k.geometria()
    raja = k.isa.asiakasleveys() - 4 if k.isa is not None else 10 ** 6
    for s in k.sisarukset():
        sl, st, sw, sh = s.geometria()
        if st < t + h and t < st + sh and sl > l and sl >= l + min(w, leveys) - 2:
            raja = min(raja, sl - 4)
    return raja - l


def reunukset(k):
    """Kontrollin reunusten ja kuvakkeiden viemä tila tekstin ympärillä (px)."""
    if k.luokka in ('TCheckBox', 'TRadioButton'):
        return 18
    if k.luokka == 'TButton':
        return 8
    if k.luokka == 'TBitBtn':
        return 30 if ('Glyph.Data' in k.om or 'Kind' in k.om) else 8
    if k.luokka in ('TGroupBox', 'TRadioGroup'):
        return 16
    if k.luokka == 'TPanel':
        return 6
    return 0


# Kontrollit, joille cbKielet.cpp käyttää lyhyttä muotoa (msgctxt "lyhyt"), kun pitkä ei mahdu
LYHENNETTAVAT = ('TLabel', 'TCheckBox', 'TRadioButton', 'TButton', 'TBitBtn', 'TGroupBox', 'TRadioGroup')


def mittaa_komponentti(k, kaannos, lyhyet, mittari):
    """Tuottaa ylivuodot: (ominaisuus, fi, käännös, tarve_px, tila_px, ratkaisu, lyhyt, lyhyt_px).

    ratkaisu: 'levenee' (kaannaLomake leventää kontrollin vapaaseen tilaan), 'lyhyt' (lyhyt
    muoto mahtuu) tai '' (ei mahdu: lyhennä, lisää lyhyt muoto tai muuta lomaketta)."""
    nimi, px, liha = k.fontti()
    f = mittari.fontti(nimi, px, liha)
    tulos = []

    def ratkaise(fi, tila, levenee):
        if levenee:
            return 'levenee', '', 0
        if fi in lyhyet and k.luokka in LYHENNETTAVAT:
            lyh = lyhyet[fi]
            lpx = mittari.leveys(lyh, f)
            return ('lyhyt' if lpx <= tila + TOL else ''), lyh, lpx
        return '', '', 0

    cap = k.p('Caption')
    if cap and cap in kaannos and k.luokka in MITATTAVAT:
        t = kaannos[cap]
        tarve = mittari.leveys(t, f)
        tarve_fi = mittari.leveys(cap, f)
        autosize = (k.luokka == 'TLabel' and k.p('AutoSize', 'True') != 'False'
                    and k.p('WordWrap') != 'True')
        if autosize:
            l, top, w, h = k.geometria()
            if k.p('Alignment') == 'taRightJustify':
                oikea = l + w
                raja = 0
                for s in k.sisarukset():
                    sl, st, sw, sh = s.geometria()
                    if st < top + h and top < st + sh and sl + sw <= oikea - tarve_fi + 2:
                        raja = max(raja, sl + sw + 4)
                tila = oikea - raja
            else:
                tila = tilaa_oikealla(k, tarve)
            tila = max(tila, tarve_fi)
            if tarve > tila + TOL:
                tulos.append(('Caption', cap, t, tarve, tila) + ratkaise(cap, tila, False))
        else:
            reuna = reunukset(k)
            tila = k.luku('Width') - reuna
            if k.p('WordWrap') == 'True':
                rivikorkeus = px + (2 if k.luokka == 'TLabel' else 6)
                tila *= max(1, k.luku('Height') // rivikorkeus)
            tila = max(tila, tarve_fi)
            if tarve > tila + TOL:
                lev = (k.luokka in LEVENNETTAVAT
                       and k.p('WordWrap') != 'True'
                       and k.p('Align', 'alNone') == 'alNone'
                       and 'akRight' not in k.p('Anchors', '')
                       and tilaa_oikealla(k, tarve + reuna) >= tarve + reuna)
                tulos.append(('Caption', cap, t, tarve, tila) + ratkaise(cap, tila, lev))
    if k.luokka == 'TRadioGroup':
        sarakkeita = max(1, k.luku('Columns', 1))
        tila0 = (k.luku('Width') - 10) / sarakkeita - 22
        for kohta in k.p('Items.Strings') or []:
            if kohta in kaannos:
                tarve = mittari.leveys(kaannos[kohta], f)
                tila = max(tila0, mittari.leveys(kohta, f))
                if tarve > tila + TOL:
                    tulos.append(('Items', kohta, kaannos[kohta], tarve, tila)
                                 + ratkaise(kohta, tila, False))
    return tulos


def kaannostaulu(kielet, kieli, konteksti=None):
    d = {}
    for m in kielet.lue_po(os.path.join(kielet.KIELIHAK, kieli + '.po')):
        if m.msgstr and m.ktx == konteksti and 'fuzzy' not in m.liput and not m.vanhentunut:
            d[m.msgid] = m.msgstr
    return d


def lomakkeet(kielet):
    for d in LOMAKEHAK:
        hak = os.path.join(kielet.V52, d)
        for nimi in sorted(os.listdir(hak)):
            if nimi.lower().endswith('.dfm'):
                yield os.path.join(hak, nimi)


def ylivuodot(kielet, kielilista):
    """Palauttaa listan ylivuotoja sanakirjoina."""
    mittari = Mittari()
    taulut = {k: kaannostaulu(kielet, k) for k in kielilista}
    lyhyet = {k: kaannostaulu(kielet, k, kielet.LYHYT) for k in kielilista}
    rivit = []
    for polku in lomakkeet(kielet):
        juuri = lue_lomake(polku, kielet)
        if juuri is None:
            continue
        rel = os.path.relpath(polku, kielet.V52).replace('\\', '/')
        for k in kaikki(juuri):
            if k.isa is None:
                continue
            for kieli in kielilista:
                for om, fi, t, tarve, tila, ratk, lyh, lpx in mittaa_komponentti(
                        k, taulut[kieli], lyhyet[kieli], mittari):
                    rivit.append({
                        'kieli': kieli, 'lomake': rel, 'kontrolli': k.nimi, 'luokka': k.luokka,
                        'ominaisuus': om, 'tarve': round(tarve), 'tila': int(tila),
                        'ratkaisu': ratk, 'fi': fi, 'kaannos': t,
                        'lyhyt': lyh, 'lyhyt_px': round(lpx),
                        'fontti': '%s %d%s' % (k.fontti()[0], k.fontti()[1],
                                               ' bold' if k.fontti()[2] else ''),
                    })
    return rivit
