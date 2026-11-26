# One-off generator for rekisteri/riskiarvio.csv and rekisteri/korjauskokonaisuudet.csv (C-13).
# The registers are maintained by hand afterwards; tyokalut/synteesi.py --tarkista validates them.
import csv, io, re, sys

root = sys.argv[1]
doc = root + '/doc/arkkitehtuuri/'


def ids(spec, kind):
    out = []
    for part in spec.split():
        m = re.match(r'(\d+)-(\d+)$', part)
        if m:
            out += ['%s-%03d' % (kind, i) for i in range(int(m.group(1)), int(m.group(2)) + 1)]
        else:
            out.append('%s-%03d' % (kind, int(part)))
    return out

# kk: (nimi, tyyppi, vaihe, aalto, H, T, Q, juurisyy, riippuu, rinnakkain, tyomaara, muutosriski,
#      yht_tiedosto, yht_laite, yht_protokolla, testit, paatokset, haara, palautettavuus)
P = [
('KK-C13-1', 'Työasemaprotokollan vastaanoton rajatarkistukset', 'valiton', 1, 1,
 '26 32 33 61 71 72', '', 'Q43 Q18', 'JS-1', 'KK-C13-18', 'KK-C13-2 KK-C13-3 KK-C9-1 KK-C12-2', 'M', 'matala',
 'ei', 'ei', 'ei muutosta sanomamuotoon; virheellinen paketti hylätään; FILESEND-polun rajaus vasta päätöksen OP-5 jälkeen',
 'auto integraatio', 'OP-5', 'korjaus/kk-c13-1-vastaanoton-rajatarkistukset', 'PR kerrallaan (git revert); ei tiedostomuutoksia'),
('KK-C13-2', 'Laitesyötteen jäsentimien rajat ja silmukat', 'valiton', 1, 1,
 '8 55 56 60 70', '', 'Q44 Q45 Q49 Q50 Q51 Q56', 'JS-1', 'KK-C13-18', 'KK-C13-1 KK-C13-3 KK-C9-1', 'M', 'matala',
 'ei', 'ei muutosta kehyksiin; virheellinen sanoma ohitetaan; ampumapaikan arvo 00 varmistettava (H-060)', 'ei',
 'auto laite', 'OP-13', 'korjaus/kk-c13-2-laitejasentimet', 'PR kerrallaan; ei tiedostomuutoksia'),
('KK-C13-3', 'Vaiheen vaihdon ja käynnistyksen tila (HK)', 'valiton', 1, 1,
 '13 102 105 128 129 131 132 133 134 141 143 174', '', 'Q78 Q79 Q80 Q110 Q67', 'JS-6', 'KK-C13-18', 'KK-C13-1 KK-C13-2 KK-C9-1', 'M', 'keskitaso',
 'KILP.DAT-muoto ennallaan (uudelleenrakennus kirjoittaa muuttuneet tietueet samassa muodossa); AJAT-tiedoston nimeämistä ei muuteta ilman päätöstä (H-131, H-132)', 'ei', 'ei',
 'auto gui kasi', 'OP-6', 'korjaus/kk-c13-3-vaiheen-vaihto', 'sarja a yksinään palautettavissa; nimeämismuutokset (sarja c) vaativat siirtymäohjeen'),
('KK-C13-4', 'Lukkojen vapautus paluu- ja poikkeuspoluilla', 'valiton', 1, 1,
 '12 24 40 87 88 89 90 110 115 125', '36', 'Q15 Q41 Q65', 'JS-2', 'KK-C13-18', 'KK-C13-1 KK-C13-2 KK-C12-2', 'M', 'keskitaso',
 'ei', 'ei', 'ei', 'auto gui', 'OP-3', 'korjaus/kk-c13-4-lukkojen-vapautus', 'PR kerrallaan; H-087:n korjaus vasta kun H-092/H-103 on arvioitu (KK-C13-5)'),
('KK-C13-5', 'Lukitusjärjestys ja estävät kutsut lukon alla (writeerror_w)', 'juurisyy', 1, 2,
 '9 19 91 92 93 103 153', '31 32', 'Q14 Q66 Q89', 'JS-4', 'KK-C13-18 KK-C13-4', 'KK-C13-9 KK-C13-10 KK-C10-1', 'L', 'korkea',
 'ei', 'ei', 'ei', 'auto gui kasi', '', 'korjaus/kk-c13-5-lukitusjarjestys', 'PR kerrallaan; lukitusjärjestyksen muutos todennettava GUI-ajolla ennen julkaisua'),
('KK-C13-6', 'Tallennuksen ja jaetun tilan synkronointi', 'juurisyy', 2, 3,
 '10 11 45 47 51 136 137 144', '', 'Q69 Q81', 'JS-10', 'KK-C13-4 KK-C13-5', 'KK-C13-16 KK-C9-2', 'L', 'korkea',
 'ei', 'ei', 'ei', 'auto integraatio gui', '', 'korjaus/kk-c13-6-jaettu-tila', 'PR kerrallaan; uudet lukot voivat tuoda lukkiutumisia, siksi KK-C13-5:n jälkeen'),
('KK-C13-7', 'VCL-käyttö työsäikeistä', 'valiton', 1, 2,
 '18 101 106 107', '33', 'Q88', 'JS-10', 'KK-C13-18', 'KK-C13-9 KK-C13-10', 'M', 'keskitaso',
 'ei', 'ei', 'ei', 'gui kasi', '', 'korjaus/kk-c13-7-vcl-tyosaikeet', 'PR kerrallaan'),
('KK-C13-8', 'Säikeiden elinkaari, vaiheenvaihto-lippu ja hallittu lopetus', 'juurisyy', 1, 2,
 '31 38 94 95 97 98 104 157', '30', 'Q67 Q68 Q70 Q71 Q73', 'JS-7', 'KK-C13-18 KK-C13-3', 'KK-C13-9 KK-C13-10', 'L', 'korkea',
 'ei', 'lukijasäikeiden pysäytys ja jatko vaiheen vaihdossa varmistettava laitteilla', 'ei',
 'auto gui laite kasi', 'OP-6', 'korjaus/kk-c13-8-saikeiden-elinkaari', 'H-031 yksinään palautettavissa; lipun merkityksen muutos (pause/exit) yhtenä sarjana'),
('KK-C13-9', 'Tiedonsiirron yhteydet (soketit)', 'valiton', 1, 2,
 '27 28 37 96 155', '', 'Q20', 'JS-9', 'KK-C13-18', 'KK-C13-5 KK-C13-7 KK-C13-10', 'M', 'keskitaso',
 'ei', 'ei', 'ei muutosta sanomiin; uudelleenyhteyden ajoitus muuttuu, sekaversiot toimivat', 'auto integraatio', '',
 'korjaus/kk-c13-9-tiedonsiirron-yhteydet', 'PR kerrallaan'),
('KK-C13-10', 'Vastaanotetun tietueen soveltaminen (HK tark_kilp)', 'valiton', 1, 2,
 '7 22 23', '', 'Q16 Q21', 'JS-5', 'KK-C13-18 KK-C13-1', 'KK-C13-7 KK-C13-9', 'M', 'keskitaso',
 'ei', 'ei', 'ei muutosta sanomamuotoon; vastaanottaja hylkää ristiriitaisen päivityksen (aiemmin hiljainen väärä kirjoitus) - hylkäyksen näkyvyys käyttäjälle suunniteltava', 'auto integraatio', 'OP-4',
 'korjaus/kk-c13-10-tietueen-soveltaminen', 'PR kerrallaan'),
('KK-C13-11', 'Tietueen kokorajat', 'pieni_itsenainen', 1, 1,
 '41 42 46', '', 'Q26 Q29 Q31 Q34 Q37', 'JS-3', 'KK-C13-18', 'KK-C13-1 KK-C13-2 KK-C13-3', 'S', 'matala',
 'H-041: ei (KILPBUFSIZE on vain pinopuskurin koko); H-046: viestin vientitiedoston sisältö muuttuu - vaatii päätöksen (Q29)', 'ei', 'ei',
 'auto', 'OP-14', 'korjaus/kk-c13-11-tietueen-kokorajat', 'H-041 ja H-042 PR kerrallaan; H-046 erikseen päätöksen jälkeen'),
('KK-C13-12', 'Paikallisten tiedostojen ja tuonnin jäsentimet', 'pieni_itsenainen', 2, 3,
 '2 49 57 58 65 74 78 79 86', '', 'Q47 Q55 Q59 Q64', 'JS-1', 'KK-C13-18', 'KK-C13-13 KK-C13-16 KK-C9-2', 'M', 'matala',
 'lukijat hylkäävät virheellisen tiedoston; kelvolliset tiedostot luetaan ennallaan; TextFl a-tilan korjaus muuttaa sakkokierroslokin säilymistä (H-086)', 'ei', 'ei',
 'auto', 'OP-14', 'korjaus/kk-c13-12-tiedostojasentimet', 'PR kerrallaan'),
('KK-C13-13', 'Kiinteät puskurit muotoilussa ja tulostuksessa', 'pieni_itsenainen', 1, 2,
 '1 3 59 62 64 67 68 73 75 76 77', '', 'Q38 Q39 Q46 Q52 Q53 Q54', 'JS-3', 'KK-C13-18', 'KK-C13-9 KK-C13-10 KK-C13-12', 'M', 'matala',
 'tulosteiden ja lokirivien sisältö ei saa muuttua (vertailutestit); EMVALOKI-rivi säilytetään kokonaisena (puskuria suurennetaan)', 'C-Power-näyttötaulun kehys ennallaan (H-077)', 'ei',
 'auto kasi', '', 'korjaus/kk-c13-13-muotoilupuskurit', 'PR kerrallaan'),
('KK-C13-14', 'GUI: dataa korruptoivat toiminnot', 'valiton', 1, 2,
 '17 109 111 112 113 114 116 117 121', '35 37', 'Q76 Q15', 'JS-1 JS-5 JS-12', 'KK-C13-18', 'KK-C13-9 KK-C13-10 KK-C13-13', 'L', 'keskitaso',
 'ei', 'ei', 'ei', 'gui kasi auto', 'OP-14', 'korjaus/kk-c13-14-gui-datakorruptio', 'PR kerrallaan; jokainen HK-korjaus tarkistetaan viestihaarasta (T-034)'),
('KK-C13-15', 'GUI: syöterajat ja pienet logiikkaviat', 'pieni_itsenainen', 3, 4,
 '69 82 83 84 85 118 119 120 122 123 124 126 127 175', '', 'Q63 Q75', 'JS-1 JS-12', 'KK-C13-14', 'KK-C13-17 KK-C12-3', 'L', 'matala',
 'ei', 'ei', 'ei', 'gui kasi', 'OP-9', 'korjaus/kk-c13-15-gui-syoterajat', 'PR kerrallaan'),
('KK-C13-16', 'Johdettujen rakenteiden eheys', 'pieni_itsenainen', 2, 3,
 '66 130 135 139 140 142', '', 'Q82', 'JS-6', 'KK-C13-3', 'KK-C13-12 KK-C9-2', 'M', 'keskitaso',
 'ei muutosta tiedostomuotoon; hakemistot rakennetaan muistissa', 'ei', 'ei', 'auto', '',
 'korjaus/kk-c13-16-johdetut-rakenteet', 'PR kerrallaan'),
('KK-C13-17', 'Merkistöt ja kääntäjäriippuvuudet', 'pieni_itsenainen', 3, 4,
 '4 5 80 81', '4 16 23 25 27', 'Q33 Q57 Q60 Q62', 'JS-15', 'KK-C13-18', 'KK-C13-15 KK-C12-3', 'M', 'keskitaso',
 'lähdetiedostojen merkistöä ei muuteta ilman päätöstä; tekstitiedostojen sisältö (UTF-8/ANSI) voi muuttua - vertailutesti', 'ei', 'T-025: versiotunniste 520/521 näkyy sanomissa - varmistettava ennen muutosta',
 'auto kasi', 'OP-15 OP-9', 'korjaus/kk-c13-17-merkistot-kaantajat', 'objektivertailu (vertaa_omf.py, /Brepro) ennen yhdistämistä'),
('KK-C13-18', 'Testaus-, CI- ja varoitusperusta', 'testausinfra', 1, 0,
 '6', '21', 'Q35 Q36 Q39 Q40 Q48 Q103', 'JS-13', '', 'kaikki', 'M', 'matala',
 'ei', 'ei', 'ei', 'auto', 'OP-1 OP-2', 'testit/regressioperusta', 'ei vaikuta tuotantokoodiin'),
('KK-C13-19', 'Yhteiset turvalliset apukerrokset (syötteen validointi, rajatut kopiot)', 'juurisyy', 3, 5,
 '', '22 26', '', 'JS-1 JS-3', 'KK-C13-1 KK-C13-2 KK-C13-13', 'KK-C13-20', 'L', 'keskitaso',
 'ei', 'ei', 'ei', 'auto', 'OP-3', 'rakenne/kk-c13-19-apukerrokset', 'käyttöönotto moduuli kerrallaan'),
('KK-C13-20', 'Vaihetilan ja johdettujen rakenteiden keskitetty hallinta', 'arkkitehtuuri', 4, 5,
 '', '38 39', '', 'JS-6', 'KK-C13-3 KK-C13-16 KK-C13-8', 'KK-C13-19', 'XL', 'korkea',
 'ei tavoitteena; tiedostonimien käsittely vaiheittain voi muuttua - siirtymä suunniteltava', 'ei', 'ei', 'auto gui kasi', 'OP-6',
 'rakenne/kk-c13-20-vaihetila', 'vaatii kattavat vaiheen vaihdon regressiotestit ennen aloitusta'),
('KK-C13-21', 'Ytimen ja käyttöliittymän erottaminen, globaali tila', 'arkkitehtuuri', 4, 6,
 '', '1 2 3', 'Q6', 'JS-10', 'KK-C13-18 KK-C13-5 KK-C13-8', '', 'XL', 'korkea',
 'ei tavoitteena', 'ei', 'ei', 'auto gui', 'OP-17', 'rakenne/kk-c13-21-ydin-ja-gui', 'pienin askelin; objektivertailu missä mahdollista'),
('KK-C13-22', 'HK- ja viestihaarojen lähentäminen', 'arkkitehtuuri', 4, 6,
 '', '34', 'Q77', 'JS-12', 'KK-C13-14 KK-C13-15 KK-C13-18', '', 'XL', 'korkea',
 'HK- ja viestitiedostot pysyvät erillisinä muotoina', 'ei', 'HK ja viesti eivät liikennöi keskenään; ei muutosta', 'auto gui', 'OP-16',
 'rakenne/kk-c13-22-haarojen-lahentaminen', 'yksikkö kerrallaan'),
('KK-C13-23', 'Replikoinnin ja tiedostomuotojen versiointi', 'arkkitehtuuri', 4, 5,
 '16 30 34 39', '19', 'Q3 Q19 Q21 Q22 Q27 Q32', 'JS-5', 'KK-C13-10 KK-C13-24 KK-C13-1', '', 'XL', 'korkea',
 'KYLLÄ: tunniste, versio tai tarkistussumma muuttaa KILP.DAT- ja jonotiedostoja - vaatii lukuyhteensopivuuden vanhaan muotoon', 'ei', 'KYLLÄ: versio- tai aikaleimakenttä muuttaa sanomia - vaatii versioneuvottelun ja sekaversiotuen tai yhtäaikaisen päivityksen',
 'auto integraatio kasi', 'OP-4', 'rakenne/kk-c13-23-versiointi', 'vain taaksepäin yhteensopivana; vaatii omistajan päätöksen ennen suunnittelua'),
('KK-C13-24', 'Lähetysjonon asettelu (COMFILE.DAT)', 'valiton', 1, 2,
 '21 25 35 48 50', '', 'Q23 Q42 Q90', 'JS-3', 'KK-C13-18 KK-C13-1', 'KK-C13-9', 'M', 'keskitaso',
 'KYLLÄ (viesti): COMFILE.DAT:n tietuepituus muuttuu; vanha jono todetaan yhteensopimattomaksi ja lähettämättömät hävitetään tai lähetetään uudelleen - päivitys kilpailujen välissä', 'ei',
 'FILESEND-luokan pituus (combufsize-14) jäädytettävä nykyarvoon, muuten korjattu ja korjaamaton ohjelma eivät siirrä tiedostoja', 'auto integraatio', 'OP-4',
 'korjaus/kk-c13-24-lahetysjono', 'vanhaan versioon paluu vaatii COMFILE.DAT:n poiston; ohje julkaisutietoihin'),
('KK-C9-1', 'PRFILE-osoittimien elinkaari ja autofile-polku', 'valiton', 1, 1,
 '149 150 151 152 154', '42', 'Q91 Q93 Q95', 'JS-8', 'KK-C13-18', 'KK-C13-1 KK-C13-2 KK-C13-3', 'M', 'keskitaso',
 'julkaisutiedostojen sisältö ei saa muuttua (vertailutesti)', 'ei', 'FILETCP-tiedostolähetyksen virta ennallaan (Q93)', 'auto gui', '',
 'korjaus/kk-c9-1-prfile-elinkaari', 'PR kerrallaan'),
('KK-C9-2', 'Tallennusvirheiden välitys, kuittaus ja palautus', 'juurisyy', 2, 3,
 '15 44 53 54 145 146 147 148 156', '40 41', 'Q86 Q87 Q94 Q100', 'JS-11', 'KK-C13-18 KK-C13-5', 'KK-C13-12 KK-C13-16', 'L', 'korkea',
 'tiedostomuodot ennallaan; atominen kirjoitus (väliaikaistiedosto + uudelleennimeäminen) muuttaa julkaisutiedostojen päivitystapaa', 'kortin kuittaus lukijalle vasta tallennuksen jälkeen - laitteen käytös varmistettava (Q86, Q100)', 'ei',
 'auto laite kasi', 'OP-13', 'korjaus/kk-c9-2-tallennusvirheet', 'PR kerrallaan; kuittausjärjestyksen muutos laitetestin jälkeen'),
('KK-C9-3', 'Resurssien tarkistukset', 'pieni_itsenainen', 3, 4,
 '63', '20', '', 'JS-8', 'KK-C13-18', 'KK-C13-15 KK-C13-17', 'S', 'matala',
 'ei', 'ei', 'ei', 'auto', '', 'korjaus/kk-c9-3-resurssit', 'PR kerrallaan; MSVC:n invalid parameter -käsittelijän lisäys muuttaa H-059/H-078/H-105-testien odotuksia'),
('KK-C10-1', 'SQL-integraatio', 'valiton', 1, 2,
 '14 43 100 160 161 162 163 169', '', 'Q97 Q104 Q105 Q106 Q109', 'JS-2 JS-7', 'KK-C13-18', 'KK-C13-9 KK-C13-10', 'L', 'keskitaso',
 'SQLjonoVarm.dat:n käsittely muuttuu (H-043) - vanha tiedosto hylättävä hallitusti', 'ei', 'ei; SQL-skeema ennallaan', 'sql gui auto', 'OP-7',
 'korjaus/kk-c10-1-sql-integraatio', 'PR kerrallaan; tuonnin semantiikka (H-169) vasta päätöksen jälkeen'),
('KK-C10-2', 'Laiteyhteyksien hallinta', 'valiton', 1, 1,
 '108 138 159 164 166 167 168 170', '48', 'Q81 Q96 Q99', 'JS-9', 'KK-C13-18', 'KK-C13-1 KK-C13-3 KK-C9-1', 'L', 'keskitaso',
 'ei', 'laiteprotokollat ennallaan; uudelleenyhteys ja elävyysvalvonta muuttavat ajoitusta - laitetesti (Sirit/Zebra, sarjaportti, emiTag-HTTP)', 'ei',
 'laite auto kasi', 'OP-13', 'korjaus/kk-c10-2-laiteyhteydet', 'H-159 yksirivisenä palautettavissa; yhteinen yhteyskerros erillisenä sarjana'),
('KK-C10-3', 'Jakelu ja ulkoiset riippuvuudet', 'testausinfra', 2, 3,
 '165', '5 6 44 46 47 49', 'Q24 Q98 Q101 Q102', 'JS-13', 'KK-C13-18', 'KK-C13-12 KK-C13-16', 'M', 'matala',
 'ei', 'ei', 'ei', 'julkaisu kasi', 'OP-7 OP-8 OP-18', 'julkaisu/kk-c10-3-jakelu', 'julkaisupaketin sisältö versioitu; ei vaikuta ohjelmiin'),
('KK-C10-4', 'Ulkoisten yhteyksien turvallisuus', 'arkkitehtuuri', 1, 2,
 '29', '43', 'Q99 Q105', 'JS-1', 'KK-C13-1', '', 'L', 'korkea',
 'tunnukset asetustiedostossa - muoto voi muuttua', 'emiTag-HTTP: palvelimen tuki ratkaisee (Q99)', 'H-029: osoitteen vaihdon rajaus voi estää nykyisen käyttötavan (dynaaminen vastapuoli) - päätös; todennus tai eheystarkistus muuttaisi protokollaa',
 'integraatio kasi', 'OP-4 OP-13', 'korjaus/kk-c10-4-yhteyksien-turvallisuus', 'asetuksella ohjattava; oletus nykyinen käytös kunnes päätetty'),
('KK-C12-1', 'Viestin XML-julkaisutiedostot', 'pieni_itsenainen', 2, 3,
 '158 172', '', 'Q108', 'JS-14', 'KK-C9-1', 'KK-C13-12 KK-C13-16', 'S', 'matala',
 'XML-tiedostoon tulee sisältöä, jota siinä ei nyt ole - vastaanottavat järjestelmät huomioitava', 'ei', 'ei', 'auto kasi', 'OP-9',
 'korjaus/kk-c12-1-viestin-xml', 'PR kerrallaan'),
('KK-C12-2', 'Asetusten hallinta', 'valiton', 1, 1,
 '36 52 171 173 176', '14 17 24 51', 'Q25 Q107 Q112', 'JS-3 JS-14', 'KK-C13-18', 'KK-C13-1 KK-C13-2 KK-C13-4', 'M', 'matala',
 'H-171: ei (cfgfilename vain luetaan); avainten ja yksiköiden muutos (sarja b) muuttaisi nykyisten asetustiedostojen tulkintaa - vain päätöksellä ja vanhat avaimet hyväksyen', 'laitenumeroiden kuvaus NREGNLY (T-024) ennallaan ilman päätöstä', 'ei',
 'auto gui', 'OP-11 OP-12', 'korjaus/kk-c12-2-asetukset', 'H-171 yksinään palautettavissa; avainmuutokset vain taaksepäin yhteensopivina'),
('KK-C12-3', 'Valitsinhygienia ja kuollut koodi', 'pieni_itsenainen', 3, 4,
 '20 177', '7 8 9 11 12 13 15 18 28 29 45 50 52', 'Q10 Q11 Q12 Q28 Q111', 'JS-14', 'KK-C13-18', 'KK-C13-15 KK-C13-17', 'L', 'keskitaso',
 'ei', 'ei', 'ei', 'auto', 'OP-9 OP-10', 'siivous/kk-c12-3-valitsinhygienia', 'poistot omina commiteinaan; objektivertailu osoittaa, ettei käännetty koodi muutu'),
('ULKOPUOLELLA', 'Ei korjauskokonaisuutta: T-010 on korjattu (CLAUDE.md, c273b9f); H-099 on kumottu eikä ole jäsen', '-', 0, 0,
 '', '10', '', '', '', '', '', '', '', '', '', '', '', '', ''),
]

# secondary package references (a finding stays in ONE primary package)
SEC = {'H-149': 'KK-C12-1', 'H-150': 'KK-C9-3 KK-C12-1', 'H-158': 'KK-C9-1', 'H-153': 'KK-C9-2', 'H-134': 'KK-C9-2',
       'H-161': 'KK-C10-4', 'H-166': 'KK-C10-4', 'H-170': 'KK-C10-1', 'H-176': 'KK-C12-1', 'H-167': 'KK-C13-5 KK-C13-8',
       'H-008': 'KK-C13-4', 'H-155': 'KK-C9-3', 'H-044': 'KK-C13-24', 'H-148': 'KK-C13-4', 'H-047': 'KK-C13-4',
       'H-136': 'KK-C13-8', 'H-014': 'KK-C13-8', 'H-097': 'KK-C10-1', 'H-116': 'KK-C13-5', 'H-073': 'KK-C13-1',
       'H-060': 'KK-C13-1', 'H-033': 'KK-C13-10', 'H-071': 'KK-C10-4', 'H-140': 'KK-C13-14', 'H-139': 'KK-C13-14',
       'H-168': 'KK-C9-2', 'H-145': 'KK-C10-2', 'H-105': 'KK-C13-8', 'H-017': 'KK-C13-10', 'H-050': 'KK-C13-12',
       'H-128': 'KK-C13-16', 'H-013': 'KK-C13-16', 'H-174': 'KK-C13-12', 'H-031': 'KK-C13-3', 'H-094': 'KK-C13-3',
       'H-115': 'KK-C13-14', 'H-019': 'KK-C13-6', 'H-101': 'KK-C13-6', 'H-034': 'KK-C13-9', 'H-096': 'KK-C13-8',
       'H-025': 'KK-C13-1', 'H-077': 'KK-C13-2', 'H-150_': ''}
SEC.pop('H-150_')

# main-session overrides of the Opus draft (see korjaussuunnitelma.md / auditoinnin-yhteenveto.md)
OVR = {
    'H-061': dict(prioriteetti='P1', muutos='',
                  peruste='Main session: kept at P1 (C-4 closing). Reachable from the network with default settings and the effect of the uninitialised length is unknown (Q43); an unknown effect on a default network path is ranked by the worse case until verified. Fixed in the same change as H-032.',
                  huomio_add='Opus draft proposed P2 (medium severity, effect unknown); not adopted without new evidence.'),
    'H-116': dict(prioriteetti='P2', muutos='nostettu',
                  peruste='Main session (C-13): second manifestation found in the root-cause analysis and read from code: errorMsgHandler (cbTp/cbTpApu.cpp:445-463) treats ShowModal result 2 as "Poistu ohjelmasta", and closing the dialog with X returns mrCancel = 2, so closing any GUI error dialog with X would end the program. Routine operator action that stops the program; VCL result value inferred, not run (J7-4).',
                  huomio_add='Raised P3 -> P2 on the error-dialog manifestation; returns to P3 if a GUI run shows X does not return 2.',
                  ajanoton_jatkuvuus='pysayttaa'),
}

OVR.update({
    'H-033': dict(prioriteetti='P1', muutos='uusi', tekninen_vakavuus='korkea', ajanoton_jatkuvuus='pysayttaa',
                  peruste='Main session after the independent review (K1), read from code: Hk/HkCom32.cpp:274-277 indexes the heap array pv[] (new kilppvtp[n_pv], Hk/HkDat.cpp:206) with a signed 16-bit stage index taken from the KILPPVT packet without any check and unpacks about 20 fields into the out-of-range object; every KILPPVT packet reaches it with default settings. Network-default heap write = P1 by the uniform definition.',
                  huomio_add='Register severity is matala (C-2), which was never justified; C-13 rates the technical severity korkea. The Opus draft capped it at P4 because of the low register severity and itself noted that a write outside pv[] would make it P1. Run-time effect not reproduced: an ASan harness test is part of the fix.'),
    'H-153': dict(prioriteetti='P3', muutos='nostettu',
                  peruste='Main session after the independent review (K2): with the H-116 extension the exit choice is reached by closing an error dialog with X, a routine action, so the exit-while-a-worker-holds-a-lock hang is no longer limited to a deliberate exit. Raised P4 -> P3; depends on the same GUI run as H-116.',
                  huomio_add='Returns to P4 if a GUI run shows X does not return 2.'),
    'H-009': dict(peruste='P3: the deadlock needs shooting-penalty use in HkKisaWin or an error branch plus a timing coincidence (rarer precondition than H-091, whose option makes the cycle recur). Not lowered because of verification strength.'),
})

hk = {r['id']: r for r in csv.DictReader(io.open(doc + 'todisteet/c13/osaA0/havainnot.csv', encoding='utf-8', newline=''))}
draft = list(csv.DictReader(io.open(doc + 'todisteet/c13/osaC/riskiarvio_luonnos.csv', encoding='utf-8', newline='')))
prim = {}
for p in P:
    for i in ids(p[5], 'H') + ids(p[6], 'T'):
        assert i not in prim, (i, p[0], prim[i])
        prim[i] = p[0]
allH = ['H-%03d' % i for i in range(1, 178) if i != 99]
allT = ['T-%03d' % i for i in range(1, 53)]
missing = [i for i in allH + allT if i not in prim]
assert not missing, missing

out = []
for r in draft:
    i = r['id']
    if i == 'H-099':
        out.append([i, hk[i]['era'], 'kumottu', '-', '-', '-', '-', '-', '-', '-', '', '', '-', '', 'Refuted in C-6 step 2; kept in the register for history.', ''])
        continue
    o = OVR.get(i, {})
    pr = o.get('prioriteetti', r['prioriteetti'])
    mu = o.get('muutos', r['muutos'])
    if i == 'H-033':
        r = dict(r, huomio='')
    per = o.get('peruste', r['peruste'])
    huo = r['huomio']
    if o.get('huomio_add'):
        huo = (huo + ' ' if huo else '') + o['huomio_add']
    out.append([i, hk[i]['era'], o.get('tekninen_vakavuus', r['tekninen_vakavuus']), r['saavutettavuus'], r['todennus'], r['tulosten_oikeellisuus'],
                o.get('ajanoton_jatkuvuus', r['ajanoton_jatkuvuus']), r['palautettavuus'], r['tietoturva'], pr,
                r['prioriteetti_aiempi'], mu, prim[i], SEC.get(i, ''), per, huo])


def write(path, header, rows):
    buf = io.StringIO(newline='')
    w = csv.writer(buf, lineterminator='\n')
    w.writerow(header)
    w.writerows(rows)
    open(path, 'wb').write(buf.getvalue().encode('utf-8'))

write(doc + 'rekisteri/riskiarvio.csv',
      ['id', 'era', 'tekninen_vakavuus', 'saavutettavuus', 'todennus', 'tulosten_oikeellisuus', 'ajanoton_jatkuvuus',
       'palautettavuus', 'tietoturva', 'prioriteetti', 'prioriteetti_aiempi', 'muutos', 'kk', 'kk_muut', 'peruste', 'huomio'], out)

prio = {r[0]: r[9] for r in out}
# dependencies of later PR series (part:package); the first series uses 'riippuu'
OSAT = {'KK-C13-4': 'PR4(H-087):KK-C13-5',
        'KK-C10-1': 'PR3(H-163):KK-C10-3 PR4(H-014):KK-C13-8',
        'KK-C9-3': 'T-020-kasittelija:KK-C13-13 T-020-kasittelija:KK-C13-12 T-020-kasittelija:KK-C13-3',
        'KK-C13-6': 'H-047:KK-C9-2',
        'KK-C13-14': 'PR2(H-116):KK-C13-5',
        'KK-C13-16': 'H-139,H-140:KK-C13-14'}
rows = []
for p in P:
    (kk, nimi, tyyppi, vaihe, aalto, h, t, q, js, dep, par, size, risk, yf, yl, yp, tests, op, branch, rb) = p
    hs = ids(h, 'H')
    ps = sorted(prio[x] for x in hs)
    pp = ps[0] if ps else ''
    if kk.startswith('KK-') and not pp:
        pp = {'KK-C13-19': 'P3', 'KK-C13-20': 'P3', 'KK-C13-21': 'P4', 'KK-C13-22': 'P4'}[kk]
    cnt = {}
    for x in ps:
        cnt[x] = cnt.get(x, 0) + 1
    rows.append([kk, nimi, tyyppi, pp, ' '.join('%s:%d' % (k, v) for k, v in sorted(cnt.items())), vaihe, aalto,
                 ' '.join(hs), ' '.join(ids(t, 'T')), q, js, dep, OSAT.get(kk, ''), par, size, risk, yf, yl, yp, tests, op, branch, rb])
write(doc + 'rekisteri/korjauskokonaisuudet.csv',
      ['kk', 'nimi', 'tyyppi', 'prioriteetti', 'prioriteettijakauma', 'vaihe', 'aalto', 'havainnot', 'tekninen_velka',
       'kysymykset', 'juurisyy', 'riippuu', 'riippuu_osat', 'rinnakkain', 'tyomaara', 'muutosriski', 'yhteensopivuus_tiedosto',
       'yhteensopivuus_laite', 'yhteensopivuus_protokolla', 'testit', 'omistajan_paatokset', 'haara', 'palautettavuus'], rows)
print(len(rows))
