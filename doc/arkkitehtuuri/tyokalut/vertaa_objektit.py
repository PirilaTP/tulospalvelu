"""Compare two directories of MSVC .obj files, ignoring parts that change on comment-only edits.

Usage: python -I vertaa_objektit.py <dumpbin.exe> <dir_before> <dir_after>

Build both trees with CL=/Brepro and /p:DebugInformationFormat=None first.
Ignored: the "Dump of file" line, the header time date stamp, the
.chks64 section and all .debug$* sections (source file checksum, type
info with build paths). None of these end up in executable code.
Everything else (code, data, relocations, symbols) must be identical.
Exit code 0 = no code/data difference, 1 = differences found.
"""
import hashlib, os, re, subprocess, sys

IGNORED_SECTIONS = {'.debug$S', '.chks64'}


def normalized_dump(dumpbin, path):
    out = subprocess.run([dumpbin, '/all', path], capture_output=True, text=True,
                         encoding='cp1252', errors='replace').stdout
    lines, keep, i = out.splitlines(), [], 0
    skip = False
    while i < len(lines):
        line = lines[i]
        if line.startswith('SECTION HEADER #'):
            name = lines[i + 1].split()[0] if i + 1 < len(lines) else ''
            skip = name in IGNORED_SECTIONS or name.startswith('.debug$')
        elif re.match(r'^RAW DATA #|^RELOCATIONS #|^COFF SYMBOL TABLE|^String Table|^  Summary', line):
            if line.startswith(('COFF SYMBOL TABLE', 'String Table', '  Summary')):
                skip = False
        if not skip and not line.startswith('Dump of file') and 'time date stamp' not in line:
            keep.append(line)
        i += 1
    # symbol table lines reference section numbers but not checksums; keep them.
    # .chks64/.debug$S sizes appear in the Summary; drop those lines.
    keep = [l for l in keep if not re.search(r'\s(\.chks64|\.debug\$\w)\s*$', l)]
    return '\n'.join(keep)


def objs(d):
    res = {}
    for root, _, files in os.walk(d):
        for f in files:
            if f.lower().endswith('.obj'):
                res[f.lower()] = os.path.join(root, f)
    return res


def main():
    dumpbin, before, after = sys.argv[1:4]
    a, b = objs(before), objs(after)
    bad = 0
    for name in sorted(set(a) | set(b)):
        if name not in a or name not in b:
            print('MISSING', name); bad += 1; continue
        ha = hashlib.sha256(normalized_dump(dumpbin, a[name]).encode()).hexdigest()
        hb = hashlib.sha256(normalized_dump(dumpbin, b[name]).encode()).hexdigest()
        if ha != hb:
            print('DIFF', name); bad += 1
    print(f'{len(a)} objects compared, {bad} differ')
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
