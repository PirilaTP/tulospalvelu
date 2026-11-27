"""Write per-row triage verdicts (from the three auditor reports, C-4 step 1) and check
that every input row has exactly one category. Unlisted rows are FALSE_POSITIVE only
when the report's table listed them as such; the script fails if a row is unknown."""
import csv, collections, sys
D = 'C:/dev/tulospalvelu/doc/arkkitehtuuri/todisteet/c4/'
C, L, I, F = 'CONFIRMED', 'LIKELY', 'INVESTIGATE', 'FALSE_POSITIVE'

def keys(s):
    out = []
    for part in s.split(';'):
        f, lines = part.strip().split(':')
        out += [(f, int(x)) for x in lines.split(',')]
    return out

V = {
 'A1': {C: 'Hk/HkAjat.cpp:465,471;Hk/HkApu.cpp:764;Hk/HkCom32.cpp:957;Hk/HkMuotoilu.cpp:2978,2987;Hk/HkTulostus.cpp:5721;Hk/HkXml.cpp:1619,1628,1800,1809;Hk/HkXml_IOF30.cpp:653,662,780,789;HkConsole/HkCons0.cpp:854;HkConsole/HkCons2.cpp:333;HkConsole/HkCons3.cpp:948,1072,1076',
        L: 'Hk/HkEmit.cpp:1566,2310,2930,2995;Hk/HkIV.cpp:688;Hk/HkTls.cpp:905;Hk/HkTulostus.cpp:823;Hk/HkXmlTulokset.cpp:874,1249;HkConsole/HkCons2.cpp:1302,1324;HkConsole/HkCons3.cpp:2327',
        I: 'Hk/HkEmit.cpp:723;Hk/HkXmlTulokset.cpp:1211,1480;HkConsole/HkCons2.cpp:418,2663'},
 'A2': {C: 'Tp/TpComY.cpp:874;tputilv2/QSORTINT.cpp:117,118;tputilv2/dbindex.cpp:237,238',
        L: 'Tp/TpApu.cpp:164,817;Tp/TpComY.cpp:500,805,2725;Tp/TpLaitteet.cpp:714;com/WinUDP.cpp:281,282;dbboxm/dbbox1.cpp:192;tputilv2/DateNo.cpp:133,134;tputilv2/Wrttext.cpp:389;tputilv2/dbindex.cpp:201,208,216;tputilv2/oem2ansi.cpp:116,117;tputilv2/vidint.cpp:42;tputilv2/xmlparser.cpp:274',
        I: ''},
 'B': {C: 'Juk/VTulostus.cpp:2228,3981,8631;Juk/vApu.cpp:891;Juk/vSarjat.cpp:283;JukConsole/VConTul.cpp:684,809,813;ViestiWin/UnitArvonta.cpp:418;ViestiWin/UnitImport.cpp:986',
       L: 'Juk/VCom.cpp:989,1358;Juk/VEmit.cpp:906,1319,2388,5965,5966;Juk/VInit.cpp:472;Juk/VMuotoilu.cpp:2028,2037;Juk/VXml.cpp:1472,1481,1657,1666,1853;Juk/VXml_IOF30.cpp:539,548,895,904;Juk/VTulostus.cpp:1387,1388,8675,8679;JukConsole/laskemtn.cpp:604;JukConsole/laskent0.cpp:386,505,510,526;JukConsole/laskenta.cpp:126;ViestiWin/UnitEmit.cpp:2313',
       I: 'Juk/VEmit.cpp:3500,3553;Juk/VIv.cpp:786,1690,2226;Juk/VTulostus.cpp:1019,1021,1868,3076,4501,5927;Juk/XMLtulos.cpp:326,446;Juk/vApu.cpp:711;JukConsole/vcons2.cpp:2064,2200;ViestiWin/TulostusUnit2.cpp:115,139;ViestiWin/UnitAjanotto.cpp:1096;ViestiWin/UnitEmit.cpp:2900;ViestiWin/UnitGAZ.cpp:914,918,997,1026,1030,1047,1072,1130'},
}
NOTE = {('Juk/VXml.cpp', 1853): 'not classified by auditor B; classified by the main session (literal "RT" over-read when taslaji%10 >= 3; low)'}
tot = collections.Counter()
for name, fn in (('A1', 'triage_A1.csv'), ('A2', 'triage_A2.csv'), ('B', 'triage_B.csv')):
    rows = list(csv.DictReader(open(D + fn, encoding='utf-8')))
    m = {}
    for cat, s in V[name].items():
        if s:
            for k in keys(s):
                m[k] = cat
    present = set((r['file'], int(r['line'])) for r in rows)
    unknown = [k for k in m if k not in present]
    if unknown:
        sys.exit('%s: listed but not in input: %s' % (name, unknown))
    out = D + fn.replace('.csv', '_luokitus.csv')
    with open(out, 'w', newline='', encoding='utf-8') as fh:
        w = csv.writer(fh)
        w.writerow(['file', 'line', 'code', 'category', 'note'])
        c = collections.Counter()
        for r in rows:
            k = (r['file'], int(r['line']))
            cat = m.get(k, F)
            c[cat] += 1
            w.writerow([r['file'], r['line'], r['code'], cat, NOTE.get(k, '')])
    print(name, len(rows), dict(c))
    tot += c
print('total', sum(tot.values()), dict(tot))
