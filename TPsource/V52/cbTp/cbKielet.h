// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
// Copyright (C) 2015 Pekka Pirila

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Windows-käyttöliittymän lomakkeiden kääntäminen käyttöliittymän kielelle (ks. Kielet.h).

#ifndef CBKIELET_H
#define CBKIELET_H

#include <vcl.h>
#include "Kielet.h"

// Kääntää lomakkeen ja sen komponenttien tekstit (Caption, Hint, Items, Lines,
// EditLabel, Title, Filter, StatusBar-paneelit, ListView-sarakkeet).
// Kutsutaan lomakkeen konstruktorin alussa: kaannaLomake(this);
void kaannaLomake(TComponent *Lomake);

// Lisää valikkoon kohdat käyttöliittymän kielen valintaan (Suomi / Svenska / English).
// Valinta tallennetaan käyttäjän asetuksiin ja tulee voimaan uudelleenkäynnistyksessä.
void lisaaKielivalikko(TMenuItem *Valikko);

// Lisää valikkoon alivalikon "Tulosteiden kieli" (Suomi / Svenska / English), joka asettaa
// muuttujan tulostekieli. Palauttaa alivalikon, jonka valinnan voi päivittää funktiolla
// paivitaTulostekielivalikko, jos tulostekieli muuttuu muualla (esim. asetuksista).
TMenuItem *lisaaTulostekielivalikko(TMenuItem *Valikko);
void paivitaTulostekielivalikko(TMenuItem *Ylin);

// Lukee käyttöliittymän kielen käyttäjän asetuksista. Kutsutaan ennen lomakkeiden luontia.
void alustaKayttokieli(void);

#endif
