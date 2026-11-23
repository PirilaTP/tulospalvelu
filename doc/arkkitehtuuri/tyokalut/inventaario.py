"""Read-only inventory of TPsource/V52.

Usage: python -I inventaario.py <repo_root> <out_json>
Writes per-file encoding, size, comment lines, project membership, include
graph (directory level), entry points, extern counts and orphan candidates.
Does not modify any file. See doc/arkkitehtuuri/inventaario.md.
"""
import json, os, re, sys, collections, xml.etree.ElementTree as ET

ROOT = os.path.abspath(sys.argv[1])
SRC = os.path.join(ROOT, 'TPsource', 'V52')
OUT = sys.argv[2]
SRC_EXT = ('.c', '.cpp', '.h', '.hpp')

def rel(p):
    return os.path.relpath(p, SRC).replace('\\', '/')

def norm(p):
    return os.path.normcase(os.path.normpath(p))

# ---------- files ----------
files = {}
for d, dn, fn in os.walk(SRC):
    if any(x in d for x in ('.git', 'Release_Build', 'Debug_Build', '__history', '__recovery')):
        continue
    for f in fn:
        if f.lower().endswith(SRC_EXT + ('.dfm',)):
            p = os.path.join(d, f)
            b = open(p, 'rb').read()
            bom = b.startswith(b'\xef\xbb\xbf')
            if all(c < 0x80 for c in b):
                enc = 'ascii'
            else:
                try:
                    b.decode('utf-8'); enc = 'utf-8-bom' if bom else 'utf-8'
                except UnicodeDecodeError:
                    enc = 'mixed' if re.search(rb'\xc3[\xa4\xb6\x84\x96\xa5\x85]', b) else 'cp1252'
            text = b.decode('utf-8' if enc.startswith('utf') else 'cp1252', errors='replace')
            lines = text.splitlines()
            com = sum(1 for l in lines if re.match(r'\s*(//|/\*|\*)', l))
            files[norm(p)] = dict(path=rel(p), dir=rel(d).split('/')[0], lines=len(lines),
                                  comment_lines=com, encoding=enc, fffd=text.count('�') if enc.startswith('utf') else 0,
                                  text=text)

# ---------- projects ----------
projects = {}
def add_proj(path, kind, srcs, defines, libs, incdirs, ptype):
    projects[rel(path)] = dict(kind=kind, type=ptype, sources=sorted(set(srcs)), defines=sorted(set(defines)),
                               libs=sorted(set(libs)), incdirs=sorted(set(incdirs)))

for d, dn, fn in os.walk(SRC):
    for f in fn:
        p = os.path.join(d, f)
        fl = f.lower()
        if fl.endswith('.vcxproj') or fl.endswith('.cbproj'):
            raw = open(p, 'rb').read().decode('utf-8', errors='replace')
            raw = re.sub(r'xmlns="[^"]+"', '', raw, count=1)
            try:
                tree = ET.fromstring(raw.lstrip('﻿'))
            except ET.ParseError as e:
                print('parse error', p, e); continue
            srcs, defs, libs, incs = [], [], [], []
            tags_src = ('ClCompile', 'ClInclude', 'CppCompile', 'None') if fl.endswith('.vcxproj') else ('CppCompile', 'PCHCompile', 'LibFiles', 'None')
            for el in tree.iter():
                tag = el.tag.split('}')[-1]
                inc = el.get('Include')
                if inc and tag in ('ClCompile', 'ClInclude', 'CppCompile', 'PCHCompile', 'LibFiles', 'FormResources', 'None'):
                    full = os.path.normpath(os.path.join(d, inc))
                    srcs.append(full)
                if tag in ('PreprocessorDefinitions', 'Defines', 'BCC_Defines') and el.text:
                    defs += [x.strip() for x in el.text.split(';') if x.strip() and not x.strip().startswith(('$(', '%('))]
                if tag in ('AdditionalDependencies', 'LinkPackageImports', 'AllPackageLibs', 'PackageImports', 'LinkPackageStatics') and el.text:
                    libs += [x.strip() for x in re.split(r'[;,]', el.text) if x.strip() and not x.strip().startswith(('$(', '%('))]
                if tag in ('AdditionalIncludeDirectories', 'IncludePath', 'BCC_IncludePath') and el.text:
                    incs += [x.strip() for x in el.text.split(';') if x.strip() and not x.strip().startswith(('$(', '%('))]
            ptype = ''
            for el in tree.iter():
                if el.tag.split('}')[-1] in ('ConfigurationType', 'ProjectType') and el.text:
                    ptype = el.text; break
            add_proj(p, 'vs' if fl.endswith('.vcxproj') else 'cb', srcs, defs, libs, incs, ptype)

# membership
member = collections.defaultdict(list)
for pn, pr in projects.items():
    keep = []
    for s in pr['sources']:
        k = norm(s)
        if k in files:
            member[k].append(pn); keep.append(files[k]['path'])
        else:
            keep.append('?' + os.path.relpath(s, SRC).replace('\\', '/'))
    pr['sources'] = keep

# ---------- includes ----------
by_base = collections.defaultdict(list)
for k, v in files.items():
    by_base[os.path.basename(k)].append(k)
inc_re = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.M)
edges = collections.Counter()
included_by = collections.defaultdict(set)
unresolved = collections.Counter()
for k, v in files.items():
    if v['path'].lower().endswith('.dfm'):
        continue
    for m in inc_re.finditer(v['text']):
        name = m.group(1).replace('\\', '/')
        base = os.path.normcase(os.path.basename(name))
        cand = by_base.get(base, [])
        if not cand:
            unresolved[name.lower()] += 1; continue
        # prefer same dir, then include/, else first
        same = [c for c in cand if os.path.dirname(c) == os.path.dirname(k)]
        incd = [c for c in cand if norm(os.path.join(SRC, 'include')) in c]
        tgt = (same or incd or cand)[0]
        included_by[tgt].add(k)
        edges[(v['dir'], files[tgt]['dir'])] += 1

# ---------- entry points / globals ----------
entry_re = re.compile(r'^\s*(?:int|void|INT|WINAPI|int\s+WINAPI|int\s+_tmain)\b[^;\n]*\b(main|wmain|_tmain|WinMain|_tWinMain|wWinMain)\s*\(', re.M)
entries = []
for k, v in files.items():
    for m in entry_re.finditer(v['text']):
        line = v['text'][:m.start()].count('\n') + 1
        entries.append(dict(file=v['path'], func=m.group(1), line=line, projects=member.get(k, [])))

extern_re = re.compile(r'^\s*extern\s+(?!"C")[^;(]*?;', re.M)
externs = collections.Counter()
for k, v in files.items():
    if v['path'].lower().endswith(('.h', '.hpp')):
        externs[v['path']] = len(extern_re.findall(v['text']))

# ---------- summary ----------
dirs = collections.defaultdict(lambda: collections.Counter())
for k, v in files.items():
    if v['path'].lower().endswith('.dfm'):
        dirs[v['dir']]['dfm'] += 1; continue
    c = dirs[v['dir']]
    c['files'] += 1; c['lines'] += v['lines']; c['comment_lines'] += v['comment_lines']; c[v['encoding']] += 1

orphans = sorted(v['path'] for k, v in files.items()
                 if not member.get(k) and not included_by.get(k) and not v['path'].lower().endswith('.dfm'))

out = dict(
    dirs={d: dict(c) for d, c in sorted(dirs.items())},
    projects=projects,
    files=[{kk: vv for kk, vv in v.items() if kk != 'text'} | dict(projects=member.get(k, []), included_by=len(included_by.get(k, ())))
           for k, v in sorted(files.items(), key=lambda kv: kv[1]['path'].lower())],
    dir_edges=sorted([[a, b, n] for (a, b), n in edges.items()], key=lambda x: (x[0], -x[2])),
    unresolved_includes=unresolved.most_common(),
    entries=entries,
    externs_per_header=sorted([[p, n] for p, n in externs.items() if n], key=lambda x: -x[1]),
    orphans=orphans,
)
json.dump(out, open(OUT, 'w', encoding='utf-8'), indent=1, ensure_ascii=False)
print('files', len(files), 'projects', len(projects), 'entries', len(entries), 'orphans', len(orphans))
