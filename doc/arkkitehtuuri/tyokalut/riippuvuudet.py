#!/usr/bin/env python3
"""C-10 part A: per-product inventory of external dependencies.

Products and the projects that build them (shipped configuration):
  HkMaali      VS/Hk/HkMaali/HkMaali.vcxproj  Release|Win32  + tputilv2, dbboxm
  ViestiMaali  VS/V/JukMaali520.vcxproj       Release|Win32  + tputilv2, dbboxm
  ViestiLuenta VS/V/JukLuenta520.vcxproj      Release|Win32  + tputilv2, dbboxm
  HkKisaWin    RADStudio10/HkKisaWin.cbproj   Release        + Tputil-XE, DBboxm-XE
  ViestiWin    RADStudio10/ViestiWin.cbproj   Release Build  + Tputil-XE, DBboxm-XE
(the .cbproj default <Config>, which build-gui.ps1 builds; CI builds Release|Win32).

What it does:
 1. Evaluates each project file for its shipped configuration (MSBuild property
    groups in order, $(X) substitution, simple condition evaluation): compiled
    sources, preprocessor defines, include path, link libraries, runtime
    packages, RTL options.
 2. Builds the include closure of every compiled source (quoted includes
    resolved from the including directory, then the project include path,
    then a unique basename anywhere under TPsource/V52; #if around includes is
    ignored = over-approximation).
 3. Tracks the #if/#ifdef/#elif/#else stack of every line and evaluates it per
    translation-unit owner (product or library project) with tri-state logic:
    project defines + compiler predefines + macros #defined unconditionally in a
    header of the closure -> known; a macro #defined only conditionally or
    #undef'd anywhere -> unknown.
 4. Lists dependency markers (system/VCL headers, link pragmas, Win32 API areas,
    database, HTTP, browser, sockets, serial ports, registry, shell, COM, URLs,
    SQL text, device ids LID_*) per line, with the condition and per-product
    reachability (compiled / included, active yes / no / unknown).
 5. Lists the component classes declared in every .dfm and maps each form to
    the products that compile its unit.

The analysis is lexical. It does not run a preprocessor; header order, macros
defined after use and #include inside a false branch are not modelled. Output is
raw evidence, not a verified result.

Usage: python -I riippuvuudet.py <repo-root> <out-dir>
Writes: tuotteet.csv, kaannosyksikot.csv, tiedostot.csv, osumat.csv,
        dfm.csv, makrot.csv, yhteenveto.txt
"""
import csv
import os
import re
import sys
import xml.etree.ElementTree as ET
from collections import Counter, defaultdict

PRODUCTS = [
    ("HkMaali", "msvc", "VS/Hk/HkMaali/HkMaali.vcxproj", "Release|Win32", ["tputilv2", "dbboxm"]),
    ("ViestiMaali", "msvc", "VS/V/JukMaali520.vcxproj", "Release|Win32", ["tputilv2", "dbboxm"]),
    ("ViestiLuenta", "msvc", "VS/V/JukLuenta520.vcxproj", "Release|Win32", ["tputilv2", "dbboxm"]),
    ("HkKisaWin", "bcc32", "RADStudio10/HkKisaWin.cbproj", None, ["Tputil-XE", "DBboxm-XE"]),
    ("ViestiWin", "bcc32", "RADStudio10/ViestiWin.cbproj", None, ["Tputil-XE", "DBboxm-XE"]),
]
LIBS = {
    "tputilv2": ("msvc", "VS/Libs/tputilv2.vcxproj", "Release|Win32"),
    "dbboxm": ("msvc", "VS/Libs/dbboxm/dbboxm.vcxproj", "Release|Win32"),
    "Tputil-XE": ("bcc32", "RADStudio10/Tputil-XE.cbproj", None),
    "DBboxm-XE": ("bcc32", "RADStudio10/DBboxm-XE.cbproj", None),
}
# Compiler predefined macros relevant to the code base. Values: 1 = defined.
PREDEF = {
    "msvc": {"_MSC_VER": 1950, "_WIN32": 1, "_M_IX86": 600, "__cplusplus": 199711,
             "_UNICODE": 1, "UNICODE": 1, "_MT": 1},
    "bcc32": {"__BORLANDC__": 0x0730, "__WIN32__": 1, "_WIN32": 1, "__TURBOC__": 0x0730,
              "__cplusplus": 1, "__MT__": 1, "_RTLDLL": 1, "UNICODE": 1, "_UNICODE": 1,
              "__CODEGEARC__": 0x0730},
}
# bcc32 projects without -tU: DBboxm-XE (C-5 tbl1, TCHAR=char)
NO_UNICODE = {"DBboxm-XE"}
# Macros known to be defined by system or VCL headers included before use.
SYSTEM_MACROS = {"WIN32": None}  # WIN32 comes from windows.h in bcc32 libs (C-5 section 8)

SRC_EXT = (".cpp", ".c")


def read_text(path):
    with open(path, "rb") as f:
        data = f.read()
    if data.startswith(b"\xef\xbb\xbf"):
        return data[3:].decode("utf-8", errors="replace")
    if data.startswith(b"\xff\xfe"):
        return data[2:].decode("utf-16-le", errors="replace")
    try:
        return data.decode("utf-8")
    except UnicodeDecodeError:
        return data.decode("cp1252", errors="replace")


def norm(p):
    return os.path.normpath(p).replace("\\", "/")


# ---------------------------------------------------------------- MSBuild

def strip_ns(tag):
    return tag.split("}", 1)[-1]


def subst(val, props):
    def rep(m):
        name = m.group(1)
        if name.startswith("%("):
            return ""
        return props.get(name, "")
    val = re.sub(r"%\(\w+\)", "", val)
    return re.sub(r"\$\((\w+)\)", rep, val)


def eval_cond(cond, props):
    if not cond or not cond.strip():
        return True
    c = subst(cond, props)
    # 'a'=='b'  'a'!='b'  and/or/parens. Values never contain quotes here.
    c = re.sub(r"\band\b", " and ", c, flags=re.I)
    c = re.sub(r"\bor\b", " or ", c, flags=re.I)
    c = c.replace("'", '"')
    if re.search(r"[^\w\s\"=!().|\\/:;,$-]", c.replace("\"", "")):
        pass
    try:
        return bool(eval(c, {"__builtins__": {}}, {}))
    except Exception:
        # Exists('...') and the like: treat as false, record
        return False


def eval_project(path, cfg_name):
    """Return dict with props, compile items, defines, libs, etc."""
    tree = ET.parse(path)
    root = tree.getroot()
    props = {"Platform": "Win32", "Configuration": "", "Config": "",
             "MSBuildProjectDirectory": os.path.dirname(path)}
    is_vcx = path.lower().endswith(".vcxproj")
    if is_vcx:
        conf, plat = cfg_name.split("|")
        props["Configuration"] = conf
        props["Platform"] = plat
    elif cfg_name:
        # .cbproj: a preset Config skips the "'$(Config)'==''" default group
        props["Config"] = cfg_name
    items = defaultdict(list)
    itemdefs = defaultdict(dict)
    groups_used = []
    for el in root:
        tag = strip_ns(el.tag)
        cond = el.get("Condition", "")
        if tag == "PropertyGroup":
            if not eval_cond(cond, props):
                continue
            groups_used.append(cond)
            for p in el:
                ptag = strip_ns(p.tag)
                pcond = p.get("Condition", "")
                if not eval_cond(pcond, props):
                    continue
                props[ptag] = subst(p.text or "", props)
                if ptag == "Config" and not is_vcx and not cfg_name:
                    pass
        elif tag == "ItemDefinitionGroup":
            if not eval_cond(cond, props):
                continue
            for kind in el:
                k = strip_ns(kind.tag)
                for p in kind:
                    itemdefs[k][strip_ns(p.tag)] = subst(p.text or "", props)
        elif tag == "ItemGroup":
            if not eval_cond(cond, props):
                continue
            for it in el:
                k = strip_ns(it.tag)
                icond = it.get("Condition", "")
                if not eval_cond(icond, props):
                    continue
                inc = it.get("Include")
                if inc is None:
                    continue
                meta = {strip_ns(m.tag): (m.text or "") for m in it}
                excl = False
                for m in it:
                    if strip_ns(m.tag) == "ExcludedFromBuild" and eval_cond(m.get("Condition", ""), props) \
                            and (m.text or "").strip().lower() == "true":
                        excl = True
                if not excl:
                    items[k].append((inc, meta))
    return props, items, itemdefs


def project_info(src, compiler, rel, cfg):
    path = os.path.join(src, rel)
    pdir = os.path.dirname(path)
    info = {"project": rel, "compiler": compiler}
    if compiler == "msvc":
        props, items, idefs = eval_project(path, cfg)
        cl = idefs.get("ClCompile", {})
        ln = idefs.get("Link", {}) or idefs.get("Lib", {})
        defs = [d for d in cl.get("PreprocessorDefinitions", "").split(";") if d.strip()]
        cs = props.get("CharacterSet", "")
        if cs == "Unicode":
            defs += ["UNICODE", "_UNICODE"]
        elif cs == "MultiByte":
            defs += ["_MBCS"]
        incs = [d for d in cl.get("AdditionalIncludeDirectories", "").split(";") if d.strip()]
        info.update(config=cfg, defines=defs,
                    include_dirs=[norm(os.path.join(pdir, d)) for d in incs],
                    link_libs=[x for x in (ln.get("AdditionalDependencies", "")).split(";") if x.strip()],
                    runtime=cl.get("RuntimeLibrary", ""),
                    toolset=props.get("PlatformToolset", ""),
                    charset=cs,
                    wpo=props.get("WholeProgramOptimization", ""),
                    subsystem=ln.get("SubSystem", ""),
                    target_xp=props.get("PlatformToolset", "").endswith("_xp"),
                    sources=[norm(os.path.join(pdir, i)) for i, _ in items.get("ClCompile", [])],
                    packages=[], link_packages=[], lib_files=[])
    else:
        # first pass: find default config
        txt = open(path, "rb").read().decode("utf-8", errors="replace")
        m = re.search(r"<Config Condition=\"'\$\(Config\)'==''\">([^<]+)</Config>", txt)
        cfgname = cfg or (m.group(1) if m else "Release")
        props, items, idefs = eval_project(path, cfg)
        # eval_project sets Config from the default; check it
        info["config"] = props.get("Config", cfgname)
        defs = []
        for key in ("Defines", "BCC_Defines"):
            defs += [d for d in props.get(key, "").split(";") if d.strip()]
        incs = []
        for key in ("IncludePath", "BCC_IncludePath"):
            incs += [d for d in props.get(key, "").split(";") if d.strip()]
        srcs = [norm(os.path.join(pdir, i)) for i, _ in items.get("CppCompile", [])]
        info.update(defines=defs,
                    include_dirs=[norm(os.path.join(pdir, d)) for d in incs if "$(" not in d and not d.startswith("$")],
                    link_libs=[i for i, _ in items.get("LibFiles", [])],
                    runtime="DynamicRTL=%s UsePackages=%s Multithreaded=%s UsingDelphiRTL=%s" % (
                        props.get("DynamicRTL", ""), props.get("UsePackages", ""),
                        props.get("Multithreaded", ""), props.get("UsingDelphiRTL", "")),
                    toolset="ProjectVersion %s" % props.get("ProjectVersion", ""),
                    charset="_TCHARMapping=%s" % props.get("_TCHARMapping", ""),
                    wpo="", subsystem=props.get("ILINK_AppType", ""), target_xp=False,
                    sources=srcs,
                    packages=[p for p in props.get("PackageImports", "").split(";") if p.strip()],
                    link_packages=[p for p in props.get("LinkPackageImports", "").split(";") if p.strip()],
                    link_package_statics=[p for p in props.get("LinkPackageStatics", "").split(";") if p.strip()],
                    all_package_libs=[p for p in props.get("AllPackageLibs", "").split(";") if p.strip()],
                    lib_files=[i for i, _ in items.get("LibFiles", [])],
                    dynamic_rtl=props.get("DynamicRTL", ""),
                    use_packages=props.get("UsePackages", ""))
    return info


def define_map(defs, compiler, owner):
    d = dict(PREDEF[compiler])
    if owner in NO_UNICODE:
        d.pop("UNICODE", None)
        d.pop("_UNICODE", None)
    for x in defs:
        x = x.strip()
        if not x:
            continue
        if "=" in x:
            k, v = x.split("=", 1)
            try:
                d[k.strip()] = int(v.strip(), 0)
            except ValueError:
                d[k.strip()] = 1
        else:
            d[x] = 1
    return d


# ---------------------------------------------------------- preprocessing

INC_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*([<"])([^>"]+)[>"]', re.M)
PP_LINE = re.compile(r"^[ \t]*#[ \t]*(if|ifdef|ifndef|elif|else|endif|define|undef)\b(.*)$")


def strip_comments_keep_strings(text):
    """Blank // and /* */ comments; keep string and char literals and newlines."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            if j < 0:
                j = n
            out.append(" " * (j - i))
            i = j
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join(ch if ch == "\n" else " " for ch in text[i:j]))
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(text[i:j])
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def join_continuations(lines):
    """Return list of (logical_text, first_line_no) for preprocessor parsing,
    keeping a per-physical-line view."""
    res = []
    i = 0
    while i < len(lines):
        s = lines[i]
        start = i
        while s.endswith("\\") and i + 1 < len(lines):
            i += 1
            s = s[:-1] + " " + lines[i]
        res.append((s, start))
        i += 1
    return res


class FileInfo:
    def __init__(self, path, text):
        self.path = path
        self.text = text
        self.clean = strip_comments_keep_strings(text)
        self.lines = self.clean.split("\n")
        self.raw_lines = text.split("\n")
        self.cond = [""] * (len(self.lines) + 1)   # condition string per line (1-based)
        self.cond_list = [()] * (len(self.lines) + 1)
        self.defines = []   # (name, line, cond_tuple, kind define/undef, value)
        self.includes = []  # (kind, name, line)
        self._parse()

    def _parse(self):
        stack = []  # each frame: list of conditions; current branch expr and prior branch exprs
        cur = ()
        logical = join_continuations(self.lines)
        # include guard: first directive #ifndef X / #if !defined(X), next #define X.
        # The guard condition is true on the first inclusion, so it is replaced by 1.
        self.guard = None
        pps = [s for s, ln in logical if PP_LINE.match(s)]
        if pps:
            g1 = re.match(r"^\s*#\s*(?:ifndef\s+(\w+)|if\s+!\s*defined\s*\(?\s*(\w+)\s*\)?\s*$)", pps[0])
            if g1:
                gname = g1.group(1) or g1.group(2)
                depth, end_idx, defined_inside = 0, None, False
                for k, d in enumerate(pps):
                    kw = PP_LINE.match(d).group(1)
                    if kw in ("if", "ifdef", "ifndef"):
                        depth += 1
                    elif kw == "endif":
                        depth -= 1
                        if depth == 0:
                            end_idx = k
                            break
                    elif kw in ("elif", "else") and depth == 1:
                        break
                    elif kw == "define" and depth == 1 and \
                            re.match(r"^\s*#\s*define\s+%s(?!\w)" % re.escape(gname), d):
                        defined_inside = True
                if defined_inside and end_idx == len(pps) - 1:
                    self.guard = gname
        line_conds = {}
        guard_pending = self.guard is not None
        for s, ln in logical:
            m = PP_LINE.match(s)
            if m:
                d, rest = m.group(1), m.group(2).strip()
                if d in ("if", "ifdef", "ifndef"):
                    if d == "ifdef":
                        e = "defined(%s)" % rest.split()[0] if rest else "0"
                    elif d == "ifndef":
                        e = "!defined(%s)" % rest.split()[0] if rest else "0"
                    else:
                        e = rest
                    if guard_pending and not stack:
                        e = "1"
                        guard_pending = False
                    stack.append({"prev": [], "cur": e})
                elif d == "elif":
                    if stack:
                        fr = stack[-1]
                        fr["prev"].append(fr["cur"])
                        fr["cur"] = rest
                elif d == "else":
                    if stack:
                        fr = stack[-1]
                        fr["prev"].append(fr["cur"])
                        fr["cur"] = None
                elif d == "endif":
                    if stack:
                        stack.pop()
                elif d in ("define", "undef"):
                    mm = re.match(r"(\w+)(\([^)]*\))?\s*(.*)$", rest)
                    if mm:
                        val = mm.group(3).strip()
                        self.defines.append((mm.group(1), ln + 1, self._cond_tuple(stack), d, val,
                                             mm.group(2) is not None))
                cur = self._cond_tuple(stack)
                line_conds[ln] = self._cond_tuple(stack) if d not in ("if", "ifdef", "ifndef") else \
                    self._cond_tuple(stack[:-1])
                if d == "endif":
                    line_conds[ln] = cur
            else:
                cur = self._cond_tuple(stack)
                line_conds[ln] = cur
        last = ()
        for i in range(len(self.lines)):
            if i in line_conds:
                last = line_conds[i]
            self.cond_list[i + 1] = last
            self.cond[i + 1] = " && ".join("(%s)" % c for c in last)
        for m in INC_RE.finditer(self.clean):
            ln = self.clean.count("\n", 0, m.start()) + 1
            self.includes.append((m.group(1), m.group(2).strip(), ln))

    @staticmethod
    def _cond_tuple(stack):
        out = []
        for fr in stack:
            if fr["cur"] == "1" and not fr["prev"]:
                continue
            for p in fr["prev"]:
                out.append("!(%s)" % p)
            if fr["cur"] is not None:
                out.append(fr["cur"])
        return tuple(out)


# ------------------------------------------------------- #if evaluation

TOK_RE = re.compile(r"\s*(defined|0[xX][0-9a-fA-F]+[uUlL]*|\d+[uUlL]*|\w+|&&|\|\||==|!=|<=|>=|<<|>>|[!()<>+\-*/%~?:,])")


class Unknown(Exception):
    pass


def eval_pp(expr, defs, unknown_macros):
    """Tri-state: returns True, False or None (unknown)."""
    toks = []
    pos = 0
    expr = expr.strip()
    while pos < len(expr):
        m = TOK_RE.match(expr, pos)
        if not m:
            if expr[pos:].strip() == "":
                break
            return None
        toks.append(m.group(1))
        pos = m.end()
    idx = [0]

    def peek():
        return toks[idx[0]] if idx[0] < len(toks) else None

    def take():
        t = peek()
        idx[0] += 1
        return t

    U = object()

    def val_of(name):
        if name in defs:
            v = defs[name]
            return 1 if v is None else v
        if name in unknown_macros:
            return U
        return 0

    def primary():
        t = take()
        if t is None:
            raise Unknown()
        if t == "defined":
            par = False
            if peek() == "(":
                take()
                par = True
            name = take()
            if par:
                if take() != ")":
                    raise Unknown()
            if name in defs:
                return 1
            if name in unknown_macros:
                return U
            return 0
        if t == "(":
            v = ternary()
            if take() != ")":
                raise Unknown()
            return v
        if t == "!":
            v = primary()
            return U if v is U else int(not v)
        if t == "-":
            v = primary()
            return U if v is U else -v
        if t == "~":
            v = primary()
            return U if v is U else ~v
        if re.match(r"0[xX]", t):
            return int(re.sub(r"[uUlL]+$", "", t), 16)
        if t[0].isdigit():
            return int(re.sub(r"[uUlL]+$", "", t))
        if re.match(r"\w+$", t):
            if peek() == "(":   # function-like macro: unknown
                depth = 0
                while True:
                    x = take()
                    if x is None:
                        break
                    if x == "(":
                        depth += 1
                    elif x == ")":
                        depth -= 1
                        if depth == 0:
                            break
                return U
            return val_of(t)
        raise Unknown()

    def binop(sub, ops):
        def f():
            v = sub()
            while peek() in ops:
                op = take()
                w = sub()
                if op == "&&":
                    if v == 0 or w == 0:
                        v = 0
                    elif v is U or w is U:
                        v = U
                    else:
                        v = 1
                    continue
                if op == "||":
                    if (v is not U and v) or (w is not U and w):
                        v = 1
                    elif v is U or w is U:
                        v = U
                    else:
                        v = 0
                    continue
                if v is U or w is U:
                    v = U
                    continue
                try:
                    v = {"*": lambda a, b: a * b, "/": lambda a, b: a // b if b else 0,
                         "%": lambda a, b: a % b if b else 0, "+": lambda a, b: a + b,
                         "-": lambda a, b: a - b, "<<": lambda a, b: a << b, ">>": lambda a, b: a >> b,
                         "<": lambda a, b: int(a < b), ">": lambda a, b: int(a > b),
                         "<=": lambda a, b: int(a <= b), ">=": lambda a, b: int(a >= b),
                         "==": lambda a, b: int(a == b), "!=": lambda a, b: int(a != b)}[op](v, w)
                except Exception:
                    v = U
            return v
        return f

    mul = binop(primary, ("*", "/", "%"))
    add = binop(mul, ("+", "-"))
    sh = binop(add, ("<<", ">>"))
    rel = binop(sh, ("<", ">", "<=", ">="))
    eq = binop(rel, ("==", "!="))
    land = binop(eq, ("&&",))
    lor = binop(land, ("||",))

    def ternary():
        c = lor()
        if peek() == "?":
            take()
            a = ternary()
            if take() != ":":
                raise Unknown()
            b = ternary()
            if c is U:
                return U if a != b else a
            return a if c else b
        return c

    try:
        v = ternary()
        if idx[0] != len(toks):
            return None
    except Unknown:
        return None
    if v is U:
        return None
    return bool(v)


def eval_cond_tuple(conds, defs, unknown_macros):
    res = True
    for c in conds:
        v = eval_pp(c, defs, unknown_macros)
        if v is False:
            return False
        if v is None:
            res = None
    return res


# ----------------------------------------------------- dependency markers

def R(p, flags=0):
    return re.compile(p, flags)


MARKERS = [
    # category, kind, regex (applied per line on comment-stripped text)
    ("link", "pragma-lib", R(r"#\s*pragma\s+comment\s*\(\s*lib\s*,\s*\"([^\"]+)\"")),
    ("link", "pragma-link", R(r"#\s*pragma\s+link\s+\"([^\"]+)\"")),
    ("link", "pragma-package", R(r"#\s*pragma\s+package\s*\(([^)]*)\)")),
    ("link", "pragma-resource", R(r"#\s*pragma\s+resource\s+\"([^\"]+)\"")),
    ("dynload", "LoadLibrary", R(r"\b(LoadLibrary\w*|GetProcAddress|FreeLibrary|GetModuleHandle\w*)\s*\(")),
    ("http", "winhttp", R(r"\b(WinHttp\w+|WINHTTP_\w+)\b")),
    ("http", "wininet", R(r"\b(Internet(?:Open|Connect|ReadFile|CloseHandle|OpenUrl)\w*|HttpOpenRequest\w*|HttpSendRequest\w*|URLDownloadToFile\w*)\b")),
    ("http", "gethttp", R(r"\b(gethttp\w*|httpget\w*|HttpGet\w*|haehttp\w*)\s*\(")),
    ("browser", "vcl-browser", R(r"\b(TWebBrowser|TEdgeBrowser|ICoreWebView2\w*|IWebBrowser2|IHTMLDocument\w*)\b")),
    ("browser", "navigate", R(r"(?:->|\.)\s*(Navigate2?)\s*\(")),
    ("socket", "winsock", R(r"\b(WSAStartup|WSACleanup|WSAGetLastError|WSAAsyncSelect|WSAEventSelect|WSAIoctl|socket|connect|bind|listen|accept|recv|recvfrom|send|sendto|closesocket|shutdown|gethostbyname|gethostname|getaddrinfo|freeaddrinfo|inet_addr|inet_ntoa|inet_pton|htons|ntohs|htonl|ntohl|ioctlsocket|setsockopt|getsockopt|select)\s*\(")),
    ("serial", "comm-api", R(r"\b(SetCommState|GetCommState|BuildCommDCB\w*|SetCommTimeouts|GetCommTimeouts|PurgeComm|EscapeCommFunction|ClearCommError|SetupComm|WaitCommEvent|SetCommMask|GetCommModemStatus|TransmitCommChar|SetCommBreak|ClearCommBreak)\s*\(")),
    ("serial", "comport-name", R(r"\"(?:\\\\\\\\\.\\\\)?COM\d*|L\"(?:\\\\\\\\\.\\\\)?COM|\\\\\\\\\.\\\\COM")),
    ("usb", "setupapi", R(r"\b(SetupDi\w+|HidD_\w+|FT_\w+)\s*\(")),
    ("file-api", "CreateFile", R(r"\b(CreateFile[AW]?)\s*\(")),
    ("registry", "reg-api", R(r"\b(Reg(?:Open|Create|Query|Set|Close|Delete|Enum)\w*|TRegistry|TRegIniFile|HKEY_\w+)\b")),
    ("ini", "ini-api", R(r"\b((?:Get|Write)PrivateProfile\w*|(?:Get|Write)Profile\w*|TIniFile|TMemIniFile|TCustomIniFile)\b")),
    ("database", "dbx", R(r"\b(TSQLConnection|TSQLQuery|TSQLDataSet|TSQLTable|TSQLStoredProc|TSimpleDataSet|TClientDataSet|TDataSetProvider|TDBXConnection\w*|TDBX\w+|TFDConnection|TFDQuery|TADOConnection|TADOQuery|TDataSource|TSQLTransaction|TTransactionDesc)\b")),
    ("database", "dbx-call", R(r"(?:->|\.)\s*(ExecSQL|ExecuteDirect|StartTransaction|Commit|Rollback|CommitFreeAndNil|RollbackFreeAndNil|GetTableNames|ParamByName|Params|DriverName|LibraryName|VendorLib|GetDriverFunc|ConnectionName|LoadParamsFromIniFile|LoadParamsOnConnect)\b")),
    ("database", "mysql-c", R(r"\b(mysql_\w+|MYSQL\w*)\b")),
    ("database", "odbc", R(r"\b(SQLConnect|SQLDriverConnect|SQLAllocHandle|SQLExecDirect|SQLAllocEnv)\s*\(")),
    ("sql-text", "sql", R(r"(?i)\"[^\"]*\b(CREATE\s+TABLE|INSERT\s+INTO|REPLACE\s+INTO|UPDATE\s+\w+\s+SET|SELECT\s+.+\s+FROM|DELETE\s+FROM|DROP\s+TABLE|ALTER\s+TABLE|SHOW\s+TABLES|CREATE\s+DATABASE|USE\s+\w+)\b")),
    ("shell", "process", R(r"\b(ShellExecute\w*|WinExec|CreateProcess\w*|_wsystem|_wspawn\w*|_spawn\w*|system)\s*\(")),
    ("shell", "shell-folder", R(r"\b(SHGetFolderPath\w*|SHGetSpecialFolder\w*|SHGetKnownFolderPath|SHBrowseForFolder\w*|SHFileOperation\w*|SHCreateDirectory\w*|SelectDirectory)\s*\(")),
    ("com", "ole", R(r"\b(CoInitialize\w*|CoCreateInstance\w*|CoUninitialize|CreateOleObject|GetActiveOleObject|OleVariant|OleInitialize|CoTaskMemFree)\b")),
    ("xml", "xml-lib", R(r"\b(TXMLDocument|_di_IXMLDocument|LoadXMLDocument|IXMLDOMDocument\w*|MSXML\w*|xmlrtl)\b")),
    ("multimedia", "sound-timer", R(r"\b(PlaySound\w*|sndPlaySound\w*|MessageBeep|Beep|timeBeginPeriod|timeEndPeriod|timeGetTime|timeSetEvent|mciSendString\w*)\s*\(")),
    ("printing", "print-api", R(r"\b(OpenPrinter\w*|ClosePrinter|StartDoc\w*|EndDoc|StartPage|EndPage|WritePrinter|StartDocPrinter\w*|EnumPrinters\w*|GetDefaultPrinter\w*|PrintDlg\w*|DocumentProperties\w*|CreateDC\w*|TPrinter|Printer\s*\(\s*\))")),
    ("clipboard", "clipboard", R(r"\b(OpenClipboard|SetClipboardData|GetClipboardData|Clipboard\s*\(\s*\))")),
    ("time", "time-api", R(r"\b(QueryPerformanceCounter|QueryPerformanceFrequency|GetSystemTimeAsFileTime|GetTimeZoneInformation|SetSystemTime|SetLocalTime|GetTickCount64|GetTickCount)\s*\(")),
    ("thread-api", "win-thread", R(r"\b(_beginthread\w*|CreateThread|TThread)\b")),
    ("url", "url-literal", R(r"(?i)\"[^\"]*\b((?:https?|ftp|wss?)://[^\"\s]*)")),
    ("url", "ip-literal", R(r"\"(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})(?::\d+)?\"")),
    ("device", "LID", R(r"\b(LID_\w+)\b")),
    ("device", "reader-class", R(r"\b(SiritReader|ZebraReader|FeigReader\w*|TSiritReader|TZebraReader|emitag\w*|EmiTag\w*|EMITAG)\b")),
    ("network-transport", "comtp", R(r"\b(IP_\w+|ipTCP\w*|ipUDP\w*|ipXML\w*|ipLOKI\w*)\b")),
    ("crypto", "crypt-api", R(r"\b(Crypt\w+|BCrypt\w+|Cert\w+Store\w*|SECURITY_FLAG_\w+)\b")),
]
SYS_HEADER_GROUPS = [
    ("dbexpress", R(r"(?i)(sqlexpr|dbxmysql|data\.sqlexpr|data\.dbx\w*|dbxcommon|dbxdbreaders|simpleds|dbclient|provider|datasnap|db\.hpp|data\.db\.hpp|data\.fmtbcd|fmtbcd|sqltimst)")),
    ("browser", R(r"(?i)(shdocvw|oleserver|vcl\.edge|webview2|mshtml|vcl\.olectrls)")),
    ("winsock", R(r"(?i)^(winsock2?\.h|ws2tcpip\.h|iphlpapi\.h|mswsock\.h)$")),
    ("winhttp", R(r"(?i)^(winhttp\.h|wininet\.h|urlmon\.h)$")),
    ("windows", R(r"(?i)^(windows\.h|winbase\.h|winuser\.h|wingdi\.h|winspool\.h|commdlg\.h|commctrl\.h|shellapi\.h|shlobj\.h|shlwapi\.h|mmsystem\.h|setupapi\.h|hidsdi\.h|objbase\.h|ole2\.h|process\.h|io\.h|direct\.h|dos\.h|conio\.h|share\.h|sys/.*|tchar\.h|locale\.h|time\.h|signal\.h)$")),
    ("vcl", R(r"(?i)^(vcl|vcl\.\w+|forms|controls|stdctrls|extctrls|comctrls|dialogs|menus|buttons|grids|graphics|classes|sysutils|system\.\w+|vcl/.*|.*\.hpp)$")),
    ("crt", R(r"(?i)^(std\w*\.h|string\.h|stdlib\.h|stdio\.h|ctype\.h|wchar\.h|wctype\.h|math\.h|malloc\.h|memory\.h|limits\.h|float\.h|errno\.h|fcntl\.h|assert\.h|search\.h|mem\.h|alloc\.h|setjmp\.h|stdarg\.h|stddef\.h|iostream|fstream|sstream|string|vector|map|algorithm|list|set|cstdio|cstdlib|cstring|new|exception|stdexcept|typeinfo)$")),
]


def header_group(name):
    for g, rx in SYS_HEADER_GROUPS:
        if rx.search(name):
            return g
    return "other"


# --------------------------------------------------------------- DFM

DFM_OBJ = re.compile(r"^\s*(object|inherited|inline)\s+(\w+)?\s*:\s*(\w+)", re.M)
DFM_PROPS_OF_INTEREST = {"DriverName", "LibraryName", "VendorLib", "GetDriverFunc", "ConnectionName",
                         "LoginPrompt", "Params.Strings", "Connected", "SQL.Strings", "CommandText",
                         "ParamCheck", "KeepConnection", "Interval", "Enabled", "Active", "FileName",
                         "TableName", "ConnectionTimeout"}


def parse_dfm(path):
    data = open(path, "rb").read()
    if data[:1] == b"\xff":
        return None, "binary"
    txt = data.decode("utf-8", errors="replace") if data.startswith(b"\xef\xbb\xbf") else data.decode("cp1252", errors="replace")
    objs = []
    lines = txt.split("\n")
    stack = []
    for i, line in enumerate(lines, 1):
        m = re.match(r"^\s*(object|inherited|inline)\s+(?:(\w+)\s*)?:\s*(\w+)", line)
        if m:
            stack.append((m.group(2) or "", m.group(3), i))
            objs.append({"name": m.group(2) or "", "cls": m.group(3), "line": i, "props": {}})
            continue
        if re.match(r"^\s*end\s*$", line) and stack:
            stack.pop()
            continue
        pm = re.match(r"^\s*([\w.]+)\s*=\s*(.*)$", line)
        if pm and objs and stack:
            key = pm.group(1)
            if key in DFM_PROPS_OF_INTEREST:
                # find the object for the current stack top
                top = stack[-1]
                for o in reversed(objs):
                    if o["line"] == top[2]:
                        val = pm.group(2).strip()
                        if val in ("(", "<"):
                            # collect following item lines until ')' / '>'
                            j = i
                            coll = []
                            while j < len(lines):
                                s = lines[j].strip()
                                j += 1
                                if s in (")", ">") or s.endswith(")") and not s.startswith("'"):
                                    coll.append(s.rstrip(")"))
                                    break
                                coll.append(s)
                            val = " | ".join(x for x in coll if x)
                        o["props"][key] = val[:300]
                        break
    return objs, "text"


# --------------------------------------------------------------- main

def build_model(root, cfg_override=None):
    """Evaluate projects, include closures and macro sets. Returns a namespace.
    cfg_override: {owner: configuration} to evaluate another configuration
    (C-12 valitsimet.py); None = shipped configurations as in C-10."""
    cfg_override = cfg_override or {}
    src = norm(os.path.join(root, "TPsource", "V52"))

    # all source files under V52
    all_files = {}
    by_base = defaultdict(list)
    for dp, dns, fns in os.walk(src):
        dns[:] = [d for d in dns if d not in (".git", "obj", "Win32", "__history", "build-logs", "Release", "Asan")]
        for fn in fns:
            if fn.lower().endswith((".cpp", ".c", ".h", ".hpp")):
                p = norm(os.path.join(dp, fn))
                all_files[p.lower()] = p
                by_base[fn.lower()].append(p)

    owners = {}   # owner name -> info
    for name, comp, rel, cfg, libs in PRODUCTS:
        owners[name] = project_info(src, comp, rel, cfg_override.get(name, cfg))
        owners[name]["kind"] = "product"
        owners[name]["libs"] = libs
    for name, (comp, rel, cfg) in LIBS.items():
        owners[name] = project_info(src, comp, rel, cfg_override.get(name, cfg))
        owners[name]["kind"] = "library"
        owners[name]["libs"] = []

    # product -> owners making it up
    product_owners = {p[0]: [p[0]] + p[4] for p in PRODUCTS}

    cache = {}

    def finfo(p):
        if p not in cache:
            try:
                cache[p] = FileInfo(p, read_text(p))
            except OSError:
                cache[p] = None
        return cache[p]

    unresolved = Counter()
    ambiguous = Counter()

    def resolve(inc, from_dir, inc_dirs, quoted):
        cands = []
        if quoted:
            cands.append(norm(os.path.join(from_dir, inc)))
        for d in inc_dirs:
            cands.append(norm(os.path.join(d, inc)))
        for c in cands:
            if c.lower() in all_files:
                return all_files[c.lower()]
        base = inc.replace("\\", "/").split("/")[-1].lower()
        lst = by_base.get(base, [])
        if len(lst) == 1:
            return lst[0]
        if len(lst) > 1:
            ambiguous[(inc, from_dir)] += 1
            # prefer one under include/
            for x in lst:
                if "/include/" in x.lower():
                    return x
            return None
        return None

    # macro definitions across the repo (every repo source and header)
    for p in list(all_files.values()):
        finfo(p)
    macro_defs = defaultdict(list)  # name -> [(file, line, cond, kind, value, fnlike)]
    for p, fi in cache.items():
        if not fi:
            continue
        for name, ln, cond, kind, val, fnlike in fi.defines:
            macro_defs[name].append((p, ln, cond, kind, val, fnlike))

    def owner_defs(oname, files):
        """Known macro values and unknown macro names for one owner, given its
        include closure. files=None: project defines only, every other macro
        that is #defined anywhere in the repo is unknown (first pass)."""
        info = owners[oname]
        d = define_map(info["defines"], info["compiler"], oname)
        unknown = set()
        pending = {}
        for name, lst in macro_defs.items():
            if name in d:
                continue
            if files is None:
                unknown.add(name)
                continue
            # a #define in a .c/.cpp file is local to that translation unit
            in_scope = [x for x in lst if x[0] in files and not x[0].lower().endswith(SRC_EXT)]
            if not in_scope:
                continue
            pending[name] = in_scope
        # conditional #defines: evaluate their own condition with what is known;
        # "#ifndef X / #define X v" (default value) counts as defined when X is not
        # set by the project. Iterate until stable.
        for _ in range(4):
            changed = False
            for name, in_scope in pending.items():
                if name in d:
                    continue
                if any(x[3] == "undef" for x in in_scope):
                    if name not in unknown:
                        unknown.add(name)
                        changed = True
                    continue
                state = False
                val = None
                for x in in_scope:
                    conds = tuple(c for c in x[2] if c.replace(" ", "") != "!defined(%s)" % name)
                    v = eval_cond_tuple(conds, d, unknown | set(n for n in pending if n not in d and n != name and n not in unknown))
                    if v is True:
                        state, val = True, x[4]
                        break
                    if v is None:
                        state = None
                if state is True:
                    try:
                        d[name] = int(re.sub(r"[uUlL]+$", "", val), 0) if val else 1
                    except ValueError:
                        d[name] = 1
                    unknown.discard(name)
                    changed = True
                elif state is None and name not in unknown:
                    unknown.add(name)
                    changed = True
            if not changed:
                break
        if oname in ("Tputil-XE", "DBboxm-XE"):
            unknown.add("WIN32")
        return d, unknown

    def local_defs(fpath, defs, unk):
        """Add the unconditional #defines of a .c/.cpp file itself (TU-local);
        its conditional #defines and #undefs make the name unknown."""
        if not fpath.lower().endswith(SRC_EXT):
            return defs, unk
        fi = cache.get(fpath)
        if not fi or not fi.defines:
            return defs, unk
        d2, u2 = dict(defs), set(unk)
        for name, ln, cond, kind, val, fnlike in fi.defines:
            if kind == "define" and not cond and name not in d2:
                try:
                    d2[name] = int(re.sub(r"[uUlL]+$", "", val), 0) if val else 1
                except ValueError:
                    d2[name] = 1
            elif name not in defs:
                u2.add(name)
                d2.pop(name, None)
        return d2, u2

    def closure(oname, defs, unk, record):
        """Include closure; an #include whose condition is false for this owner
        is not followed (unknown conditions are followed)."""
        info = owners[oname]
        files = {}
        for s in info["sources"]:
            sp = all_files.get(s.lower())
            if sp:
                files[sp] = "compiled"
        todo = list(files)
        seen = set(todo)
        while todo:
            f = todo.pop()
            fi = finfo(f)
            if not fi:
                continue
            ld, lu = local_defs(f, defs, unk)
            for q, inc, ln in fi.includes:
                if eval_cond_tuple(fi.cond_list[ln], ld, lu) is False:
                    continue
                r = resolve(inc, os.path.dirname(f), info["include_dirs"], q == '"')
                if r is None:
                    if q == '"' and record:
                        unresolved[(oname, inc)] += 1
                    continue
                if r not in seen:
                    seen.add(r)
                    files.setdefault(r, "included")
                    todo.append(r)
        return files

    owner_files = {}
    odefs = {}
    tu_rows = []
    for oname, info in owners.items():
        d, unk = owner_defs(oname, None)
        files = closure(oname, d, unk, False)
        for _ in range(3):   # iterate until the closure is stable
            d, unk = owner_defs(oname, files)
            nfiles = closure(oname, d, unk, False)
            if set(nfiles) == set(files):
                break
            files = nfiles
        files = closure(oname, d, unk, True)
        owner_files[oname] = files
        odefs[oname] = (d, unk)
        info["missing_sources"] = [s for s in info["sources"] if s.lower() not in all_files]
        for s in info["sources"]:
            tu_rows.append((oname, info["compiler"], os.path.relpath(s, src).replace("\\", "/"),
                            "yes" if s.lower() in all_files else "MISSING"))

    # file -> owners
    file_owners = defaultdict(dict)
    for o, files in owner_files.items():
        for f, how in files.items():
            file_owners[f][o] = how

    pnames = [p[0] for p in PRODUCTS]

    def reach(fpath, ln):
        """per product: '' (not in), 'C:y', 'I:n', ..."""
        fi = cache.get(fpath)
        conds = fi.cond_list[ln] if fi else ()
        res = {}
        for p in pnames:
            parts = []
            for o in product_owners[p]:
                how = owner_files[o].get(fpath)
                if not how:
                    continue
                d, unk = local_defs(fpath, *odefs[o])
                v = eval_cond_tuple(conds, d, unk)
                parts.append("%s%s:%s" % (o if o != p else "", "C" if how == "compiled" else "I",
                                          {True: "y", False: "n", None: "?"}[v]))
            res[p] = ";".join(parts)
        return res

    def summarize(r):
        """yes if any part active, unknown if any '?', no if present but inactive, '' absent"""
        if not r:
            return ""
        if ":y" in r:
            return "yes"
        if ":?" in r:
            return "unknown"
        return "no"

    from types import SimpleNamespace
    return SimpleNamespace(**locals())


def main():
    root, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    m = build_model(root)
    src, cache, owners, owner_files, odefs = m.src, m.cache, m.owners, m.owner_files, m.odefs
    file_owners, pnames, reach, summarize = m.file_owners, m.pnames, m.reach, m.summarize
    by_base, all_files, product_owners = m.by_base, m.all_files, m.product_owners
    unresolved, ambiguous, tu_rows = m.unresolved, m.ambiguous, m.tu_rows

    # ---- scan markers
    rows = []
    scan_files = sorted(set(cache.keys()))
    for fpath in scan_files:
        fi = cache[fpath]
        if not fi:
            continue
        rel = os.path.relpath(fpath, src).replace("\\", "/")
        if rel.startswith("Tests/") or rel.startswith("VS/"):
            continue
        for i, line in enumerate(fi.lines, 1):
            if not line.strip():
                continue
            m = INC_RE.match(line)
            if m and m.group(1) == "<":
                name = m.group(2).strip()
                if norm(name).lower().split("/")[-1] in by_base and header_group(name) == "other":
                    grp = "repo-angle"
                else:
                    grp = header_group(name)
                rows.append((rel, i, "sys-include", grp, name, fi.cond[i], reach(fpath, i),
                             fi.raw_lines[i - 1].strip()[:200]))
                continue
            for cat, kind, rx in MARKERS:
                for mm in rx.finditer(line):
                    tok = mm.group(1) if mm.groups() else mm.group(0)
                    rows.append((rel, i, cat, kind, tok, fi.cond[i], reach(fpath, i),
                                 fi.raw_lines[i - 1].strip()[:200]))

    with open(os.path.join(out, "osumat.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["file", "line", "category", "kind", "token", "condition"] +
                   ["%s" % p for p in pnames] + ["%s_active" % p for p in pnames] + ["text"])
        for rel, ln, cat, kind, tok, cond, r, text in rows:
            w.writerow([rel, ln, cat, kind, tok, cond] + [r[p] for p in pnames] +
                       [summarize(r[p]) for p in pnames] + [text])

    # ---- products table
    with open(os.path.join(out, "tuotteet.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["owner", "kind", "compiler", "project", "config", "toolset", "runtime", "charset",
                    "subsystem", "defines", "include_dirs", "link_libs", "packages_import",
                    "link_packages", "link_package_statics", "all_package_libs", "lib_files",
                    "n_sources", "missing_sources", "n_closure_files"])
        for o, info in owners.items():
            w.writerow([o, info["kind"], info["compiler"], info["project"], info.get("config", ""),
                        info.get("toolset", ""), info.get("runtime", ""), info.get("charset", ""),
                        info.get("subsystem", ""), ";".join(info["defines"]),
                        ";".join(os.path.relpath(d, src).replace("\\", "/") for d in info["include_dirs"]),
                        ";".join(info["link_libs"]), ";".join(info.get("packages", [])),
                        ";".join(info.get("link_packages", [])),
                        ";".join(info.get("link_package_statics", [])),
                        ";".join(info.get("all_package_libs", [])),
                        ";".join(info.get("lib_files", [])), len(info["sources"]),
                        ";".join(info["missing_sources"]), len(owner_files[o])])

    with open(os.path.join(out, "kaannosyksikot.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["owner", "compiler", "source", "exists"])
        w.writerows(tu_rows)

    with open(os.path.join(out, "tiedostot.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["file"] + list(owners))
        for fp in sorted(file_owners):
            w.writerow([os.path.relpath(fp, src).replace("\\", "/")] +
                       [file_owners[fp].get(o, "") for o in owners])

    # macros: switch values per owner
    with open(os.path.join(out, "makrot.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["owner", "defined_known", "unknown"])
        for o in owners:
            d, unk = odefs[o]
            w.writerow([o, ";".join("%s=%s" % (k, v) for k, v in sorted(d.items())), ";".join(sorted(unk))])

    # ---- DFM
    dfm_rows = []
    dfm_classes = defaultdict(Counter)
    binaries = []
    for dp, dns, fns in os.walk(src):
        dns[:] = [d for d in dns if d not in (".git", "obj", "Win32", "__history")]
        for fn in fns:
            if not fn.lower().endswith(".dfm"):
                continue
            p = norm(os.path.join(dp, fn))
            objs, kind = parse_dfm(p)
            cpp = all_files.get(p[:-4].lower() + ".cpp")
            prods = []
            if cpp:
                for pn in pnames:
                    if any(owner_files[o].get(cpp) == "compiled" for o in product_owners[pn]):
                        prods.append(pn)
            rel = os.path.relpath(p, src).replace("\\", "/")
            if objs is None:
                binaries.append(rel)
                dfm_rows.append((rel, "", "", 0, "BINARY", ";".join(prods), ""))
                continue
            for o in objs:
                dfm_rows.append((rel, o["name"], o["cls"], o["line"], kind, ";".join(prods),
                                 " ; ".join("%s=%s" % kv for kv in o["props"].items())))
                for pn in prods:
                    dfm_classes[pn][o["cls"]] += 1
    with open(os.path.join(out, "dfm.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["dfm", "object", "class", "line", "format", "products", "props"])
        w.writerows(dfm_rows)

    # ---- summary
    with open(os.path.join(out, "yhteenveto.txt"), "w", encoding="utf-8") as f:
        f.write("C-10 part A dependency inventory (riippuvuudet.py)\n\n")
        for o, info in owners.items():
            f.write("%s [%s, %s] %s config=%s sources=%d closure=%d missing=%d\n" % (
                o, info["kind"], info["compiler"], info["project"], info.get("config", ""),
                len(info["sources"]), len(owner_files[o]), len(info["missing_sources"])))
        f.write("\nunresolved quoted includes (name: count): %s\n" % dict(unresolved.most_common(40)))
        f.write("ambiguous includes resolved/unresolved: %d\n" % len(ambiguous))
        for (inc, frm), n in sorted(ambiguous.items())[:60]:
            f.write("  %s from %s\n" % (inc, os.path.relpath(frm, src)))
        f.write("\nmarker rows: %d\n" % len(rows))
        cnt = Counter((r[2], r[3]) for r in rows)
        for k, v in sorted(cnt.items()):
            f.write("  %-18s %-16s %d\n" % (k[0], k[1], v))
        f.write("\nper product, active rows by category (yes/unknown):\n")
        for p in pnames:
            c = Counter()
            for r in rows:
                s = summarize(r[6][p])
                if s in ("yes", "unknown"):
                    c[(r[2], s)] += 1
            f.write("  %s: %s\n" % (p, ", ".join("%s/%s=%d" % (k[0], k[1], v) for k, v in sorted(c.items()))))
        f.write("\nDFM files: %d rows=%d binary=%d %s\n" % (
            len(set(r[0] for r in dfm_rows)), len(dfm_rows), len(binaries), binaries))
        for p in pnames:
            f.write("  %s classes: %s\n" % (p, ", ".join("%s=%d" % kv for kv in sorted(dfm_classes[p].items()))))


if __name__ == "__main__":
    main()
