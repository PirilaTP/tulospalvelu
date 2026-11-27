"""Create or refresh the per-file audit register (doc/arkkitehtuuri/rekisteri/tiedostot.csv).

Usage:
  python -I inventaario.py <repo_root> <inv.json>        (first, outside the repo)
  python -I rekisteri.py <inv.json> <repo_root>/doc/arkkitehtuuri/rekisteri/tiedostot.csv <git_rev>

Measured columns (lines, encoding, projects) are refreshed from the inventory.
Manual columns (analyysitaso, arkkitehtuuri, kommentointi, tarkistettu_rev,
havainnot, huomiot) are kept for files that already exist in the CSV.
Files that disappeared are kept with status "poistunut" so history is not lost.
The analysis level is never raised automatically.
"""
import csv, json, sys

MEASURED = ['polku', 'hakemisto', 'rivit', 'merkisto', 'projektit']
MANUAL = ['analyysitaso', 'arkkitehtuuri', 'kommentointi', 'tarkistettu_rev', 'havainnot', 'huomiot']
COLUMNS = MEASURED + MANUAL + ['inventoitu_rev']


def main():
    inv_path, csv_path, rev = sys.argv[1:4]
    inv = json.load(open(inv_path, encoding='utf-8'))
    old = {}
    try:
        with open(csv_path, encoding='utf-8', newline='') as f:
            for row in csv.DictReader(f):
                old[row['polku']] = row
    except FileNotFoundError:
        pass

    rows = []
    seen = set()
    for f in inv['files']:
        p = f['path']
        if p.lower().endswith('.dfm'):
            continue
        seen.add(p)
        row = {
            'polku': p,
            'hakemisto': f['dir'],
            'rivit': f['lines'],
            'merkisto': f['encoding'],
            'projektit': ' '.join(x.split('/')[-1].rsplit('.', 1)[0] for x in f['projects']) or '-',
            'inventoitu_rev': rev,
        }
        prev = old.get(p, {})
        for c in MANUAL:
            row[c] = prev.get(c, '')
        if not row['analyysitaso']:
            row['analyysitaso'] = '1'
        if not row['arkkitehtuuri']:
            row['arkkitehtuuri'] = '-'
        if not row['kommentointi']:
            row['kommentointi'] = '-'
        rows.append(row)
    for p, prev in old.items():
        if p not in seen:
            prev = dict(prev)
            prev['huomiot'] = (prev.get('huomiot', '') + ' poistunut').strip()
            rows.append({c: prev.get(c, '') for c in COLUMNS})
    rows.sort(key=lambda r: r['polku'].lower())
    with open(csv_path, 'w', encoding='utf-8', newline='') as f:
        w = csv.DictWriter(f, fieldnames=COLUMNS)
        w.writeheader()
        w.writerows(rows)
    levels = {}
    for r in rows:
        levels[r['analyysitaso']] = levels.get(r['analyysitaso'], 0) + 1
    print(f'{len(rows)} rows; analysis levels: {dict(sorted(levels.items()))}')


if __name__ == '__main__':
    main()
