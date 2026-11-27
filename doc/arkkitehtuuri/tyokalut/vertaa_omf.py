"""Compare classic bcc32 (Intel OMF) .obj files, ignoring parts that change on comment-only edits.

Usage: python -I vertaa_omf.py <before.obj|dir_before> <after.obj|dir_after>

The bcc32 counterpart of vertaa_objektit.py (MSVC). Build both versions with the
recorded command line (build-logs/*.log), outputs redirected to scratch folders.
An OMF record is [type:1][length:2 LE][body: length-1][checksum:1]; COMENT records
are named by their comment class.
Ignored (measured in C-5 step 2, see doc/arkkitehtuuri/kaantajaerot.md section 13):
THEADR, COMENT class E8 (source file name + DOS time stamp), E9 (dependency file +
time stamp), FB sub-type 06 (output directory path), LINNUM/LINSYM (line numbers).
None of these is code or data. Other FB sub-types (#pragma resource, link, package)
are compared: ignoring the whole class hid them (found in the independent review). Everything else (LEDATA, LIDATA, FIXUPP, PUBDEF, EXTDEF,
SEGDEF, COMDAT, ...) must be identical, in order.
Files using __LINE__ (tputilv2/perr.cpp etc.) really change when lines move.
Exit code 0 = no code/data difference, 1 = differences found.
"""
import collections, os, sys

NAMES = {0x80: 'THEADR', 0x82: 'LHEADR', 0x88: 'COMENT', 0x8A: 'MODEND', 0x8B: 'MODEND32',
         0x8C: 'EXTDEF', 0x90: 'PUBDEF', 0x91: 'PUBDEF32', 0x94: 'LINNUM', 0x95: 'LINNUM32',
         0x96: 'LNAMES', 0x98: 'SEGDEF', 0x99: 'SEGDEF32', 0x9A: 'GRPDEF', 0x9C: 'FIXUPP',
         0x9D: 'FIXUPP32', 0xA0: 'LEDATA', 0xA1: 'LEDATA32', 0xA2: 'LIDATA', 0xA3: 'LIDATA32',
         0xB0: 'COMDEF', 0xB2: 'BAKPAT', 0xB3: 'BAKPAT32', 0xB4: 'LEXTDEF', 0xB6: 'LPUBDEF',
         0xB7: 'LPUBDEF32', 0xB8: 'LCOMDEF', 0xBC: 'CEXTDEF', 0xC2: 'COMDAT', 0xC3: 'COMDAT32',
         0xC4: 'LINSYM', 0xC5: 'LINSYM32', 0xC6: 'ALIAS', 0xC8: 'NBKPAT', 0xC9: 'NBKPAT32',
         0xCA: 'LLNAMES'}
IGNORE = {'THEADR', 'COMENT_E8', 'COMENT_E9', 'COMENT_FB06', 'LINNUM', 'LINNUM32', 'LINSYM', 'LINSYM32'}


def records(path):
    b = open(path, 'rb').read()
    i, out = 0, []
    while i + 3 <= len(b):
        t = b[i]
        n = b[i + 1] | (b[i + 2] << 8)
        body = b[i + 3:i + 3 + n - 1]
        kind = NAMES.get(t, 'T%02X' % t)
        if kind == 'COMENT' and len(body) >= 2:
            kind = 'COMENT_%02X' % body[1]
            # FB has sub-types: 06 = output path (ignored); 08 libraries,
            # 09 resource (#pragma resource), 0A unit name, 00/0C package data,
            # 05 unknown - these are compared
            if body[1] == 0xFB and len(body) >= 3:
                kind = 'COMENT_FB%02X' % body[2]
        out.append((kind, body))
        i += 3 + n
        if kind in ('MODEND', 'MODEND32'):
            break
    return out


def compare(a, b):
    """Return the record kinds whose code/data content differs (empty = equal)."""
    ra = [(k, x) for k, x in records(a) if k not in IGNORE]
    rb = [(k, x) for k, x in records(b) if k not in IGNORE]
    if ra == rb:
        return []
    ka, kb = collections.defaultdict(list), collections.defaultdict(list)
    for k, x in ra:
        ka[k].append(x)
    for k, x in rb:
        kb[k].append(x)
    return sorted(k for k in set(ka) | set(kb) if ka.get(k) != kb.get(k)) or ['ORDER']


def main():
    a, b = sys.argv[1:3]
    if os.path.isdir(a):
        names = sorted(f for f in os.listdir(a) if f.lower().endswith('.obj'))
        pairs = [(os.path.join(a, f), os.path.join(b, f), f) for f in names]
        extra = sorted(f for f in os.listdir(b) if f.lower().endswith('.obj') and f not in names)
    else:
        pairs = [(a, b, os.path.basename(a))]
        extra = []
    bad = len(extra)
    for f in extra:
        print('ONLY IN AFTER', f)
    for x, y, f in pairs:
        if not os.path.exists(y):
            print('MISSING', f)
            bad += 1
            continue
        d = compare(x, y)
        if d:
            bad += 1
            print('DIFF', f, ' '.join(d))
    print('%d / %d differ' % (bad, len(pairs) + len(extra)))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
