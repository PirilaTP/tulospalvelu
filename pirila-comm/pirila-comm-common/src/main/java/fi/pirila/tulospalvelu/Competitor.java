package fi.pirila.tulospalvelu;

/**
 * A competitor read from KILP.DAT.
 * Mutable fields can be updated via network messages (KILPPVT/KILPT).
 */
public class Competitor {

    /**
     * "Not set" sentinel for tlahto, mirroring C++ TMAALI0 = -24*TUNTI.
     * TUNTI in TpDef.h = 36000*KSEK = 3_600_000 ticks (1 tick = 1 ms).
     * So TMAALI0 = -24 * 3_600_000 = -86_400_000.
     */
    public static final int TLAHTO_NOT_SET = -24 * 3_600_000;
    private static final int MS_PER_DAY = 24 * 3_600_000;

    public final int recordIndex;
    public int kilpno;
    public String sukunimi;
    public String etunimi;
    public String seura;
    public String seuralyh;  // club abbreviation (WTEXT @ 244 in KILP.DAT)
    public int piiri;        // district/area code (INT16 @ 34); 0 = unset
    public int sarja;
    public int badge;   // emit card from pv[0]
    public int badge2;  // second emit card from pv[0]
    public char keskhyl; // K=DNF, H=DSQ, E=DNS, 0=normal
    public int ysija;    // position (1-based), 0=no result
    public int finishTime; // result time in milliseconds, 0=no time
    public int startTime;  // tlahto from pv[0], time-of-day in milliseconds, 0=no start time

    public Competitor(int recordIndex, int kilpno, String sukunimi, String etunimi,
                      String seura, String seuralyh, int piiri,
                      int sarja, int badge, int badge2,
                      char keskhyl, int ysija, int finishTime, int startTime) {
        this.recordIndex = recordIndex;
        this.kilpno = kilpno;
        this.sukunimi = sukunimi;
        this.etunimi = etunimi;
        this.seura = seura;
        this.seuralyh = seuralyh;
        this.piiri = piiri;
        this.sarja = sarja;
        this.badge = badge;
        this.badge2 = badge2;
        this.keskhyl = keskhyl;
        this.ysija = ysija;
        this.finishTime = finishTime;
        this.startTime = startTime;
    }

    /**
     * Format the result as a human-readable string.
     * "Ei läht." / "Kesk." / "Hyl." / "3. 1:21:39" / "Avoin"
     */
    public String formatResult() {
        if (keskhyl == 'E' || keskhyl == 'e') return "Ei läht.";
        if (keskhyl == 'K' || keskhyl == 'k') return "Kesk.";
        if (keskhyl == 'H' || keskhyl == 'h') return "Hyl.";
        if (finishTime > 0) {
            String time = formatTimeMs(finishTime);
            return ysija > 0 ? ysija + ". " + time : time;
        }
        return "Avoin";
    }

    /**
     * Numeric sort order for results.
     * Placed finishers (1,2,3...), then unplaced with time, then open, DNF, DSQ, DNS.
     */
    public int resultOrder() {
        if (ysija > 0 && finishTime > 0) return ysija;
        if (finishTime > 0) return 10000 + finishTime / 1000;
        if (keskhyl == 0 || keskhyl == 'T' || keskhyl == 't') return 100000;
        if (keskhyl == 'K' || keskhyl == 'k') return 200000;
        if (keskhyl == 'H' || keskhyl == 'h') return 300000;
        if (keskhyl == 'E' || keskhyl == 'e') return 400000;
        return 500000;
    }

    /**
     * Format milliseconds as H:MM:SS or M:SS.
     */
    public static String formatTimeMs(int millis) {
        int totalSeconds = millis / 1000;
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        if (hours > 0) {
            return String.format("%d:%02d:%02d", hours, minutes, seconds);
        }
        return String.format("%d:%02d", minutes, seconds);
    }

    /**
     * Format tlahto as HH:MM:SS clock time. tlahto is stored as milliseconds
     * from NOON (C++ Pirilä convention — see VIx.cpp:436 NORMKELLO macro):
     * value 0 = 12:00, +3 600 000 = 13:00, -3 600 000 = 11:00.
     * Returns empty string when the value equals the TMAALI0 "not set" sentinel.
     */
    public String formatStartTime() {
        if (startTime == TLAHTO_NOT_SET) return "";
        int msOfDay = (int) (((long) startTime + 36L * 3_600_000) % MS_PER_DAY);
        int totalSeconds = msOfDay / 1000;
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        return String.format("%02d:%02d:%02d", hours, minutes, seconds);
    }

    @Override
    public String toString() {
        String badgeStr = badge > 0 ? String.valueOf(badge) : "-";
        return String.format("%4d  %-20s %-15s %-20s emit:%s  %s",
                kilpno, sukunimi, etunimi, seura, badgeStr, formatResult());
    }
}
