#!/usr/bin/env python3
"""Knowledge base (phase K) tool: defect cards in doc/tietopohja/vikakortit/.

Usage (from the repository root):
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . runko  <id> ...   new card from the template
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . lohkot [<id> ...] rewrite generated blocks
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kuittaa <id> ...  record the register state
                                                                           the text was checked against
    python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . tarkista          check all cards (exit 1 on error)

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
        if "<selkokielinen otsikko" in text or "<mitä alun perin" in text:
            errors.append("%s: title not written" % i)
        for n, line in enumerate(body.splitlines(), 1):
            if LINE_REF.search(line):
                errors.append("%s: line reference in authored text: %s" % (i, line.strip()[:90]))
            if PRIO.search(line):
                errors.append("%s: priority in authored text (only in the generated block): %s" % (i, line.strip()[:90]))
            for w in FREQ_WORDS.findall(line):
                warnings.append("%s: frequency word '%s' - needs a source: %s" % (i, w, line.strip()[:80]))
            for x in ID_ANY.findall(line):
                if x not in known:
                    errors.append("%s: unknown identifier %s in authored text" % (i, x))
        sr = data.state_row(i)
        own = {sr.get("juurisyy")} | set(sr.get("juurisyy_muut", "").split())
        for x in set(re.findall(r"\bJS-\d+\b", body)):
            if x not in own:
                warnings.append("%s: mentions %s, which is not its root cause in the register" % (i, x))
    print("cards: %d (H %d, T %d)" % (len(ids), sum(1 for i in ids if i[0] == "H"), sum(1 for i in ids if i[0] == "T")))
    for w in warnings:
        print("WARNING", w.encode("ascii", "replace").decode())
    for e in errors:
        print("ERROR", e.encode("ascii", "replace").decode())
    print("OK" if not errors else "%d errors" % len(errors))
    return 1 if errors else 0


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
    elif cmd == "tarkista":
        return cmd_tarkista(data)
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
