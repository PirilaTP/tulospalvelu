#!/usr/bin/env python3
"""Generate the data of the local knowledge-base portal (phase K-4).

Usage (from the repository root):
    python -I doc/arkkitehtuuri/tyokalut/portaali.py .            write doc/tietopohja/portaali/data/data.js
    python -I doc/arkkitehtuuri/tyokalut/portaali.py . --tarkista exit 1 if data.js is missing or stale

The portal itself (index.html, app.js, tyyli.css in doc/tietopohja/portaali/) is static and
hand-maintained; it reads only data/data.js, which this script builds from the registers, the
defect cards, the handbook and the audit documents. Nothing is fetched from the network; the
page needs no server (open index.html from disk). data/ is generated and not kept in version
control. Markdown is converted to HTML here with a small converter (no external libraries).
"""
import csv
import hashlib
import html
import io
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.dont_write_bytecode = True
import synteesi  # noqa: E402
import tietopohja as tp  # noqa: E402

OUT_REL = os.path.join("doc", "tietopohja", "portaali", "data", "data.js")

# ------------------------------------------------------------------ markdown ---
INLINE_CODE = re.compile(r"`([^`]+)`")
LINK = re.compile(r"\[([^\]]+)\]\(([^)\s]+)\)")
BOLD = re.compile(r"\*\*(.+?)\*\*")
ITAL = re.compile(r"(?<![\w*])_(?!_)(.+?)_(?![\w])")
IDREF = re.compile(r"\b(H-\d{3}|T-\d{3})\b")


def inline(text, link_ids=True):
    parts = []
    pos = 0
    for m in INLINE_CODE.finditer(text):
        parts.append(("t", text[pos:m.start()]))
        parts.append(("c", m.group(1)))
        pos = m.end()
    parts.append(("t", text[pos:]))
    out = []
    for kind, s in parts:
        if kind == "c":
            out.append("<code>%s</code>" % html.escape(s))
            continue
        links = []

        def keep_link(m):
            links.append((m.group(1), m.group(2)))
            return "\x00%d\x00" % (len(links) - 1)
        s = LINK.sub(keep_link, s)
        s = html.escape(s, quote=False)
        s = BOLD.sub(r"<strong>\1</strong>", s)
        s = ITAL.sub(r"<em>\1</em>", s)
        if link_ids:
            s = IDREF.sub(r'<a href="#/kortti/\1">\1</a>', s)

        def put_link(m):
            label, href = links[int(m.group(1))]
            return '<a href="%s">%s</a>' % (html.escape(portal_href(href)), html.escape(label))
        s = re.sub("\x00(\\d+)\x00", put_link, s)
        out.append(s)
    return "".join(out)


def portal_href(href):
    if href.startswith("#"):
        return href
    m = re.search(r"sanasto\.md(#.*)?$", href)
    if m:
        return "#/sanasto"
    m = re.search(r"kasikirja/([0-9a-z-]+)\.md$", href) or re.match(r"([0-9]{2}-[a-z0-9-]+)\.md$", href)
    if m:
        return "#/kasikirja/" + m.group(1)
    m = re.search(r"(H|T)-\d{3}\.md$", href)
    if m:
        return "#/kortti/" + os.path.basename(href)[:-3]
    return href


def md_to_html(md, link_ids=True):
    lines = md.replace("\r\n", "\n").split("\n")
    out = []
    i = 0
    para = []
    lists = []  # stack of (indent, tag)

    def flush_para():
        if para:
            out.append("<p>%s</p>" % inline(" ".join(x.strip() for x in para), link_ids))
            para.clear()

    def close_lists(to_indent=-1):
        while lists and lists[-1][0] > to_indent:
            out.append("</li></%s>" % lists.pop()[1])

    while i < len(lines):
        line = lines[i]
        s = line.strip()
        if s.startswith("```"):
            flush_para()
            close_lists()
            lang = s[3:].strip()
            buf = []
            i += 1
            while i < len(lines) and not lines[i].strip().startswith("```"):
                buf.append(lines[i])
                i += 1
            cls = ' class="mermaid-src"' if lang == "mermaid" else ""
            if lang == "mermaid":
                out.append('<div class="note">Kaavio (Mermaid-lähde; portaali ei käytä ulkoisia kirjastoja, joten kaavio näytetään tekstinä):</div>')
            out.append("<pre%s><code>%s</code></pre>" % (cls, html.escape("\n".join(buf))))
            i += 1
            continue
        if s.startswith("<!--"):
            while i < len(lines) and "-->" not in lines[i]:
                i += 1
            i += 1
            continue
        if not s:
            flush_para()
            close_lists()
            i += 1
            continue
        m = re.match(r"(#{1,6})\s+(.*)", s)
        if m:
            flush_para()
            close_lists()
            lvl = min(len(m.group(1)) + 1, 6)
            out.append("<h%d>%s</h%d>" % (lvl, inline(m.group(2), link_ids), lvl))
            i += 1
            continue
        if s.startswith("|") and i + 1 < len(lines) and re.match(r"^\|?\s*:?-{2,}", lines[i + 1].strip()):
            flush_para()
            close_lists()
            head = [c.strip() for c in s.strip("|").split("|")]
            i += 2
            rows = []
            while i < len(lines) and lines[i].strip().startswith("|"):
                rows.append([c.strip() for c in lines[i].strip().strip("|").split("|")])
                i += 1
            t = ["<table><thead><tr>"] + ["<th>%s</th>" % inline(c, link_ids) for c in head] + ["</tr></thead><tbody>"]
            for r in rows:
                t.append("<tr>" + "".join("<td>%s</td>" % inline(c, link_ids) for c in r) + "</tr>")
            t.append("</tbody></table>")
            out.append("".join(t))
            continue
        if s.startswith(">"):
            flush_para()
            close_lists()
            buf = []
            while i < len(lines) and lines[i].strip().startswith(">"):
                buf.append(lines[i].strip()[1:].strip())
                i += 1
            out.append("<blockquote>%s</blockquote>" % "<br>".join(inline(b, link_ids) for b in buf))
            continue
        m = re.match(r"^(\s*)([-*]|\d+\.)\s+(.*)", line)
        if m:
            flush_para()
            indent = len(m.group(1).replace("\t", "    "))
            tag = "ol" if m.group(2)[0].isdigit() else "ul"
            if not lists or indent > lists[-1][0]:
                out.append("<%s><li>" % tag)
                lists.append((indent, tag))
            else:
                close_lists(indent)
                if lists and lists[-1][0] == indent:
                    out.append("</li><li>")
                else:
                    out.append("<%s><li>" % tag)
                    lists.append((indent, tag))
            out.append(inline(m.group(3), link_ids))
            i += 1
            continue
        if lists and line.startswith(" "):
            out.append(" " + inline(s, link_ids))
            i += 1
            continue
        close_lists()
        para.append(line)
        i += 1
    flush_para()
    close_lists()
    return "\n".join(out)


# --------------------------------------------------------------- data model ---
def sha(path):
    return hashlib.sha256(open(path, "rb").read()).hexdigest()[:12]


def git_out(root, *args):
    p = subprocess.run(["git", "-C", root] + list(args), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    return p.stdout.decode("utf-8", "replace").strip() if p.returncode == 0 else "?"


USER_H = ["Lyhyesti", "Missä tilanteessa vika voi ilmetä", "Mitä käyttäjä voi huomata", "Vaikutukset kilpailuun",
          "Mitä kilpailun aikana voi tapahtua", "Palautuminen ja uudelleenkäynnistys",
          "Mitä käyttäjä voi tehdä riskin pienentämiseksi"]
USER_T = ["Mistä on kyse", "Mitä haittaa siitä on nykyisin", "Mihin vikoihin se liittyy"]
USER_K = ["Lyhyesti", "Mitä tämä tarkoittaa käyttäjälle"]


def card_parts(text):
    """Return (title, summary_md, {heading: md}) of a card; comments removed."""
    body = re.sub(r"<!--.*?-->", "", text, flags=re.S)
    lines = body.split("\n")
    title = lines[0][2:].strip()
    rest = "\n".join(lines[1:])
    chunks = re.split(r"^## (?:\d+\. )?(.+?)\s*$", rest, flags=re.M)
    summary = chunks[0].strip()
    secs = {}
    for k in range(1, len(chunks), 2):
        secs[chunks[k]] = chunks[k + 1].strip()
    return title, summary, secs


def source_paths(data):
    reg = os.path.join(data.doc, "rekisteri")
    paths = [os.path.join(reg, f) for f in sorted(os.listdir(reg)) if f.endswith((".md", ".csv"))]
    paths += [os.path.join(data.doc, f) for f in ("omistajan-paatokset.md", "seuranta.md", "korjaussuunnitelma.md",
                                                  "auditoinnin-yhteenveto.md", "regressiotestaussuunnitelma.md")]
    kb = os.path.join(data.root, "doc", "tietopohja")
    for sub in ("vikakortit/H", "vikakortit/T", "kasikirja"):
        d = os.path.join(kb, sub)
        if os.path.isdir(d):
            paths += [os.path.join(d, f) for f in sorted(os.listdir(d)) if f.endswith((".md", ".csv"))]
    paths.append(os.path.join(kb, "sanasto.md"))
    return paths


def source_digest(data):
    h = hashlib.sha256()
    for p in source_paths(data):
        h.update(os.path.relpath(p, data.root).replace(os.sep, "/").encode())
        h.update(open(p, "rb").read())
    return h.hexdigest()[:16]


def build(root):
    data = tp.Data(root)
    reg = os.path.join(data.doc, "rekisteri")
    dims = data.dims
    ids = sorted(data.reg, key=lambda x: (x[0], x))
    cards = set(tp.card_ids(data))
    items = []
    for i in ids:
        r = data.reg[i]
        sr = data.state_row(i)
        rk = data.risk.get(i, {})
        imp = {d: v["arvo"] for d, v in data.imp.get(i, {}).items()}
        title = r["otsikko"]
        user_html = tech_html = summary_html = md = ""
        valmis = False
        if i in cards:
            text = tp.read_text(data.path(i))
            md = re.sub(r"<!--.*?-->\n?", "", text, flags=re.S)
            valmis = "<selkokielinen otsikko" not in text and "<mitä alun perin" not in text
            if valmis:
                ctitle, summary, secs = card_parts(text)
                title = ctitle.split(" – ", 1)[-1]
                want = {"H": USER_H, "K": USER_K, "T": USER_T}[data.kind(i)]
                summary_html = md_to_html(summary)
                user_html = "".join("<h3>%s</h3>%s" % (html.escape(h), md_to_html(secs.get(h, "")))
                                    for h in want if h in secs)
                tech_html = md_to_html(md.split("\n", 1)[1])
        kk = data.primary.get(i, "-")
        items.append({
            "id": i, "laji": "H" if i.startswith("H") else "T", "otsikko": title,
            "rekisteri": r["otsikko"], "era": r["era"], "tila": sr.get("tila", ""),
            "prioriteetti": rk.get("prioriteetti", "-") if i.startswith("H") else "-",
            "vakavuus": rk.get("tekninen_vakavuus") or r["vakavuus"],
            "todennus": rk.get("todennus", "-") if i.startswith("H") else "-",
            "saavutettavuus": rk.get("saavutettavuus", "-") if i.startswith("H") else "-",
            "ohjelmat": sr.get("ohjelmat", "").split(), "ohjelmat_peruste": sr.get("ohjelmat_peruste", ""),
            "juurisyy": sr.get("juurisyy", ""), "juurisyy_muut": sr.get("juurisyy_muut", "").split(),
            "kk": kk, "kk_muut": rk.get("kk_muut", "").split(), "aalto": data.packs.get(kk, {}).get("aalto", ""),
            "vaikutukset": imp, "kortti": i in cards and valmis, "md": md if valmis else "",
            "yhteenveto": summary_html, "kayttaja": user_html, "tekninen": tech_html,
            "korjaus": sr.get("korjaus", ""),
        })
    packs = []
    for p in data.packs.values():
        members = (p["havainnot"] + " " + p["tekninen_velka"]).split()
        packs.append({"kk": p["kk"], "nimi": p["nimi"], "prioriteetti": p["prioriteetti"], "aalto": p["aalto"],
                      "vaihe": p["vaihe"], "juurisyy": p["juurisyy"].split(), "riippuu": p["riippuu"].split(),
                      "riippuu_osat": p["riippuu_osat"].split(), "jasenet": members,
                      "op": p["omistajan_paatokset"].split(), "kysymykset": p["kysymykset"].split(),
                      "tyomaara": p["tyomaara"], "muutosriski": p["muutosriski"], "testit": p["testit"],
                      "haara": p["haara"]})
    js = []
    summ = synteesi.read(os.path.join(data.doc, "auditoinnin-yhteenveto.md"))
    for m in re.finditer(r"\| \d+ \| \*\*(JS-\d+)\*\* ([^|]+?) \| ([^|]+) \|", summ):
        js.append({"js": m.group(1), "nimi": m.group(2).split(" / ")[0].strip(), "mekanismi": m.group(3).strip()})
    ops = []
    op = synteesi.read(os.path.join(data.doc, "omistajan-paatokset.md"))
    for m in re.finditer(r"^## (OP-\d+) ([^\n]*)\n(.*?)(?=^## |\Z)", op, re.S | re.M):
        st = re.search(r"\*\*Tila / status\.\*\* ([^\n]*)", m.group(3))
        ops.append({"op": m.group(1), "nimi": m.group(2), "tila": st.group(1) if st else "?",
                    "html": md_to_html(m.group(3))})
    qs = [{"q": "Q%d" % n, "tila": q["tila"] or "?", "kuvaus": q["kuvaus"]} for n, q in sorted(data.qs.items())]
    handbook = []
    hb = os.path.join(root, "doc", "tietopohja", "kasikirja")
    if os.path.isdir(hb):
        for f in sorted(os.listdir(hb)):
            if re.match(r"\d{2}-.*\.md$", f):
                t = tp.read_text(os.path.join(hb, f))
                handbook.append({"id": f[:-3], "otsikko": t.split("\n", 1)[0].lstrip("# ").strip(),
                                 "html": md_to_html(t.split("\n", 1)[1] if "\n" in t else ""), "teksti": t})
        aineisto = os.path.join(hb, "aineisto")
        if os.path.isdir(aineisto):
            for f in sorted(os.listdir(aineisto)):
                if f.endswith(".md"):
                    t = tp.read_text(os.path.join(aineisto, f))
                    handbook.append({"id": "aineisto-" + f[:-3], "otsikko": "Aineisto: " + t.split("\n", 1)[0].lstrip("# ").strip(),
                                     "html": md_to_html(t.split("\n", 1)[1] if "\n" in t else ""), "teksti": t})
    functions = []
    tf = os.path.join(hb, "toiminnot.csv")
    if os.path.exists(tf):
        for r in tp.rcsv(tf):
            functions.append({k: r[k] for k in r})
    levels = {}
    for r in tp.rcsv(os.path.join(reg, "tiedostot.csv")):
        d = levels.setdefault(r["hakemisto"], {"1": 0, "2": 0, "3": 0, "4": 0, "rivit": 0, "tiedostot": 0})
        d[r["analyysitaso"]] = d.get(r["analyysitaso"], 0) + 1
        d["rivit"] += int(r["rivit"] or 0)
        d["tiedostot"] += 1
    sanasto = md_to_html(tp.read_text(os.path.join(root, "doc", "tietopohja", "sanasto.md")).split("\n", 1)[1], False)
    docs = {}
    for name, f in (("yhteenveto", "auditoinnin-yhteenveto.md"), ("regressio", "regressiotestaussuunnitelma.md")):
        docs[name] = md_to_html(synteesi.read(os.path.join(data.doc, f)).split("\n", 1)[1])
    revs = sorted({data.reg[i]["rev"] for i in ids})
    meta = {
        "tuotettu": git_out(root, "log", "-1", "--format=%ci"),
        "commit": git_out(root, "rev-parse", "--short", "HEAD"),
        "lahdetiiviste": source_digest(data),
        "viiterevisiot": revs,
        "rekisterit": {f: sha(os.path.join(reg, f)) for f in sorted(os.listdir(reg)) if f.endswith((".csv", ".md"))},
        "ulottuvuudet": [{"id": d["ulottuvuus"], "ryhma": d["ryhma"], "nimi": d["nimi"], "kysymys": d["kysymys"]} for d in dims],
        "todennus": tp.TODENNUS, "saavutettavuus": tp.SAAVUTETTAVUUS,
    }
    return {"meta": meta, "havainnot": items, "kokonaisuudet": packs, "juurisyyt": js, "paatokset": ops,
            "kysymykset": qs, "kasikirja": handbook, "toiminnot": functions, "kattavuus": levels,
            "sanasto": sanasto, "asiakirjat": docs}


def main(argv):
    root = argv[1] if len(argv) > 1 else "."
    out = os.path.join(root, OUT_REL)
    if "--tarkista" in argv:
        if not os.path.exists(out):
            print("portal data missing: run portaali.py . (not an error before the first build)")
            return 1
        cur = open(out, "rb").read().decode("utf-8")
        m = re.search(r'"lahdetiiviste":\s*"([0-9a-f]+)"', cur)
        want = source_digest(tp.Data(root))
        if not m or m.group(1) != want:
            print("portal data is stale (sources changed since it was generated): run portaali.py .")
            return 1
        print("portal data up to date (%s)" % want)
        return 0
    d = build(root)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    payload = json.dumps(d, ensure_ascii=False, separators=(",", ":"))
    payload = payload.replace("</", "<\\/")
    with open(out, "wb") as fh:
        fh.write(("// GENERATED by doc/arkkitehtuuri/tyokalut/portaali.py - do not edit\n"
                  "window.TIETOPOHJA = " + payload + ";\n").encode("utf-8"))
    print("written %s (%d findings, %d cards, %d chapters, %d KB)" % (
        OUT_REL.replace(os.sep, "/"), len(d["havainnot"]), sum(1 for x in d["havainnot"] if x["kortti"]),
        len(d["kasikirja"]), len(payload) // 1024))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
