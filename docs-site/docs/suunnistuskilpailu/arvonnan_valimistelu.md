# Arvonnan valimistelu

Ennen arvonnan toteuttamista on määrättävä sarjojen ensimmäisten lähtijöiden
lähtöajat sekä numerot, joista kunkin sarjan osanottajien numerointi aloitetaan.
Nämä toimet tehdään sarjamäärityksissä, missä tietojen muokkaus
sarjatietotaulukossa on helpon soveltuvin vaihtoehto. Ohjelma tukee parhaiten
seuraavaa menettelyä:

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
       peräkkäin alustavien lähtöaikojen mukaisessa järjestyksessä. Ohjelman laskemat kunkin sarjan
       viimeiset lähtöajat helpottavat nyt lähtöaikojen määrittämistä niin, että
       samasta karsinasta lähtevät sarjat on helppo panna lähtemään sopivin
       porrastuksin. Koska yllensä ei ole tarkoituksenmukaista ilmoittaa lähtöpaikkaa
       karsinan tarkkuudella ei automatisoitu lähtöaikojen menettely sovellu tavanomaisiin suunnistuskilpailuihin,
       mutta lasketuista sarjojen viimeisistä lähtöajoista on apua. Luonnollisesti on myös varmistettava,
       että lähtövälit on määritelty oikein.- Sen, että numerointialueet eivät mene päällekkäin voi
         vielä varmistaa arvontakaavakkeen painikkeella *Etsi numeroalueiden
         päällekkäisyydet*
         .