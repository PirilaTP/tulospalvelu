///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * 2-koneen JBang-IT: kahden päivän kortinvaihto, molemmat osat avoinna.
 *
 * Webadmin (WB) lähettää KILPPVT pv=0 ja pv=1 kun molemmat osat ovat
 * avoinna. Verifikaatio lukee MA:n KILP.DAT:n suoraan KilpReaderilla.
 */

import fi.pirila.tulospalvelu.KilpReader;

import java.nio.file.*;

import static java.lang.System.out;

public class TwoDayBothOpen {

    static final int KILPNO = 88;
    static final int NEW_BADGE = 999111;
    static final int PORT_MA = 44901;
    static final int PORT_WB = 44902;
    static final int HTTP_PORT = 48096;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(64));
        out.println("TEST: 2-day card change → both stages updated");
        out.println("=".repeat(64));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found at " + Harness.HKMAALI);
            System.exit(1);
        }

        Path dirMa = Harness.setupDataDir("jb_2day_MA", "MA",
                PORT_MA, "127.0.0.1", PORT_WB);
        Path dirWb = Harness.setupDataDir("jb_2day_WB", "WB",
                PORT_WB, "127.0.0.1", PORT_MA);
        Path kilpMa = dirMa.resolve("KILP.DAT");
        Path kilpWb = dirWb.resolve("KILP.DAT");

        // Layout sanity (n_pv must be 2)
        int reclen = KilpReader.detectRecordSize(kilpMa);
        int kilppvtpsize = KilpReader.getKilppvtpsize();
        int npv = KilpReader.getNpv();
        out.printf("%n  Layout: reclen=%d kilppvtpsize=%d n_pv=%d%n",
                reclen, kilppvtpsize, npv);
        if (npv != 2) {
            out.println("FAIL: expected n_pv=2");
            System.exit(1);
        }

        int record = Harness.findRecordByKilpno(kilpMa, KILPNO);
        if (record < 0) { out.println("FAIL: kilpno=" + KILPNO + " not found"); System.exit(1); }
        out.println("  Test competitor kilpno=" + KILPNO + " at record_index=" + record);

        // Make both stages "open" so the change must propagate to both.
        for (Path p : new Path[]{kilpMa, kilpWb}) {
            Harness.clearPvStatus(p, record, 0);
            Harness.clearPvStatus(p, record, 1);
        }

        int pv0Before = Harness.readPvBadge(kilpMa, record, 0);
        int pv1Before = Harness.readPvBadge(kilpMa, record, 1);
        out.printf("  Before: pv[0].badge=%d, pv[1].badge=%d%n", pv0Before, pv1Before);

        boolean success = false;
        Harness.HkMaali ma = new Harness.HkMaali(dirMa);
        Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT);
        try {
            out.println("\n[1] Starting MA (HkMaali)...");
            ma.start();
            try {
                ma.acceptAndWait();
            } catch (Exception e) {
                ma.writeLog(Path.of("/tmp/jb-2day-MA.log"));
                out.println("    acceptAndWait failed: " + e.getMessage());
                out.println("    HkMaali output saved to /tmp/jb-2day-MA.log");
                throw e;
            }
            out.println("    MA at main menu.");

            out.println("\n[2] Starting webadmin...");
            wb.start();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(1.0);

            out.printf("%n[3] Webadmin: kilpno %d emit → %d%n", KILPNO, NEW_BADGE);
            boolean ok = wb.changeEmit(String.valueOf(KILPNO), String.valueOf(NEW_BADGE));
            out.println("    Playwright submit: " + (ok ? "ok" : "FAIL"));
            if (!ok) System.exit(1);

            Harness.sleep((Harness.SYNC_WAIT_SEC + 2) * 1000L);
            ma.read(2.0);

            out.println("\n[4] Verifying MA's KILP.DAT...");
            int pv0After = Harness.readPvBadge(kilpMa, record, 0);
            int pv1After = Harness.readPvBadge(kilpMa, record, 1);
            boolean pv0Ok = pv0After == NEW_BADGE;
            boolean pv1Ok = pv1After == NEW_BADGE;
            out.printf("    pv[0].badge: %d → %d  %s%n", pv0Before, pv0After, Harness.tag(pv0Ok));
            out.printf("    pv[1].badge: %d → %d  %s%n", pv1Before, pv1After, Harness.tag(pv1Ok));
            success = pv0Ok && pv1Ok;

            ma.writeLog(Path.of("/tmp/jb-2day-MA.log"));
        } finally {
            ma.close();
            wb.close();
        }

        out.println("\n" + "=".repeat(64));
        if (success) {
            out.println("RESULT: PASS ✓ — molemmat osat saivat uuden kortin");
            Harness.deleteRecursive(dirMa);
            Harness.deleteRecursive(dirWb);
            System.exit(0);
        }
        out.println("RESULT: FAIL — see /tmp/jb-2day-MA.log + /tmp/webadmin-" + HTTP_PORT + ".log");
        System.exit(1);
    }
}
