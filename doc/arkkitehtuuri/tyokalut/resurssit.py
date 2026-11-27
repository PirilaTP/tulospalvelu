#!/usr/bin/env python3
"""C-9 part A: mechanical inventory of resource and error-handling sites.

Scans TPsource/V52 (directories in SCAN_DIRS) and lists, per call site:
  * resource acquire / release calls (files, TextFl, dbbox files, handles,
    sockets, GDI objects, heap memory),
  * write / persist calls,
  * process exit calls,
with the enclosing function (reusing the parser of globaalit.py) and a
lexical classification of how the return value is used:
  checked      call is inside an if/while/for/switch condition, negated,
               compared, or used with ?: / && / ||
  returned     return <call>
  assigned-checked    x = call(...); and x appears in a condition or a
                      comparison within the next LOOKAHEAD lines of the
                      same function
  assigned-unchecked  x = call(...); x not tested in that window
  ignored      the call is the whole expression statement
  nested       the call is an argument or part of a larger expression
  void         the callee returns nothing (closefile, addrec, ...): n/a
For every writeerror_w / writeerror call it also records what follows the
statement (return, exit, break, continue, goto, end of block, or another
statement) and whether the call is inside an if/else block.

The analysis is lexical (no preprocessor beyond dropping #else branches,
no type info). Output is raw evidence, not a verified result.

v2 (2026-11-26): fixes S1-S3, S5-S8 found by the part A review
(todisteet/c9/osaA-luokittelu.md section 1.3); adds a `built` column.

Usage: python -I resurssit.py <repo-root> <out-dir>
Writes: kutsut.csv, writeerror.csv, funktiot.csv, yhteenveto.txt
"""
import csv
import os
import re
import sys
from collections import Counter, defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from globaalit import read_text, strip_code, tokenize, scan_file, column0_heads  # noqa: E402

SCAN_DIRS = ["Hk", "Juk", "Tp", "com", "cbTp", "tputilv2", "dbboxm",
             "cbHk", "ViestiWin", "HkConsole", "JukConsole"]
SRC_EXT = (".cpp", ".c")
LOOKAHEAD = 6

# name -> (group, kind, returns_value)
CALLS = {}


def _reg(group, kind, names, rv=True):
    for n in names.split():
        CALLS[n] = (group, kind, rv)


_reg("file", "acquire", "fopen _wfopen _fsopen _wfsopen _open _wopen _sopen _wsopen "
     "CreateFile CreateFileW CreateFileA freopen _wfreopen _wfdopen _fdopen "
     "_wfopen_s fopen_s")
_reg("prfile", "acquire", "openprfile")   # v2: wrapper (S8)
_reg("prfile", "release", "closeprfile", rv=False)
_reg("file", "release", "fclose _close _fcloseall")
_reg("file", "write", "fwrite _write fputs fputws fprintf fwprintf fputc fputwc "
     "WriteFile fflush _commit FlushFileBuffers _chsize _chsize_s SetEndOfFile "
     "_wrename _wremove CopyFile CopyFileW MoveFile MoveFileW DeleteFile DeleteFileW")
_reg("file", "read", "fread _read ReadFile")
_reg("file", "seek", "fseek _lseek _fseeki64 _lseeki64 lseek")
# v2: openfile/makefile return int in the built dbbox1.cpp (S2)
_reg("dbbox", "acquire", "openfile makefile openfile_w makefile_w")
_reg("dbbox", "acquire", "openindex makeindex", rv=False)
_reg("dbbox", "release", "closefile closeindex", rv=False)
_reg("dbbox", "write", "putrec putrec_i", rv=True)
_reg("dbbox", "write", "addrec addrec0 deleterec addkey deletekey", rv=False)
_reg("textfl", "method-open", "Open")
_reg("textfl", "method-write", "WriteLine WriteLine8 WriteLineA WriteChar WriteBytes")
_reg("textfl", "method-close", "Close", rv=False)
_reg("handle", "acquire", "CreateEvent CreateEventW CreateMutex CreateMutexW "
     "CreateSemaphore OpenProcess CreateNamedPipe FindFirstFile FindFirstFileW "
     "LoadLibrary LoadLibraryW CreateProcess CreateProcessW")
_reg("handle", "release", "CloseHandle FindClose FreeLibrary")
_reg("socket", "acquire", "socket accept WSASocket")
_reg("socket", "release", "closesocket")
_reg("socket", "write", "send sendto")
_reg("gdi", "acquire", "CreateFont CreateFontW CreateFontIndirect CreateFontIndirectW "
     "CreatePen CreateSolidBrush CreateBrushIndirect CreateCompatibleDC CreateDC "
     "CreateDCW CreateCompatibleBitmap GetDC GetPrinterDC")
_reg("gdi", "release", "DeleteObject DeleteDC ReleaseDC", rv=True)
_reg("mem", "acquire", "malloc calloc realloc _wcsdup _strdup strdup wcsdup")
_reg("mem", "release", "free", rv=False)
_reg("exit", "exit", "exit _exit abort ExitProcess TerminateProcess _endthread "
     "ExitThread lopetus lopetus1", rv=False)
_reg("exit", "shell", "system ShellExecute ShellExecuteW")
_reg("error", "report", "writeerror_w writeerror", rv=False)
_reg("error", "dialog", "perr wperr iowerr")

# member-call names only count after '.' or '->'; free functions only without
MEMBER_ONLY = {n for n, v in CALLS.items() if v[0] == "textfl"}
NOT_TEXTFL = re.compile(r"SQL|Form|Connection|DataSet|Table|Query|Dialog|Timer|^this$")  # S5
COND_KW = ("if", "while", "for", "switch")
CMP = {"==", "!=", "<", ">", "<=", ">=", "!", "?", "&&", "||"}


def stmt_bounds(toks, idx):
    """Return (start, end) token indexes of the statement containing idx.
    start = first token after the previous ';', '{' or '}' at paren depth 0
    relative to idx; end = index of the terminating ';' (or len)."""
    depth = 0
    s = idx - 1
    while s >= 0:
        t = toks[s][0]
        if t == ")":
            depth += 1
        elif t == "(":
            if depth == 0:
                # still inside a condition / argument list: keep going
                pass
            else:
                depth -= 1
        elif t in (";", "{", "}") and depth == 0:
            break
        s -= 1
    start = s + 1
    depth = 0
    e = idx
    while e < len(toks):
        t = toks[e][0]
        if t in ("(", "["):
            depth += 1
        elif t in (")", "]"):
            depth -= 1
        elif t == ";" and depth <= 0:
            break
        elif t == "{" and depth <= 0:
            break
        e += 1
    return start, e


def call_end(toks, idx):
    """Index of the ')' closing the call whose name is at idx (or idx)."""
    j = idx + 1
    if j >= len(toks) or toks[j][0] != "(":
        return idx
    depth = 0
    while j < len(toks):
        if toks[j][0] == "(":
            depth += 1
        elif toks[j][0] == ")":
            depth -= 1
            if depth == 0:
                return j
        j += 1
    return idx


def classify(toks, idx, func_range, rv):
    if not rv:
        return "void", ""
    start, end = stmt_bounds(toks, idx)
    name_start = idx
    if idx >= 2 and toks[idx - 1][0] in (".", "->"):
        # walk back over obj-> / obj. chains
        k = idx - 1
        while k >= 1 and toks[k][0] in (".", "->", "::"):
            k -= 2
        name_start = k + 1 if k + 1 < idx else idx
        if k >= 0 and re.match(r"[A-Za-z_]\w*$", toks[k][0]):
            name_start = k
    ce = call_end(toks, idx)
    before = [t for t, _ in toks[start:name_start]]
    after = toks[ce + 1][0] if ce + 1 < len(toks) else ""
    prev = toks[name_start - 1][0] if name_start > 0 else ""
    # condition context: an if/while/for/switch opener anywhere before in stmt
    # S1 (v2): a call counts as checked by if/while/for/switch only when it
    # lies inside the parenthesised condition; for a brace-less body the
    # statement is re-based after the closing ')'.
    while True:
        kw = [i for i in range(start, name_start) if toks[i][0] in COND_KW]
        if not kw:
            break
        k = kw[0]
        if k + 1 >= len(toks) or toks[k + 1][0] != "(":
            break
        cl = call_end(toks, k)
        if cl >= name_start:
            return "checked", ""
        start = cl + 1
    while start < name_start and toks[start][0] in ("else", "do"):
        start += 1
    if start < name_start and toks[start][0] in ("case", "default"):
        c = start
        while c < name_start and toks[c][0] != ":":
            c += 1
        start = c + 1
    before = [t for t, _ in toks[start:name_start]]
    if before[:1] == ["return"]:
        return "returned", ""
    if prev in CMP or after in CMP:
        return "checked", ""
    if before and before[0] == "else" and len(before) > 1 and before[1] in COND_KW:
        return "checked", ""
    # assignment x = call(...) possibly with cast
    if "=" in before or any(t in ("+=", "|=") for t in before):
        eq = max(i for i, t in enumerate(before) if t in ("=", "+=", "|="))
        lhs = [t for t in before[:eq] if re.match(r"[A-Za-z_]\w*$", t)]
        var = lhs[-1] if lhs else ""
        if not var:
            return "assigned-unchecked", ""
        line0 = toks[end][1] if end < len(toks) else toks[idx][1]  # S3
        fend = func_range[1] if func_range else line0 + LOOKAHEAD
        j = end + 1
        while j < len(toks) and toks[j][1] <= min(line0 + LOOKAHEAD, fend):
            if toks[j][0] == var and j + 1 < len(toks) and toks[j + 1][0] == "=" \
                    and toks[j - 1][0] not in (".", "->"):
                break  # S3: reassigned before any test
            if toks[j][0] == var:
                p = toks[j - 1][0]
                n = toks[j + 1][0] if j + 1 < len(toks) else ""
                # in a condition: preceded by if( / while( / ! / cmp, or followed by cmp
                k = j - 1
                depth = 0
                in_cond = False
                while k >= 0 and toks[k][1] >= toks[j][1] - 1:
                    if toks[k][0] == ")":
                        depth += 1
                    elif toks[k][0] == "(":
                        if depth == 0:
                            if k > 0 and toks[k - 1][0] in COND_KW:
                                in_cond = True
                                break
                        else:
                            depth -= 1
                    elif toks[k][0] in (";", "{", "}"):
                        break
                    k -= 1
                if in_cond or p in CMP or n in CMP:
                    return "assigned-checked", var
            j += 1
        return "assigned-unchecked", var
    if (before == [] or before == ["(", "void", ")"]) and toks[end][0] == ";" \
            and ce + 1 == end:
        return "ignored", ""
    return "nested", ""


def follow(toks, idx):
    """What follows a writeerror call: (next, in_if)."""
    start, end = stmt_bounds(toks, idx)
    j = end + 1
    nxt = toks[j][0] if j < len(toks) else "<eof>"
    if nxt == "}":
        # end of block: look at what comes after the block
        k = j + 1
        after = toks[k][0] if k < len(toks) else "<eof>"
        nxt = "}" + (" else" if after == "else" else "")
    elif nxt not in ("return", "exit", "break", "continue", "goto", "_exit", "abort"):
        nxt = "stmt:" + nxt
    # inside an if body? look back for the opening brace of the enclosing block
    depth = 0
    k = start - 1
    in_if = ""
    while k >= 0:
        t = toks[k][0]
        if t == "}":
            depth += 1
        elif t == "{":
            if depth == 0:
                # tokens before '{': ')' of if(...) or 'else'
                p = toks[k - 1][0] if k > 0 else ""
                if p == "else":
                    in_if = "else"
                elif p == ")":
                    # find the keyword before the matching '('
                    d = 0
                    m = k - 1
                    while m >= 0:
                        if toks[m][0] == ")":
                            d += 1
                        elif toks[m][0] == "(":
                            d -= 1
                            if d == 0:
                                break
                        m -= 1
                    kw = toks[m - 1][0] if m > 0 else ""
                    in_if = kw if kw in COND_KW else "function-body"
                else:
                    in_if = "block"
                break
            depth -= 1
        k -= 1
    if start > 0 and toks[start - 1][0] == ")" and in_if == "":
        in_if = "if-no-brace"
    return nxt, in_if


PROJECTS = ["RADStudio10/DBboxm-XE.cbproj", "RADStudio10/HkKisaWin.cbproj",
            "RADStudio10/Tputil-XE.cbproj", "RADStudio10/ViestiWin.cbproj",
            "VS/Hk/HkMaali/HkMaali.vcxproj", "VS/Libs/dbboxm/dbboxm.vcxproj",
            "VS/Libs/tputilv2.vcxproj", "VS/V/JukLuenta520.vcxproj",
            "VS/V/JukMaali520.vcxproj"]


def built_files(src):
    """S7 (v2): lower-case basenames of sources compiled by a product project."""
    names = set()
    for pr in PROJECTS:
        p = os.path.join(src, pr)
        if not os.path.exists(p):
            continue
        t = open(p, "rb").read().decode("utf-8", errors="replace")
        for m in re.finditer(r'<(?:ClCompile|CppCompile) Include="([^"]+)"', t):
            names.add(re.split(r"[\\/]", m.group(1))[-1].lower())
    return names


def built_flag(rel, built):
    return "yes" if rel.split("/")[-1].lower() in built else "no"


def main():
    root, out = sys.argv[1], sys.argv[2]
    src = os.path.join(root, "TPsource", "V52")
    os.makedirs(out, exist_ok=True)
    files = {}
    for d in SCAN_DIRS:
        base = os.path.join(src, d)
        for dp, _, fns in os.walk(base):
            for fn in fns:
                if fn.lower().endswith(SRC_EXT):
                    p = os.path.join(dp, fn)
                    files[os.path.relpath(p, src).replace("\\", "/")] = p
    built = built_files(src)
    ndefs = 0
    rows = []
    werr = []
    per_func = defaultdict(Counter)
    for rel, p in sorted(files.items()):
        try:
            _, _, fs = scan_file(p, rel)
        except Exception as e:
            print("scan failed", rel, e, file=sys.stderr)
            fs = []
        code = strip_code(read_text(p))
        toks = list(tokenize(code))
        heads = column0_heads(code)

        def enclosing(line):
            for fn, a, b in fs:
                if a <= line <= b:
                    return fn, (a, b)
            best = None
            for hl, hn in heads:
                if hl <= line:
                    best = hn
                else:
                    break
            return ("~" + best if best else "<file scope>"), None

        for idx, (tok, ln) in enumerate(toks):
            nxt = toks[idx + 1][0] if idx + 1 < len(toks) else ""
            prev = toks[idx - 1][0] if idx else ""
            if tok == "new" and prev != "operator":
                # new T / new T[...]
                typ = toks[idx + 1][0] if idx + 1 < len(toks) else ""
                if typ == "(":
                    continue
                fn, fr = enclosing(ln)
                group = "textfl" if typ == "TextFl" else "mem"
                kind = "acquire-new"
                arr = "[]" if any(t == "[" for t, _ in toks[idx + 2:idx + 4]) else ""
                rows.append((rel, ln, fn, group, kind, "new " + typ + arr, "n/a", ""))
                per_func[(rel, fn)][group + ":" + kind] += 1
                continue
            if tok == "delete" and prev != "operator":
                fn, fr = enclosing(ln)
                arr = "[]" if nxt == "[" else ""
                tgt = toks[idx + 3][0] if arr else nxt
                rows.append((rel, ln, fn, "mem", "release-delete", "delete" + arr + " " + tgt,
                             "n/a", ""))
                per_func[(rel, fn)]["mem:release-delete"] += 1
                continue
            if tok not in CALLS or nxt != "(":
                continue
            member = prev in (".", "->")
            if tok in MEMBER_ONLY and not member:
                continue
            if tok not in MEMBER_ONLY and member:
                continue  # obj.fopen(...) etc. is some other method
            if member and tok in MEMBER_ONLY and idx >= 2 and NOT_TEXTFL.search(toks[idx - 2][0]):
                continue  # S5: VCL object, not TextFl
            ce = call_end(toks, idx)
            aft = toks[ce + 1][0] if ce + 1 < len(toks) else ""
            if aft in ("{", "const") or (re.match(r"[A-Za-z_]\w*$", prev) and prev not in
                                         ("return", "else", "do", "case", "new", "delete",
                                          "throw", "goto")):
                ndefs += 1
                continue  # S6: definition or prototype, not a call
            group, kind, rv = CALLS[tok]
            fn, fr = enclosing(ln)
            use, var = classify(toks, idx, fr, rv)
            if group == "error":
                after, in_if = follow(toks, idx)
                werr.append((rel, ln, fn, tok, after, in_if))
            rows.append((rel, ln, fn, group, kind, tok, use, var))
            per_func[(rel, fn)][group + ":" + kind] += 1

    with open(os.path.join(out, "kutsut.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["file", "line", "function", "group", "kind", "call", "use", "var", "built"])
        for r in rows:
            w.writerow(list(r) + [built_flag(r[0], built)])
    with open(os.path.join(out, "writeerror.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["file", "line", "function", "call", "next", "in_block"])
        w.writerows(werr)
    keys = sorted({k for c in per_func.values() for k in c})
    with open(os.path.join(out, "funktiot.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["file", "function"] + keys)
        for (rel, fn), c in sorted(per_func.items()):
            w.writerow([rel, fn] + [c.get(k, 0) for k in keys])
    with open(os.path.join(out, "yhteenveto.txt"), "w", newline="\n", encoding="utf-8") as f:
        f.write("files scanned: %d (%s)\n" % (len(files), ", ".join(SCAN_DIRS)))
        f.write("call sites: %d (definitions/prototypes skipped: %d)\n" % (len(rows), ndefs))
        f.write("call sites in files not built by any product project: %d\n\n"
                % sum(1 for r in rows if built_flag(r[0], built) == "no"))
        c = Counter((r[3], r[4]) for r in rows)
        f.write("by group/kind:\n")
        for (g, k), n in sorted(c.items()):
            f.write("  %-8s %-16s %6d\n" % (g, k, n))
        f.write("\nreturn-value use by group/kind (rv-returning calls only):\n")
        u = Counter((r[3], r[4], r[6]) for r in rows if r[6] not in ("void", "n/a"))
        for (g, k, use), n in sorted(u.items()):
            f.write("  %-8s %-14s %-20s %6d\n" % (g, k, use, n))
        f.write("\nby top directory and group/kind:\n")
        d = Counter((r[0].split("/")[0], r[3], r[4]) for r in rows)
        for (dd, g, k), n in sorted(d.items()):
            f.write("  %-11s %-8s %-16s %6d\n" % (dd, g, k, n))
        f.write("\nwriteerror follow-up (next statement):\n")
        for k, n in Counter(r[4] if not r[4].startswith("stmt:") else "stmt"
                            for r in werr).most_common():
            f.write("  %-14s %6d\n" % (k, n))
        f.write("\nwriteerror enclosing block:\n")
        for k, n in Counter(r[5] for r in werr).most_common():
            f.write("  %-14s %6d\n" % (k or "<none>", n))
        fb = sum(1 for r in rows if r[2].startswith("~"))
        fs_ = sum(1 for r in rows if r[2] == "<file scope>")
        f.write("\nfallback attribution (~): %d, file scope: %d\n" % (fb, fs_))


if __name__ == "__main__":
    main()
