package fi.pirila.tulospalvelu;

/**
 * One club entry. piiri is the area/district code (matching the first column
 * in seurat.csv and the INT16 piiri field in KILP.DAT base records).
 *
 * lyhenne (= seuralyh) is the short form shown in result printouts; nimi is
 * the full club name. lyhenne and piiri may be empty/0 when the entry came
 * from a competitor record that didn't have those fields filled in.
 */
public record Seura(int piiri, String lyhenne, String nimi) {

    public Seura {
        if (lyhenne == null) lyhenne = "";
        if (nimi == null) nimi = "";
    }
}
