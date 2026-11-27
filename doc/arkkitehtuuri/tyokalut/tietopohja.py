#!/usr/bin/env python3
"""Knowledge base (phase K) tool: defect cards in doc/tietopohja/vikakortit/.

Usage (from the repository root):
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . runko  <id> ...   new card from the template
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . lohkot [<id> ...] rewrite generated blocks
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kuittaa <id> ...  record the register state
                                                                           the text was checked against
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . tarkista          check all cards (exit 1 on error)
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kasikirja         rebuild the findings block of every
                                                                           handbook chapter from kasikirja/toiminnot.csv
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . tuo <csv> ...     import writer fragments (impacts,
                                                                           checked programs) into the registers
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kooste            generated reading document of all
                                                                           cards (doc/tietopohja/laatu/vikakorttipilotti.md)

A card has three generated blocks (summary, impacts, technical data) between
<!-- generoitu:NAME alku --> and <!-- generoitu:NAME loppu --> markers. They are
rebuilt from the registers and never edited by hand. The rest is authored text.
The authored text records the hash of the register rows it was written against
(<!-- rekisteritila: HASH -->); when a row changes, 'tarkista' reports the card as
stale until a writer has re-read it and run 'kuittaa'.

Sources: rekisteri/havainnot.md, riskiarvio.csv, tekninen-velka.csv, vaikutukset.csv,
vaikutusulottuvuudet.csv, korjauskokonaisuudet.csv, seuranta.md (Q),
omistajan-paatokset.md (OP), auditoinnin-yhteenveto.md section 7 (JS names).
Read-only except for the card files. Output is ASCII.
"""
import csv
import hashlib
import io
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.dont_write_bytecode = True  # keep the repository free of __pycache__
import synteesi  # noqa: E402

H_HEADINGS = [
    "Lyhyesti",
    "Mitä ohjelma yrittää tehdä",
    "Mikä menee vikaan",
    "Missä tilanteessa vika voi ilmetä",
    "Mitä käyttäjä voi huomata",
    "Vaikutukset kilpailuun",
    "Mitä kilpailun aikana voi tapahtua",
    "Palautuminen ja uudelleenkäynnistys",
    "Mitä käyttäjä voi tehdä riskin pienentämiseksi",
    "Tekninen juurisyy",
    "Mitä on todennettu",
    "Mitä ei vielä tiedetä",
    "Miten vika on tarkoitus korjata",
    "Tekniset tiedot ja lähteet",
]
REFUTED_HEADINGS = [
    "Lyhyesti",
    "Mitä alun perin väitettiin",
    "Miksi väite kumottiin",
    "Mitä tämä tarkoittaa käyttäjälle",
    "Mitä ei tiedetä",
    "Tekniset tiedot ja lähteet",
]
T_HEADINGS = [
    "Mistä on kyse",
    "Mitä haittaa siitä on nykyisin",
    "Mihin vikoihin se liittyy",
    "Mitä hyötyä sen vähentämisestä olisi",
    "Miten se on suunniteltu käsiteltäväksi",
    "Mitä ei tiedetä",
    "Tekniset tiedot ja lähteet",
]
RECOVERY_LABELS = ["Virhetilan poistaminen", "Normaalin toiminnan palauttaminen", "Tietojen palauttaminen",
                   "Uusiutuminen", "Muut työasemat"]
ADVICE_LABELS = ["Todennettu toimintaohje", "Perusteltu, testaamaton suositus",
                 "Ei tunnettua turvallista toimintaohjetta"]
GENERATED_SECTIONS = {"Vaikutukset kilpailuun", "Tekniset tiedot ja lähteet"}

VALUE_LABEL = {
    "vahvistettu": "**Vahvistettu**",
    "mahdollinen": "Mahdollinen",
    "ei_perusteltu": "Ei vaikutusta (perusteltu)",
    "ei_tiedossa": "Ei tiedossa",
    "ei_sovellu": "Ei sovellu",
    None: "_Ei arvioitu_",
}
TODENNUS = {
    "toimitettu_ohjelma": "vahvistettu ajamalla toimitettua ohjelmaa",
    "koe_tai_testikehys": "toistettu eristetyllä kokeella tai testikehyksessä",
    "riippumaton": "luettu koodista ja riippumattomasti tarkastettu",
    "riippumaton_osittain": "luettu koodista; riippumaton tarkastus vahvisti osittain",
    "tarkastajan_loyto": "riippumattoman tarkastajan oma löytö",
    "ei": "yhden analyysin löydös, ei riippumattomasti tarkastettu",
    "ei_ratkea_staattisesti": "ei ratkaistavissa koodia lukemalla",
    "-": "kumottu",
}
SAAVUTETTAVUUS = {
    "verkko_oletus": "verkosta oletusasetuksin",
    "verkko_asetus": "verkosta, kun tietty asetus on käytössä",
    "laite": "ajanotto- tai leimauslaitteen yhteydestä tai sen lähettämästä tiedosta",
    "normaalikaytto": "tavallisessa käytössä",
    "asetus": "tietyllä asetuksella tai kokoonpanolla",
    "operaattori": "käyttäjän toimenpiteestä",
    "paikallinen_tiedosto": "virheellisestä tai käsin muokatusta paikallisesta tiedostosta",
    "ei_toimitetussa": "ei toimitetussa ohjelmassa",
    "-": "-",
}
FREQ_WORDS = re.compile(r"\b(usein|harvoin|todennäköisesti|epätodennäköi\w*|yleensä|tavallisesti|"
                        r"tyypillisesti|joskus|harvinai\w*|yleisi?n?ä?\b|lähes aina|melko varmasti)", re.I)
LINE_REF = re.compile(r"(\.(?:cpp|h|c|hpp)\s*:\s*\d)|(`:\d)|(\brivi(?:llä|ltä|lle|t|n|ä)?\s+\d)", re.I)
PRIO = re.compile(r"\bP[1-5]\b")
ID_ANY = re.compile(r"\b(H-\d{3}|T-\d{3}|KK-C\d+-\d+|JS-\d+|OP-\d+|Q\d+)\b")


def rcsv(path):
    with io.open(path, encoding="utf-8", newline="") as fh:
        return list(csv.DictReader(fh))


class Data:
    def __init__(self, root):
        self.root = root
        self.doc = os.path.join(root, "doc", "arkkitehtuuri")
        self.cards = os.path.join(root, "doc", "tietopohja", "vikakortit")
        reg = os.path.join(self.doc, "rekisteri")
        hrows, trows = synteesi.parse_register(self.doc)
        self.reg = {r["id"]: r for r in hrows + trows}
        self.risk = {r["id"]: r for r in rcsv(os.path.join(reg, "riskiarvio.csv"))}
        self.tdebt = {r["id"]: r for r in rcsv(os.path.join(reg, "tekninen-velka.csv"))}
        self.dims = rcsv(os.path.join(reg, "vaikutusulottuvuudet.csv"))
        self.imp = {}
        for r in rcsv(os.path.join(reg, "vaikutukset.csv")):
            self.imp.setdefault(r["id"], {})[r["ulottuvuus"]] = r
        self.packs = {p["kk"]: p for p in rcsv(os.path.join(reg, "korjauskokonaisuudet.csv"))}
        self.primary = {}
        for p in self.packs.values():
            for i in (p["havainnot"] + " " + p["tekninen_velka"]).split():
                self.primary[i] = p["kk"]
        self.qs = synteesi.parse_questions(self.doc)
        summ = synteesi.read(os.path.join(self.doc, "auditoinnin-yhteenveto.md"))
        self.js = {m.group(1): m.group(2).strip() for m in
                   re.finditer(r"\*\*(JS-\d+)\*\* ([^/|]+?) /", summ)}
        op = synteesi.read(os.path.join(self.doc, "omistajan-paatokset.md"))
        self.op = {}
        for m in re.finditer(r"^## (OP-\d+) ([^\n]*)\n\n- \*\*Tila / status\.\*\* ([^\n]*)\n(.*?)(?=^## |\Z)",
                             op, re.S | re.M):
            self.op[m.group(1)] = {"nimi": m.group(2), "tila": m.group(3).split(" / ")[0],
                                   "teksti": m.group(4)}

    # ------------------------------------------------------------ helpers ---
    def kind(self, ident):
        if ident.startswith("T-"):
            return "T"
        return "K" if self.risk.get(ident, {}).get("tila") == "kumottu" else "H"

    def state_row(self, ident):
        return self.risk.get(ident) or self.tdebt.get(ident) or {}

    def related(self, ident):
        line = self.reg[ident]["_line"]
        ids = [i for i in dict.fromkeys(synteesi.expand_ids(line)) if i != ident and i in self.reg]
        qs = set(int(n) for n in synteesi.expand_qs(line))
        for n, q in self.qs.items():
            if any(ident in synteesi.expand_ids(t) for t in q["tekstit"]):
                qs.add(n)
        ops = [o for o, d in self.op.items() if re.search(r"\b%s\b" % re.escape(ident), d["teksti"])]
        kk = self.primary.get(ident)
        via = []
        if kk:
            via = [o for o in self.packs[kk]["omistajan_paatokset"].split() if o in self.op and o not in ops]
        key = lambda s: int(s[3:])
        return ids, sorted(qs), sorted(set(ops), key=key), sorted(set(via), key=key)

    def reg_hash(self, ident):
        parts = [self.reg[ident]["_line"]]
        sr = self.state_row(ident)
        parts.append(repr(sorted(sr.items())))
        kk = self.primary.get(ident)
        if kk:
            parts.append(repr(sorted(self.packs[kk].items())))
        for d, r in sorted(self.imp.get(ident, {}).items()):
            parts.append(repr(sorted(r.items())))
        return hashlib.sha256("\n".join(parts).encode("utf-8")).hexdigest()[:12]

    def js_text(self, js):
        if js == "yksittainen":
            return "yksittäinen vika, ei yhteistä juurisyytä"
        if js in self.js:
            return "%s %s" % (js, self.js[js])
        return js

    # ----------------------------------------------------- generated blocks ---
    def block_yhteenveto(self, ident):
        sr = self.state_row(ident)
        rk = self.risk.get(ident, {})
        kk = self.primary.get(ident, "-")
        L = ["> **%s** · tila: **%s**" % (ident, sr.get("tila", "?"))]
        if self.kind(ident) == "H":
            L[0] += " · prioriteetti: **%s** · tekninen vakavuus: %s" % (rk["prioriteetti"], rk["tekninen_vakavuus"])
            L.append("> Ohjelmointivirheen todennus: %s. Riskiarvion saavutettavuus: %s (tarkemmin luvussa 4)." % (
                TODENNUS.get(rk["todennus"], rk["todennus"]), SAAVUTETTAVUUS.get(rk["saavutettavuus"], rk["saavutettavuus"])))
        else:
            L[0] += " · vakavuus: %s" % self.reg[ident]["vakavuus"]
        progs = sr.get("ohjelmat", "")
        basis = {"tiedosto": "yläraja: ohjelmat, joihin viitatut lähdetiedostot käännetään",
                 "tarkistettu": "tarkistettu kortin kirjoituksen yhteydessä",
                 "ei_tiedossa": "ei tiedossa"}.get(sr.get("ohjelmat_peruste"), "")
        L.append("> Ohjelmat: %s (%s)." % (progs.replace(" ", ", "), basis))
        if kk != "-":
            p = self.packs.get(kk, {})
            L.append("> Korjauskokonaisuus: %s %s (aalto %s)." % (kk, p.get("nimi", ""), p.get("aalto", "?")))
        return L

    def block_vaikutukset(self, ident):
        L = ["| Kysymys | Arvio | Peruste |", "|---|---|---|"]
        mine = self.imp.get(ident, {})
        groups = {"vaikutus": "Vaikutus", "ilmeneminen": "Ilmeneminen", "hallinta": "Hallinta"}
        for d in self.dims:
            r = mine.get(d["ulottuvuus"])
            label = VALUE_LABEL[r["arvo"] if r else None]
            reason = r["peruste"].replace("|", "/") if r else ""
            L.append("| %s: %s | %s | %s |" % (groups[d["ryhma"]], d["kysymys"], label, reason))
        L.append("")
        L.append("Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa "
                 "ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta "
                 "(perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; "
                 "_Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. "
                 "Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.")
        return L

    def block_tekniset(self, ident):
        r = self.reg[ident]
        c = r["_cells"]
        sr = self.state_row(ident)
        rk = self.risk.get(ident, {})
        ids, qs, ops, via = self.related(ident)
        kk = self.primary.get(ident, "-")
        p = self.packs.get(kk, {})
        rows = [("Tunnus", ident), ("Rekisterin kuvaus", c[1]), ("Erä", r["era"]),
                ("Tila", sr.get("tila", "?") + ((" (korjaus: %s)" % sr["korjaus"]) if sr.get("korjaus") else ""))]
        if ident.startswith("H-"):
            rows += [("Vakavuus (rekisteri)", c[3]), ("Varmuus (rekisteri)", c[4]), ("Todennus (rekisteri)", c[5])]
            if self.kind(ident) == "H":
                prio = rk["prioriteetti"]
                if rk.get("prioriteetti_aiempi") and rk["prioriteetti_aiempi"] != prio:
                    prio += " (aiempi %s)" % rk["prioriteetti_aiempi"]
                rows += [("Prioriteetti", prio),
                         ("Riskiarvio", "vakavuus %s; saavutettavuus %s; todennus %s; tulokset %s; ajanotto %s; "
                          "palautettavuus %s; tietoturva %s" % tuple(rk[k] for k in (
                              "tekninen_vakavuus", "saavutettavuus", "todennus", "tulosten_oikeellisuus",
                              "ajanoton_jatkuvuus", "palautettavuus", "tietoturva"))),
                         ("Riskiarvion peruste", rk["peruste"] + ((" " + rk["huomio"]) if rk.get("huomio") else ""))]
            rows += [("Lähde (raportti)", c[7]), ("Näytön laji", c[8])]
        else:
            rows += [("Vakavuus (rekisteri)", c[3]), ("Varmuus (rekisteri)", c[4]), ("Näytön laji", c[6])]
        js = sr.get("juurisyy", "-")
        muut = " ".join(self.js_text(j) for j in sr.get("juurisyy_muut", "").split())
        rows.append(("Juurisyy", self.js_text(js) + (("; pahentavat: " + muut) if muut else "")))
        rows.append(("Ohjelmat", "%s (peruste: %s)" % (sr.get("ohjelmat", ""), sr.get("ohjelmat_peruste", ""))))
        if kk != "-":
            rows.append(("Korjauskokonaisuus", "%s %s; vaihe %s, aalto %s; haara `%s`" % (
                kk, p.get("nimi", ""), p.get("vaihe", ""), p.get("aalto", ""), p.get("haara", ""))))
        if rk.get("kk_muut"):
            rows.append(("Muut kokonaisuudet", rk["kk_muut"]))
        fmt = lambda o: "%s %s (%s)" % (o, self.op[o]["nimi"], self.op[o]["tila"])
        rows.append(("Omistajan päätökset, joissa havainto mainitaan", ", ".join(fmt(o) for o in ops) or "-"))
        if via:
            rows.append(("Kokonaisuuden %s omistajan päätökset" % kk, ", ".join(fmt(o) for o in via)))
        rows.append(("Kysymykset", ", ".join("Q%d (%s)" % (n, self.qs[n]["tila"] or "?") for n in qs if n in self.qs) or "-"))
        rows.append(("Liittyvät havainnot", " ".join(ids) or "-"))
        rows.append(("Lähdeviitteet (revisio %s, polut `TPsource/V52/`-hakemistosta)" % r["rev"], c[2]))
        L = ["| Kenttä | Arvo |", "|---|---|"]
        L += ["| %s | %s |" % (k, str(v).replace("\n", " ")) for k, v in rows]
        L.append("")
        L.append("Rekisteririvien tiiviste nyt: `%s`." % self.reg_hash(ident))
        return L

    def blocks(self, ident):
        b = {"yhteenveto": self.block_yhteenveto(ident), "tekniset": self.block_tekniset(ident)}
        if self.kind(ident) == "H":
            b["vaikutukset"] = self.block_vaikutukset(ident)
        return b

    def path(self, ident):
        return os.path.join(self.cards, ident[0], ident + ".md")


def wrap(name, lines):
    return ["<!-- generoitu:%s alku (tietopohja.py lohkot; älä muokkaa käsin) -->" % name] + lines + \
           ["<!-- generoitu:%s loppu -->" % name]


BLOCK_RE = re.compile(r"<!-- generoitu:(\w+) alku[^>]*-->\n.*?<!-- generoitu:\1 loppu -->", re.S)


def fill(text, blocks):
    def rep(m):
        return "\n".join(wrap(m.group(1), blocks.get(m.group(1), ["(tuntematon lohko)"])))
    return BLOCK_RE.sub(rep, text)


def read_text(path):
    return open(path, "rb").read().decode("utf-8")


def write_text(path, text):
    with open(path, "wb") as fh:
        fh.write(text.encode("utf-8"))


def template_for(data, ident):
    name = {"H": "_malli-H.md", "K": "_malli-H-kumottu.md", "T": "_malli-T.md"}[data.kind(ident)]
    return read_text(os.path.join(data.cards, name))


def cmd_runko(data, ids):
    for i in ids:
        p = data.path(i)
        if os.path.exists(p):
            print("exists, not overwritten:", p)
            continue
        t = template_for(data, i).replace("{ID}", i)
        t = re.sub(r"<!-- malli:.*?-->\n", "", t, flags=re.S)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        write_text(p, fill(t, data.blocks(i)))
        print("created", os.path.relpath(p, data.root))


def card_ids(data):
    out = []
    for k in ("H", "T"):
        d = os.path.join(data.cards, k)
        if os.path.isdir(d):
            out += sorted(f[:-3] for f in os.listdir(d) if re.match(r"[HT]-\d{3}\.md$", f))
    return out


def cmd_lohkot(data, ids):
    for i in ids or card_ids(data):
        p = data.path(i)
        old = read_text(p)
        new = fill(old, data.blocks(i))
        if new != old:
            write_text(p, new)
            print("updated", i)


HASH_RE = re.compile(r"<!-- rekisteritila: ([0-9a-f]{12}|uusi) -->")


def cmd_kuittaa(data, ids):
    for i in ids:
        p = data.path(i)
        t = read_text(p)
        if not HASH_RE.search(t):
            print("ERROR %s: no rekisteritila line" % i)
            continue
        write_text(p, HASH_RE.sub("<!-- rekisteritila: %s -->" % data.reg_hash(i), t, count=1))
        print("acknowledged", i, data.reg_hash(i))


def authored(text):
    t = BLOCK_RE.sub("", text)
    return re.sub(r"<!--.*?-->", "", t, flags=re.S)


def cmd_tarkista(data):
    errors, warnings = [], []
    ids = card_ids(data)
    known = set(data.reg) | set(data.packs) | set(data.op) | set(data.js) | {"Q%d" % n for n in data.qs}
    for i in ids:
        if i not in data.reg:
            errors.append("%s: card for an unknown id" % i)
            continue
        text = read_text(data.path(i))
        heads = {"H": H_HEADINGS, "K": REFUTED_HEADINGS, "T": T_HEADINGS}[data.kind(i)]
        if not text.startswith("# %s " % i):
            errors.append("%s: first line must be '# %s - <title>'" % (i, i))
        if fill(text, data.blocks(i)) != text:
            errors.append("%s: generated blocks out of date (run: tietopohja.py . lohkot %s)" % (i, i))
        if set(m.group(1) for m in BLOCK_RE.finditer(text)) != set(data.blocks(i)):
            errors.append("%s: generated blocks missing or extra" % i)
        m = HASH_RE.search(text)
        if not m:
            errors.append("%s: no rekisteritila line" % i)
        elif m.group(1) != data.reg_hash(i):
            errors.append("%s: register rows changed since the text was checked (%s -> %s); re-read the card, "
                          "then run: tietopohja.py . kuittaa %s" % (i, m.group(1), data.reg_hash(i), i))
        found = re.findall(r"^## (?:\d+\. )?(.+?)\s*$", text, re.M)
        if found != heads:
            errors.append("%s: headings differ from the template: %s" % (i, [h for h in heads if h not in found] or
                                                                          "order"))
        body = authored(text)
        sections = re.split(r"^## .*$", body, flags=re.M)[1:]
        tmpl = re.split(r"^## .*$", authored(template_for(data, i)), flags=re.M)[1:]
        for h, s, ts in zip(found, sections, tmpl + [""] * len(sections)):
            if h not in GENERATED_SECTIONS and len(s.strip()) < 20:
                errors.append("%s: section '%s' is empty" % (i, h))
            elif h not in GENERATED_SECTIONS and ts.strip() and ts.strip() in s:
                errors.append("%s: section '%s' still has the template text" % (i, h))
        if data.kind(i) == "H":
            sec = dict(zip(found, sections))
            s8 = sec.get("Palautuminen ja uudelleenkäynnistys", "")
            for lab in RECOVERY_LABELS:
                if "**%s:**" % lab not in s8:
                    errors.append("%s: section 8 lacks '%s'" % (i, lab))
            s9 = sec.get("Mitä käyttäjä voi tehdä riskin pienentämiseksi", "")
            if not any(re.search(r"\[%s\.?\]" % r"\s+".join(map(re.escape, lab.split())), s9) for lab in ADVICE_LABELS):
                errors.append("%s: section 9 has no advice level label" % i)
        if "<selkokielinen otsikko" in text or "<mitä alun perin" in text:
            errors.append("%s: title not written" % i)
        for n, line in enumerate(body.splitlines(), 1):
            if LINE_REF.search(line):
                errors.append("%s: line reference in authored text: %s" % (i, line.strip()[:90]))
            if PRIO.search(line):
                errors.append("%s: priority in authored text (only in the generated block): %s" % (i, line.strip()[:90]))
            freq_ok = bool(re.search(r"(ei kerro|ei tiedetä|ei tiedossa|ei ole tietoa|ei arvioi)[^.]*\bkuinka", line, re.I)
                           or re.match(r"\s*-?\s*kuinka (usein|harvoin)", line, re.I))
            for w in ([] if freq_ok else FREQ_WORDS.findall(line)):
                warnings.append("%s: frequency word '%s' - needs a source: %s" % (i, w, line.strip()[:80]))
            for x in ID_ANY.findall(line):
                if x not in known:
                    errors.append("%s: unknown identifier %s in authored text" % (i, x))
        sr = data.state_row(i)
        own = {sr.get("juurisyy")} | set(sr.get("juurisyy_muut", "").split())
        for x in set(re.findall(r"\bJS-\d+\b", body)):
            if x not in own:
                warnings.append("%s: mentions %s, which is not its root cause in the register" % (i, x))
    check_handbook(data, errors, warnings)
    kp = os.path.join(data.root, KOOSTE_PATH)
    if os.path.exists(kp):
        cur = read_text(kp)
        norm = lambda t: re.sub(r"`[0-9a-f]{7,12}`|[0-9a-f]{12}", "#", t)
        if norm(cur) != norm(build_kooste(data)):
            errors.append("%s is out of date (run: tietopohja.py . kooste)" % KOOSTE_PATH.replace(os.sep, "/"))
    print("cards: %d (H %d, T %d)" % (len(ids), sum(1 for i in ids if i[0] == "H"), sum(1 for i in ids if i[0] == "T")))
    for w in warnings:
        print("WARNING", w.encode("ascii", "replace").decode())
    for e in errors:
        print("ERROR", e.encode("ascii", "replace").decode())
    print("OK" if not errors else "%d errors" % len(errors))
    return 1 if errors else 0


KOOSTE_HEAD = """# Vikakorttipilotti: lukudokumentti

> **Generoitu tarkastusaineisto - älä muokkaa käsin.** Tuotettu komennolla
> `python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kooste`. Sisältö on korttien
> (`doc/tietopohja/vikakortit/`) sellaisenaan koottu kopio; korjaukset tehdään kortteihin tai
> rekistereihin, minkä jälkeen dokumentti tuotetaan uudelleen. `tietopohja.py tarkista` ilmoittaa,
> jos dokumentti on vanhentunut. Tämä ei ole vikarekisteri: ensisijainen lähde on
> `doc/arkkitehtuuri/rekisteri/`.
>
> **Luottamuksellinen:** kuvaa korjaamattomia vikoja, myös verkosta laukeavia. Ei julkiseen
> repositorioon ilman omistajan hyväksyntää (OP-1).

"""

KOOSTE_LEGEND = """## Lukuohje

### Tekninen vakavuus ja korjausprioriteetti

Kortin yhteenvedossa ja teknisissä tiedoissa on kaksi eri asiaa:

- **Tekninen vakavuus** kertoo, kuinka paha seuraus on, jos vika laukeaa: korkea,
  keskitaso-korkea, keskitaso, matala-keskitaso tai matala. Erän alkuperäinen vakavuus
  ("Vakavuus (rekisteri)") voi poiketa vaiheen C-13 yhtenäisestä arviosta ("Riskiarvio",
  `riskiarvio.csv`); tällöin jälkimmäinen on voimassa ja ero on perusteltu.
- **Prioriteetti P1-P5** on korjausjärjestyksen arvio, joka yhdistää vakavuuden ja sen, kuka tai
  mikä vian voi laukaista (`auditoinnin-yhteenveto.md` luku 9.2):
  - **P1** verkossa oleva kolmas osapuoli voi laukaista oletusasetuksin, ja seuraus on muistin tai
    tiedoston ylikirjoitus, tiedoston korvaus tai tulosten hiljainen vääristyminen;
  - **P2** verkosta oletusasetuksin mutta lievempi seuraus, verkosta asetuksella tai laitteelta,
    tai tavallisessa käytössä toistuva pysähtyminen, jumiutuminen, kaatuminen tai tulosten hiljainen
    virhe;
  - **P3** toimitetussa ohjelmassa, mutta kapeampi seuraus tai harvinaisempi edellytys;
  - **P4** vaatii virheellisen paikallisen tiedoston tai käyttäjän virheen, tai matala vakavuus;
  - **P5** kosmeettinen, kuollut tai toimittamaton koodi, kokoomarivit.

Vakava vika voi olla matalalla prioriteetilla, jos sen laukaiseminen vaatii harvinaisen
tilanteen, ja päinvastoin.

### Todentamistasot

"Ohjelmointivirheen todennus" kertoo, miten **itse virhe** on osoitettu. Se ei kerro, onko
käytännön seuraus osoitettu (siihen on vaikutustaulukko).

| Taso | Merkitys |
|---|---|
{todennus}

Varmuusmerkinnät tekstissä: [todennettu] = osoitettu lähteestä tai koodista; [päätelty] =
johdettu, ei osoitettu; [tuntematon] / ei tiedossa = aineisto ei ratkaise.

### Käytännön vaikutusten luokat

Luvun 6 taulukko tulee rekisteristä `vaikutukset.csv`. Jokaiselle 16 kysymykselle on yksi arvo:

| Arvo | Merkitys |
|---|---|
| **Vahvistettu** | Juuri tämä seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla. Koodin lukeminen ei riitä. |
| Mahdollinen | Seuraus johtuu koodista, mutta sitä ei ole osoitettu; tai se riippuu perusteessa nimetyistä olosuhteista. |
| Ei vaikutusta (perusteltu) | Aineisto osoittaa, ettei seurausta synny. |
| Ei tiedossa | Aineisto ei ratkaise kysymystä. **Tiedon puute ei tarkoita, ettei vaikutusta ole.** |
| Ei sovellu | Kysymys ei koske tätä vikaa. |
| _Ei arvioitu_ | Riviä ei ole vielä arvioitu. |

### Auditoinnin kattavuusrajoite

Auditointi ei kata koko lähdekoodia: noin 36 % riveistä on vain inventoitu, yhtäkään
tuotantotiedostoa ei ole katselmoitu kokonaan, ja havaintojen määrä on alaraja
(`auditoinnin-yhteenveto.md` luvut 3, 4 ja 10). Vain yksi vika (H-041) on vahvistettu ajamalla
toimitettua ohjelmaa. Kortin puuttuminen ei tarkoita, että toiminto on virheetön.

### Lähderevisio

{revisio}

"""


def git_out(root, *args):
    import subprocess
    p = subprocess.run(["git", "-C", root] + list(args), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    return p.stdout.decode("utf-8", "replace").strip() if p.returncode == 0 else "?"


KOOSTE_PATH = os.path.join("doc", "tietopohja", "laatu", "vikakorttipilotti.md")


def build_kooste(data):
    ids = card_ids(data)
    revs = sorted({data.reg[i]["rev"] for i in ids if i in data.reg})
    reg_files = ["havainnot.md", "riskiarvio.csv", "tekninen-velka.csv", "vaikutukset.csv",
                 "korjauskokonaisuudet.csv"]
    hashes = []
    for f in reg_files:
        b = open(os.path.join(data.doc, "rekisteri", f), "rb").read()
        hashes.append("`%s` %s" % (f, hashlib.sha256(b).hexdigest()[:12]))
    revisio = "\n".join([
        "- Korttien lähdeviitteet (rivinumerot) on kirjattu revisioihin: %s. Tuotantokoodi "
        "`TPsource/` ei ole muuttunut revision `f7fe553` jälkeen (`doc/arkkitehtuuri/CHECKPOINT.md`)."
        % ", ".join("`%s`" % r for r in revs),
        "- Kortit ja rekisterit luettiin työpuusta, jonka viimeisin commit oli `%s`."
        % git_out(data.root, "rev-parse", "--short", "HEAD"),
        "- Rekisteritiedostojen tiivisteet tuottohetkellä (SHA-256, 12 merkkiä): " + "; ".join(hashes) + ".",
        "- Jokaisen kortin teksti on tarkistettu sen omien rekisteririvien tilaa vastaan; tiiviste on "
        "kortin teknisissä tiedoissa (\"Rekisteririvien tiiviste nyt\")."])
    tod = "\n".join("| %s | %s |" % (k, v) for k, v in TODENNUS.items() if k != "-")
    out = [KOOSTE_HEAD, "## Sisällys\n", "| Kortti | Otsikko | Tila | Prioriteetti | Tekninen vakavuus | Ohjelmointivirheen todennus |",
           "|---|---|---|---|---|---|"]
    bodies = []
    for i in ids:
        text = read_text(data.path(i))
        title = text.splitlines()[0][2:].strip()
        sr = data.state_row(i)
        rk = data.risk.get(i, {})
        anchor = i.lower()
        out.append("| [%s](#%s) | %s | %s | %s | %s | %s |" % (
            i, anchor, title.split(" – ", 1)[-1].replace("|", "/"), sr.get("tila", "?"),
            rk.get("prioriteetti", "-") if i.startswith("H") else "-",
            rk.get("tekninen_vakavuus", data.reg[i]["vakavuus"]) if i.startswith("H") else data.reg[i]["vakavuus"],
            TODENNUS.get(rk.get("todennus", ""), "-") if i.startswith("H") else "-"))
        body = re.sub(r"<!--.*?-->\n?", "", text, flags=re.S)
        body = re.sub(r"^(#+) ", lambda m: "#" + m.group(1) + " ", body, flags=re.M)
        body = body.replace("](../../", "](../")
        body = re.sub(r"\n{3,}", "\n\n", body).strip() + "\n"
        bodies.append('\n---\n\n<a id="%s"></a>\n\n%s' % (anchor, body))
    out.append("")
    out.append(KOOSTE_LEGEND.format(todennus=tod, revisio=revisio))
    out.append("## Kortit\n")
    out += bodies
    return "\n".join(out)


def cmd_kooste(data):
    p = os.path.join(data.root, KOOSTE_PATH)
    write_text(p, build_kooste(data))
    print("written", KOOSTE_PATH)


CHAPTER_HEADINGS = ["Mitä tapahtuu", "Erot ohjelmien välillä", "Asetukset ja käännösvalitsimet",
                    "Vaihtoehtoiset polut ja virhetilanteet", "Tunnetut viat tässä toiminnossa",
                    "Mihin tieto perustuu", "Mitä ei ole selvitetty"]


def handbook_dir(data):
    return os.path.join(data.root, "doc", "tietopohja", "kasikirja")


def chapters(data):
    d = handbook_dir(data)
    if not os.path.isdir(d):
        return []
    return sorted(f[:-3] for f in os.listdir(d) if re.match(r"\d{2}-[a-z0-9-]+\.md$", f))


def functions(data):
    p = os.path.join(handbook_dir(data), "toiminnot.csv")
    return rcsv(p) if os.path.exists(p) else []


def viat_block(data, chapter):
    rows = [r for r in functions(data) if r["luku"] == chapter]
    if not rows:
        return ["Tähän lukuun ei ole kytketty toimintoja (`kasikirja/toiminnot.csv`)."]
    L = ["| Toiminto | Havainto | Tila | Prioriteetti | Kuvaus |", "|---|---|---|---|---|"]
    for r in rows:
        for i in r["havainnot"].split():
            if i not in data.reg:
                continue
            sr = data.state_row(i)
            title = data.reg[i]["otsikko"].replace("|", "/")
            card = data.path(i)
            if os.path.exists(card):
                t = read_text(card).split("\n", 1)[0]
                if "<selkokielinen" not in t:
                    title = t[2:].split(" – ", 1)[-1].replace("|", "/")
            L.append("| %s | [%s](../vikakortit/%s/%s.md) | %s | %s | %s |" % (
                r["toiminto"], i, i[0], i, sr.get("tila", ""), data.risk.get(i, {}).get("prioriteetti", "-"), title))
    L.append("")
    L.append("Generoitu tiedostosta `kasikirja/toiminnot.csv` ja rekistereistä (tila, prioriteetti); älä muokkaa käsin.")
    return L


def cmd_kasikirja(data):
    for ch in chapters(data):
        p = os.path.join(handbook_dir(data), ch + ".md")
        old = read_text(p)
        new = BLOCK_RE.sub(lambda m: "\n".join(wrap(m.group(1), viat_block(data, ch))) if m.group(1) == "viat" else m.group(0), old)
        if new != old:
            write_text(p, new)
            print("updated", ch)


def check_handbook(data, errors, warnings):
    chs = chapters(data)
    if not chs:
        return
    fn = functions(data)
    for r in fn:
        if r["luku"] not in chs:
            errors.append("toiminnot.csv: unknown chapter %r (%s)" % (r["luku"], r["toiminto"]))
        for i in r["havainnot"].split():
            if i not in data.reg:
                errors.append("toiminnot.csv: unknown id %s (%s)" % (i, r["toiminto"]))
        for p in r.get("ohjelmat", "").split():
            if p not in ("HkKisaWin", "HkMaali", "ViestiWin", "ViestiMaali", "ViestiLuenta"):
                errors.append("toiminnot.csv: bad program %r (%s)" % (p, r["toiminto"]))
    for ch in chs:
        t = read_text(os.path.join(handbook_dir(data), ch + ".md"))
        found = re.findall(r"^## (?:\d+\. )?(.+?)\s*$", t, re.M)
        if found != CHAPTER_HEADINGS:
            errors.append("kasikirja/%s.md: headings differ from the template" % ch)
        blk = BLOCK_RE.search(t)
        if not blk or "\n".join(wrap("viat", viat_block(data, ch))) != blk.group(0):
            errors.append("kasikirja/%s.md: generated findings block out of date (run: tietopohja.py . kasikirja)" % ch)
        if not t.startswith("# %s " % ch[:2]):
            errors.append("kasikirja/%s.md: first line must be '# %s <title>'" % (ch, ch[:2]))
    linked = {i for r in fn for i in r["havainnot"].split()}
    missing = sorted(i for i, r in data.risk.items() if r.get("tila") == "avoin" and r.get("prioriteetti") in ("P1", "P2", "P3")
                     and i not in linked)
    if len(chs) >= 15 and missing:
        errors.append("handbook: %d open P1-P3 findings not linked to any function: %s" % (len(missing), " ".join(missing)))
    elif missing:
        warnings.append("handbook incomplete: %d open P1-P3 findings not yet linked" % len(missing))


def write_rows(path, header, rows):
    out = io.StringIO(newline="")
    w = csv.writer(out, lineterminator="\n")
    w.writerow(header)
    w.writerows(rows)
    with open(path, "wb") as fh:
        fh.write(out.getvalue().encode("utf-8"))


def cmd_tuo(data, files):
    """Import writer fragments: vaikutukset_*.csv (id,ulottuvuus,arvo,peruste,lahde) and
    ohjelmat_*.csv (id,ohjelmat,peruste). Existing impact rows of the same (id, dimension) are
    replaced; every value is validated first and nothing is written if anything is wrong."""
    reg = os.path.join(data.doc, "rekisteri")
    dims = [d["ulottuvuus"] for d in data.dims]
    values = ("vahvistettu", "mahdollinen", "ei_perusteltu", "ei_tiedossa", "ei_sovellu")
    progs_ok = ("HkKisaWin", "HkMaali", "ViestiWin", "ViestiMaali", "ViestiLuenta")
    imp_new, prog_new, errs = {}, {}, []
    for f in files:
        rows = list(csv.DictReader(io.open(f, encoding="utf-8-sig", newline="")))
        base = os.path.basename(f)
        for r in rows:
            r = {k.strip(): (v or "").strip() for k, v in r.items() if k}
            i = r.get("id", "")
            if i not in data.reg:
                errs.append("%s: unknown id %r" % (base, i))
                continue
            if "ulottuvuus" in r:
                if r["ulottuvuus"] not in dims or r.get("arvo") not in values or not r.get("peruste"):
                    errs.append("%s: bad impact row %s/%s/%s" % (base, i, r.get("ulottuvuus"), r.get("arvo")))
                    continue
                key = (i, r["ulottuvuus"])
                if key in imp_new:
                    errs.append("%s: duplicate %s/%s" % (base, i, r["ulottuvuus"]))
                imp_new[key] = [i, r["ulottuvuus"], r["arvo"], r["peruste"], r.get("lahde", "")]
            elif "ohjelmat" in r:
                p = r["ohjelmat"].replace(",", " ").split()
                if not p or any(x not in progs_ok for x in p):
                    errs.append("%s: bad programs for %s: %r" % (base, i, r["ohjelmat"]))
                    continue
                prog_new[i] = " ".join(x for x in progs_ok if x in p)
    for i in {k[0] for k in imp_new}:
        if data.kind(i) == "H":
            missing = [d for d in dims if (i, d) not in imp_new and d not in data.imp.get(i, {})]
            if missing:
                errs.append("%s: impact dimensions missing: %s" % (i, " ".join(missing)))
    if errs:
        for e in errs:
            print("ERROR", e.encode("ascii", "replace").decode())
        print("nothing written")
        return 1
    vpath = os.path.join(reg, "vaikutukset.csv")
    cur = {(r["id"], r["ulottuvuus"]): [r["id"], r["ulottuvuus"], r["arvo"], r["peruste"], r["lahde"]]
           for r in rcsv(vpath)}
    cur.update(imp_new)
    order = {d: n for n, d in enumerate(dims)}
    rows = sorted(cur.values(), key=lambda r: (r[0], order[r[1]]))
    write_rows(vpath, ["id", "ulottuvuus", "arvo", "peruste", "lahde"], rows)
    for name in ("riskiarvio.csv", "tekninen-velka.csv"):
        p = os.path.join(reg, name)
        with io.open(p, encoding="utf-8", newline="") as fh:
            rr = list(csv.reader(fh))
        h = rr[0]
        n = 0
        for r in rr[1:]:
            if r[0] in prog_new:
                r[h.index("ohjelmat")] = prog_new[r[0]]
                r[h.index("ohjelmat_peruste")] = "tarkistettu"
                n += 1
        write_rows(p, h, rr[1:])
        print("%s: programs updated for %d rows" % (name, n))
    print("impacts: %d rows imported, register now %d rows" % (len(imp_new), len(rows)))
    return 0


def main(argv):
    if len(argv) < 3:
        print(__doc__)
        return 2
    data = Data(argv[1])
    cmd, ids = argv[2], argv[3:]
    if cmd == "runko":
        cmd_runko(data, ids)
    elif cmd == "lohkot":
        cmd_lohkot(data, ids)
    elif cmd == "kuittaa":
        cmd_kuittaa(data, ids)
    elif cmd == "kasikirja":
        cmd_kasikirja(data)
    elif cmd == "tuo":
        return cmd_tuo(data, ids)
    elif cmd == "kooste":
        cmd_kooste(data)
    elif cmd == "tarkista":
        return cmd_tarkista(data)
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
