# Arvonnan valmistelu

Ennen arvonnan toteuttamista on määrättävä sarjojen ensimmäisten
lähtijöiden lähtöajat sekä numerot, joista kunkin sarjan osanottajien numerointi
aloitetaan. Nämä toimet tehdään sarjamäärityksissä, missä tietojen
muokkaus sarjatietotaulukossa on helpoin vaihtoehto. Ohjelma tukee parhaiten
seuraavaa menettelyä:

1. Kaikille sarjoille annetaan alkunumerot siten, että järjestys
   on haluttu, ja koko kilpailun ensimmäinen lähtöaika ensimmäiselle sarjalle. Tietojen
   ei tarvitse muuten olla lähelläkään lopullisia.
   Alkunumero ja ensimmäinen lähtöaika annetaan niin, että se sisältää ensimmäistä sarjaa
   edeltävät varapaikkojen (vakanttien) vapaat lähtöajat.- Sarjatietotaulukossa valitaan näytettäviksi tiedoiksi *1. vaihe* ja
     järjestykseksi *Alkunumero*
     .- Valinnassa *Automaattiset muutokset /
       Vakanttinumeroita sarjojen välissä*
       valitaan lukumäärä. Sama määrä numeroita tulee
       vapaaksi myös ensimmäisen sarjan alkuun.- Valitaan *Automaattiset muutokset /
         Hiihtokilpailun perusmalli*. Tällöin ohjelma määrää kunkin sarjan alkunumeron
         ja lähtöajan niin, että sarjat asettuvat peräkkäin jättäen tilaa
         valitulle määrälle vakanttipaikkoja jokaisen sarjan alkuun.

Toinen vähän monimutkaisempi ja enemmän valinnanmahdollisuuksia tarjoava menettely
on seuraava

1. Kaikille sarjoille annetaan lähtöpaikat, alkunumerot
   ja ensimmäiset lähtöajat siten, että järjestys on haluttu, mutta tietojen ei
   tarvitse muuten olla lähelläkään lopullisia.- Sarjatietotaulukossa valitaan näytettäviksi tiedoiksi *1. vaihe* ja
     järjestykseksi *Alkunumero*. Tällöin ohjelma näyttää sarjoittain
     osanottajien lukumäärät ja niiden perusteella määritetyt kunkin sarjan
     viimeiset numerot olettaen, että alkunumero on aiemmin määritelty. Tämän
     jälkeen voidaan joko muokata alkunumeroita käsin käyttäen apuna ohjelman
     laskemia viimeisiä numeroita tai määrittää alkunumerot automaattisesti käyttäen valikkovalintaa.
     Automaattisessa numeroinnissa on ensin valittava sarjojen väliin jätettävien vapaiden numeroiden määrä
     ja sitten toteutettava itse numerointi.- Vaihdetaan sarjatietotaulun järjestykseksi
       *Lähtö/Lähtöaika*. Tällöin näkyvät saman lähdön sarjat
       peräkkäin alustavien lähtöaikojen mukaisessa järjestyksessä. Ohjelmaa voidaan
       tämän jälkeen pyytää laskemaan sarjojen lähtöajat valitsemalla toiminto
       valikosta. Vaihtoehtoisesti voidaan käyttää hyväksi ohjelman kertomia tieoja
       ja syöttää ajat käsin.- Sen, että numerointialueet eivät mene päällekkäin voi
         vielä varmistaa arvontakaavakkeen painikkeella *Etsi numeroalueiden
         päällekkäisyydet*