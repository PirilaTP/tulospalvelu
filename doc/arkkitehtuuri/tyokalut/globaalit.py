#!/usr/bin/env python3
"""C-8 part A: mechanical inventory of global and static variables.

Scans the core directories of TPsource/V52, finds
  * namespace-scope variable definitions (non-extern) per file,
  * function-local `static` variables,
and then every reference to each global name in the scanned directories,
attributed to the enclosing function, with a heuristic access kind:
  W  = assignment / ++ / -- on the name (possibly via member or index),
  A  = address taken or passed to a known writing call (memset, wcscpy, ...),
  R  = anything else.
The heuristics are lexical (no preprocessor, no type info). Output is raw
evidence, not a verified result.

Usage: python -I globaalit.py <repo-root> <out-dir>
Writes: maarittelyt.csv, staattiset_funktiossa.csv, viittaukset.csv, yhteenveto.txt
"""
import csv
import os
import re
import sys
from collections import defaultdict

CORE_DIRS = ["Hk", "Juk", "Tp", "com", "cbTp", "tputilv2"]
# References are also counted from GUI dirs so writers there are visible.
REF_EXTRA_DIRS = ["cbHk", "ViestiWin", "HkConsole", "JukConsole", "include"]
SRC_EXT = (".cpp", ".c", ".h")

KEYWORDS = set("""
if else for while do switch case default return break continue goto sizeof
struct class union enum typedef extern static const volatile register auto
unsigned signed int char short long float double void bool wchar_t
public private protected virtual inline operator new delete this template
typename namespace using try catch throw __fastcall __stdcall __cdecl
__published __property __closure friend mutable explicit
""".split())

WRITE_CALLS = set("""
memset memcpy memmove wcscpy wcsncpy strcpy strncpy wcscat strcat strncat wcsncat
swprintf sprintf snwprintf _snwprintf wcscpy_s strcpy_s ZeroMemory fread fgets fgetws
wmemset wmemcpy ansitowcs wcstoansi
""".split())


def read_text(path):
    with open(path, "rb") as f:
        data = f.read()
    return data.decode("utf-8", errors="replace") if data.startswith(b"\xef\xbb\xbf") \
        else data.decode("cp1252", errors="replace")


PP_RE = re.compile(r"^[ \t]*#[ \t]*(if|ifdef|ifndef|elif|else|endif)\b(.*)$", re.M)


def drop_else_branches(text):
    """Keep only the first branch of every #if chain (and drop #if 0 bodies)
    so that braces stay balanced when both branches open a block. Lines are
    blanked, not removed, to keep line numbers."""
    lines = text.split("\n")
    stack = []  # each: [active_before, keep_current]
    for i, line in enumerate(lines):
        m = PP_RE.match(line)
        active = all(k for _, k in stack)
        if m:
            d = m.group(1)
            if d in ("if", "ifdef", "ifndef"):
                cond = m.group(2).split("//")[0].strip()
                stack.append([active, not (d == "if" and cond == "0")])
            elif d in ("elif", "else") and stack:
                stack[-1][1] = False
            elif d == "endif" and stack:
                stack.pop()
            continue
        if not active:
            lines[i] = ""
    return "\n".join(lines)


def strip_code(text):
    """Blank out comments, string and char literals and preprocessor lines,
    keeping line structure and offsets."""
    text = drop_else_branches(text)
    out = list(text)
    i, n = 0, len(text)
    line_start = True
    while i < n:
        c = text[i]
        if line_start and c in " \t":
            i += 1
            continue
        if line_start and c == "#":
            # preprocessor line incl. continuations
            j = i
            while j < n:
                if text[j] == "\n" and text[j - 1] != "\\":
                    break
                j += 1
            for k in range(i, j):
                if out[k] != "\n":
                    out[k] = " "
            i = j
            continue
        line_start = False
        if c == "\n":
            line_start = True
            i += 1
            continue
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            for k in range(i, j):
                out[k] = " "
            i = j
            continue
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                if out[k] != "\n":
                    out[k] = " "
            i = j
            continue
        if c in "\"'":
            q = c
            j = i + 1
            while j < n and text[j] != q and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            for k in range(i + 1, min(j, n)):
                out[k] = " "
            i = j + 1
            continue
        i += 1
    return "".join(out)


TOKEN_RE = re.compile(r"[A-Za-z_]\w*|\d[\w.]*|->|\+\+|--|<<=|>>=|[+\-*/%&|^!=<>]=|::|&&|\|\||\S")


def tokenize(code):
    line = 1
    pos = 0
    for m in TOKEN_RE.finditer(code):
        line += code.count("\n", pos, m.start())
        pos = m.start()
        yield m.group(), line


def split_declarators(tokens):
    """Given tokens of one declaration statement (without ';'), return
    declared variable names. Returns [] for function prototypes/typedefs."""
    if not tokens:
        return []
    first = tokens[0][0]
    if first in ("typedef", "extern", "using", "template", "namespace", "friend"):
        return []
    # struct/class/union/enum with body: names after the closing brace
    names = []
    depth_paren = depth_brace = depth_brack = 0
    cur = []
    groups = []
    for tok, ln in tokens:
        if tok == "(":
            depth_paren += 1
        elif tok == ")":
            depth_paren -= 1
        elif tok == "{":
            depth_brace += 1
        elif tok == "}":
            depth_brace -= 1
        elif tok == "[":
            depth_brack += 1
        elif tok == "]":
            depth_brack -= 1
        if tok == "," and depth_paren == 0 and depth_brace == 0 and depth_brack == 0:
            groups.append(cur)
            cur = []
        else:
            cur.append((tok, ln, depth_paren, depth_brace, depth_brack))
    groups.append(cur)
    if first in ("struct", "class", "union", "enum") and len(tokens) == 2:
        return []  # forward declaration "struct X;"
    for gi, g in enumerate(groups):
        # for aggregate definitions keep only tokens after the closing brace
        if gi == 0 and any(t[0] == "}" for t in g):
            last = max(ix for ix, t in enumerate(g) if t[0] == "}")
            g = g[last + 1:]
        # take tokens before '=' at depth 0
        head = []
        for t in g:
            if t[0] == "=" and t[2] == 0 and t[3] == 0 and t[4] == 0:
                break
            head.append(t)
        # skip anything inside a struct body for the first group
        cand = None
        for idx, (tok, ln, dp, db, dk) in enumerate(head):
            if db != 0 or dk != 0 or tok == "}":
                continue
            if re.match(r"[A-Za-z_]\w*$", tok) and tok not in KEYWORDS:
                nxt = head[idx + 1][0] if idx + 1 < len(head) else None
                if dp > 0:
                    # (*name)[..] function pointer / array pointer
                    prev = head[idx - 1][0] if idx > 0 else None
                    if prev == "*" and nxt == ")":
                        cand = (tok, ln)
                    continue
                if nxt == "(" and gi == 0 and not any(x[0] == "=" for x in g):
                    # function prototype or constructor-style init: treat
                    # "Type name(args)" with no body as prototype
                    cand = ("__FUNC__", ln)
                    break
                cand = (tok, ln)
        if cand and cand[0] != "__FUNC__":
            names.append(cand)
        elif cand and cand[0] == "__FUNC__":
            return []
    return names


def scan_file(path, rel):
    """Return (globals, local_statics, functions) for one file.
    globals: list of (name, line, is_static, decl_text)
    local_statics: list of (name, line, function)
    functions: list of (name, start_line, end_line)"""
    code = strip_code(read_text(path))
    toks = list(tokenize(code))
    globals_, lstat, funcs = [], [], []
    depth = 0
    stmt = []           # tokens of current namespace-scope statement
    func_name = None
    func_start = None
    func_depth = None
    agg_depth = 0       # depth of struct/enum bodies at namespace scope
    i = 0
    local_stmt = []
    while i < len(toks):
        tok, ln = toks[i]
        if func_name is None:
            if tok == "{":
                # function body? previous tokens end with ')' [const] [: init]
                is_func = False
                j = len(stmt) - 1
                while j >= 0 and stmt[j][0] in ("const", "throw", "override"):
                    j -= 1
                # skip ctor initialiser list
                k = j
                paren = 0
                name = None
                if j >= 0 and stmt[j][0] == ")":
                    # find matching '('
                    while k >= 0:
                        if stmt[k][0] == ")":
                            paren += 1
                        elif stmt[k][0] == "(":
                            paren -= 1
                            if paren == 0:
                                break
                        k -= 1
                    if k > 0 and re.match(r"[A-Za-z_~]\w*$", stmt[k - 1][0]) \
                            and stmt[k - 1][0] not in ("if", "while", "for", "switch") \
                            and not any(t[0] == "=" for t in stmt[:k]):
                        name = stmt[k - 1][0]
                        if k > 2 and stmt[k - 2][0] == "::":
                            name = stmt[k - 3][0] + "::" + name
                        is_func = True
                    elif k > 0 and any(t[0] == ":" for t in stmt):
                        pass
                # constructor initialiser lists: Foo::Foo(...) : a(1), b(2) {
                if not is_func and any(t[0] == ":" for t in stmt):
                    colon = max(ix for ix, t in enumerate(stmt) if t[0] == ":")
                    head = stmt[:colon]
                    if head and head[-1][0] == ")":
                        p = 0
                        k = len(head) - 1
                        while k >= 0:
                            if head[k][0] == ")":
                                p += 1
                            elif head[k][0] == "(":
                                p -= 1
                                if p == 0:
                                    break
                            k -= 1
                        if k > 0 and re.match(r"[A-Za-z_~]\w*$", head[k - 1][0]):
                            name = head[k - 1][0]
                            if k > 2 and head[k - 2][0] == "::":
                                name = head[k - 3][0] + "::" + name
                            is_func = True
                if is_func and agg_depth == 0:
                    func_name = name
                    func_start = ln
                    func_depth = depth
                    depth += 1
                    stmt = []
                    local_stmt = []
                    i += 1
                    continue
                if any(t[0] in ("struct", "class", "union", "enum") for t in stmt) \
                        and not any(t[0] == "=" for t in stmt):
                    agg_depth += 1
                    stmt.append((tok, ln))
                    depth += 1
                    i += 1
                    continue
                if stmt and stmt[0][0] == "extern":
                    # extern "C" { ... } -> transparent
                    stmt = []
                    i += 1
                    continue
                stmt.append((tok, ln))
                depth += 1
            elif tok == "}":
                depth -= 1
                if agg_depth > 0:
                    agg_depth -= 1
                stmt.append((tok, ln))
                if depth <= 0 and agg_depth == 0 and not any(t[0] == "=" for t in stmt):
                    # closing of extern "C"/namespace block: reset
                    if not any(t[0] in ("struct", "class", "union", "enum") for t in stmt):
                        stmt = []
                depth = max(depth, 0)
            elif tok == ";" and agg_depth == 0 and _balanced(stmt):
                is_static = any(t[0] == "static" for t in stmt[:3])
                for name, nl in split_declarators(stmt):
                    text = " ".join(t[0] for t in stmt)[:160]
                    globals_.append((name, nl, is_static, text))
                stmt = []
            else:
                stmt.append((tok, ln))
        else:
            if tok == "{":
                depth += 1
            elif tok == "}":
                depth -= 1
                if depth == func_depth:
                    funcs.append((func_name, func_start, ln))
                    func_name = None
                    stmt = []
                    i += 1
                    continue
            if tok in (";", "{", "}"):
                if local_stmt and local_stmt[0][0] == "static":
                    for name, nl in split_declarators(local_stmt[1:] if False else local_stmt):
                        lstat.append((name, nl, func_name))
                local_stmt = []
            else:
                local_stmt.append((tok, ln))
        i += 1
    return globals_, lstat, funcs


def _balanced(stmt):
    p = b = 0
    for t, _ in stmt:
        if t == "(":
            p += 1
        elif t == ")":
            p -= 1
        elif t == "{":
            b += 1
        elif t == "}":
            b -= 1
    return p == 0 and b == 0


def access_kind(toks, idx):
    """Heuristic access kind for the name token at toks[idx]."""
    prev = toks[idx - 1][0] if idx > 0 else ""
    if prev in ("++", "--"):
        return "W"
    if prev == "&":
        prev2 = toks[idx - 2][0] if idx > 1 else ""
        if prev2 in ("(", ",", "=", "return"):
            return "A"
    # skip postfix chain: [..], .x, ->x, (..) is a call -> R unless function ptr
    j = idx + 1
    while j < len(toks):
        t = toks[j][0]
        if t == "[":
            d = 0
            while j < len(toks):
                if toks[j][0] == "[":
                    d += 1
                elif toks[j][0] == "]":
                    d -= 1
                    if d == 0:
                        break
                j += 1
            j += 1
            continue
        if t in (".", "->") and j + 1 < len(toks):
            j += 2
            continue
        break
    nxt = toks[j][0] if j < len(toks) else ""
    if nxt in ("=", "+=", "-=", "*=", "/=", "%=", "|=", "&=", "^=", "<<=", ">>=", "++", "--"):
        return "W"
    # first argument of a writing call
    if prev == "(" and idx > 1 and toks[idx - 2][0] in WRITE_CALLS:
        return "A"
    return "R"


HEAD_RE = re.compile(r"^(?!(?:if|else|for|while|switch|return|do|case|typedef|extern)\b)"
                     r"[A-Za-z_][\w \t\*&:<>,~]*?\b(~?[A-Za-z_]\w*(?:::~?[A-Za-z_]\w*)?)[ \t]*\(")


def column0_heads(code):
    """(line, name) of lines that start at column 0 and look like a function
    definition head: an identifier followed by '(' and, before the next ';',
    an opening brace."""
    lines = code.split("\n")
    heads = []
    for i, l in enumerate(lines):
        m = HEAD_RE.match(l)
        if not m:
            continue
        rest = "\n".join(lines[i:i + 12])
        semi, brace = rest.find(";"), rest.find("{")
        if brace >= 0 and (semi < 0 or brace < semi):
            heads.append((i + 1, m.group(1)))
    return heads


def main():
    root, out = sys.argv[1], sys.argv[2]
    src = os.path.join(root, "TPsource", "V52")
    os.makedirs(out, exist_ok=True)
    files = {}
    for d in CORE_DIRS + REF_EXTRA_DIRS:
        base = os.path.join(src, d)
        for dp, _, fns in os.walk(base):
            for fn in fns:
                if fn.lower().endswith(SRC_EXT):
                    p = os.path.join(dp, fn)
                    files[os.path.relpath(p, src).replace("\\", "/")] = p
    defs = []
    lstats = []
    funcmap = {}
    for rel, p in sorted(files.items()):
        core = rel.split("/")[0] in CORE_DIRS
        try:
            g, ls, fs = scan_file(p, rel)
        except Exception as e:  # keep going, record failure
            print("scan failed", rel, e, file=sys.stderr)
            continue
        funcmap[rel] = fs
        if core and not rel.endswith(".h"):
            for name, ln, st, text in g:
                kind = "static" if st else "global"
                if re.search(r"(^|[ ;}])(if|for|while|return|switch) \(|\) \{", text):
                    kind = "parse-error"  # unbalanced braces in dead #if code
                defs.append((rel, ln, name, kind, text))
            for name, ln, fn in ls:
                lstats.append((rel, ln, fn, name))
        elif core and rel.endswith(".h"):
            for name, ln, st, text in g:
                defs.append((rel, ln, name, "header-def", text))
    # names to trace: non-static globals by name; file statics per file
    global_names = defaultdict(list)
    for rel, ln, name, kind, _ in defs:
        global_names[name].append((rel, kind))
    refs = []
    for rel, p in sorted(files.items()):
        code = strip_code(read_text(p))
        toks = list(tokenize(code))
        fs = funcmap.get(rel, [])
        heads = column0_heads(code)
        def enclosing(line):
            for fn, a, b in fs:
                if a <= line <= b:
                    return fn
            # fallback when brace tracking lost sync: nearest preceding
            # column-0 function head, marked with "~"
            best = None
            for hl, hn in heads:
                if hl <= line:
                    best = hn
                else:
                    break
            return "~" + best if best else "<file scope>"
        for idx, (tok, ln) in enumerate(toks):
            if tok not in global_names:
                continue
            owners = global_names[tok]
            # file-static: only references in the same file
            if all(k == "static" for _, k in owners) and rel not in [o for o, _ in owners]:
                continue
            prev = toks[idx - 1][0] if idx else ""
            if prev in (".", "->", "::"):
                continue  # member of another object
            refs.append((tok, rel, ln, enclosing(ln), access_kind(toks, idx)))
    with open(os.path.join(out, "maarittelyt.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["file", "line", "name", "kind", "declaration"])
        w.writerows(defs)
    with open(os.path.join(out, "staattiset_funktiossa.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["file", "line", "function", "name"])
        w.writerows(lstats)
    with open(os.path.join(out, "viittaukset.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["name", "file", "line", "function", "access"])
        w.writerows(refs)
    # per-name summary: writer functions and reader count
    summ = defaultdict(lambda: {"W": set(), "A": set(), "R": 0, "files": set()})
    for name, rel, ln, fn, ak in refs:
        s = summ[name]
        s["files"].add(rel.split("/")[0])
        if ak == "R":
            s["R"] += 1
        else:
            s[ak].add(rel.split("/")[-1] + ":" + fn)
    with open(os.path.join(out, "kirjoittajat.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["name", "defined_in", "dirs", "n_writer_funcs", "n_reads", "writers", "addr_or_bulk"])
        for name in sorted(global_names):
            s = summ[name]
            w.writerow([name, ";".join(sorted({o for o, _ in global_names[name]})),
                        ";".join(sorted(s["files"])), len(s["W"]), s["R"],
                        ";".join(sorted(s["W"])), ";".join(sorted(s["A"]))])
    by_dir = defaultdict(lambda: [0, 0, 0])
    for rel, ln, name, kind, _ in defs:
        d = rel.split("/")[0]
        by_dir[d][0 if kind == "global" else 1 if kind == "static" else 2] += 1
    ls_dir = defaultdict(int)
    for rel, *_ in lstats:
        ls_dir[rel.split("/")[0]] += 1
    with open(os.path.join(out, "yhteenveto.txt"), "w", encoding="utf-8") as f:
        f.write("dir\tglobal\tfile-static\theader-def\tfunc-static\n")
        for d in CORE_DIRS:
            a, b, c = by_dir[d]
            f.write(f"{d}\t{a}\t{b}\t{c}\t{ls_dir[d]}\n")
        f.write(f"distinct global names\t{len(global_names)}\n")
        f.write(f"references\t{len(refs)}\n")
        f.write("files scanned\t%d\n" % len(files))


if __name__ == "__main__":
    main()
