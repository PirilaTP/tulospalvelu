package fi.pirila.tulospalvelu;

/**
 * A competitor read from KILP.DAT.
 * Badge fields are mutable since they can be updated via network messages.
 */
public class Competitor {

    public final int recordIndex;
    public final int kilpno;
    public final String sukunimi;
    public final String etunimi;
    public final String seura;
    public final int sarja;
    public int badge;   // emit card from pv[0]
    public int badge2;  // second emit card from pv[0]

    public Competitor(int recordIndex, int kilpno, String sukunimi, String etunimi,
                      String seura, int sarja, int badge, int badge2) {
        this.recordIndex = recordIndex;
        this.kilpno = kilpno;
        this.sukunimi = sukunimi;
        this.etunimi = etunimi;
        this.seura = seura;
        this.sarja = sarja;
        this.badge = badge;
        this.badge2 = badge2;
    }

    @Override
    public String toString() {
        String badgeStr = badge > 0 ? String.valueOf(badge) : "-";
        return String.format("%4d  %-20s %-15s %-20s emit:%s",
                kilpno, sukunimi, etunimi, seura, badgeStr);
    }
}
