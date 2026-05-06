///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.10
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * Kaikki osat valmiina → viimeinen osa päivittyy (käyttäjän syöte ei putoa).
 * setPvResult asettaa pv[1]:lle synteettisen tuloksen niin että molemmat
 * osat ovat "valmiit". TulospalveluService.sendCardChange:n default
 * startStage = npv-1 jää voimaan kun yksikään ei ole avoin.
 */

import fi.pirila.tulospalvelu.KilpReader;

import java.nio.file.*;

import static java.lang.System.out;

public class TwoDayAllDone {

    static final int KILPNO = 88;
    static final int NEW_BADGE = 999333;
    static final int PORT_MA = 44905;
    static final int PORT_WB = 44906;
    static final int HTTP_PORT = 48098;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(64));
        out.println("TEST: all stages already decided → only last stage updates");
        out.println("=".repeat(64));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found at " + Harness.HKMAALI);
            System.exit(1);
        }

        Path dirMa = Harness.setupDataDir("jb_2day_ad_MA", "MA",
                PORT_MA, "127.0.0.1", PORT_WB);
        Path dirWb = Harness.setupDataDir("jb_2day_ad_WB", "WB",
                PORT_WB, "127.0.0.1", PORT_MA);
        Path kilpMa = dirMa.resolve("KILP.DAT");
        Path kilpWb = dirWb.resolve("KILP.DAT");

        KilpReader.detectRecordSize(kilpMa);
        if (KilpReader.getNpv() != 2) {
            out.println("FAIL: expected n_pv=2");
            System.exit(1);
        }

        int record = Harness.findRecordByKilpno(kilpMa, KILPNO);
        if (record < 0) { out.println("FAIL: kilpno=" + KILPNO + " not found"); System.exit(1); }

        // pv[0] is already 'T'+time in HkKisaWinData; add a synthetic result on
        // pv[1] (both copies) so every stage is decided.
        for (Path p : new Path[]{kilpMa, kilpWb}) {
            Harness.setPvResult(p, record, 1, 'T', 4_200_000, 3);
        }

        var s0 = Harness.readPvStatus(kilpMa, record, 0);
        var s1 = Harness.readPvStatus(kilpMa, record, 1);
        out.println("  pv[0] status: " + s0);
        out.println("  pv[1] status: " + s1 + "  (after setPvResult)");
        if (s0.keskhyl() != 'T' || s1.keskhyl() != 'T') {
            out.println("FAIL: precondition setup did not stick");
            System.exit(1);
        }

        int pv0Before = Harness.readPvBadge(kilpMa, record, 0);
        int pv1Before = Harness.readPvBadge(kilpMa, record, 1);
        out.printf("  Before: pv[0].badge=%d, pv[1].badge=%d%n", pv0Before, pv1Before);

        boolean success = false;
        Harness.HkMaali ma = new Harness.HkMaali(dirMa);
        Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT);
        try {
            out.println("\n[1] Starting MA...");
            ma.start();
            try { ma.acceptAndWait(); }
            catch (Exception e) {
                ma.writeLog(Path.of("/tmp/jb-2day-ad-MA.log"));
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
            boolean pv0Unchanged = pv0After == pv0Before;
            boolean pv1Updated = pv1After == NEW_BADGE;
            out.printf("    pv[0].badge: %d → %d  %s%n", pv0Before, pv0After,
                    pv0Unchanged ? "✓ ennallaan" : "FAIL — ei pitäisi muuttua");
            out.printf("    pv[1].badge: %d → %d  %s%n", pv1Before, pv1After, Harness.tag(pv1Updated));
            success = pv0Unchanged && pv1Updated;

            ma.writeLog(Path.of("/tmp/jb-2day-ad-MA.log"));
        } finally {
            ma.close();
            wb.close();
        }

        out.println("\n" + "=".repeat(64));
        if (success) {
            out.println("RESULT: PASS ✓ — viimeinen osa päivittyi, muut ennallaan");
            Harness.deleteRecursive(dirMa);
            Harness.deleteRecursive(dirWb);
            System.exit(0);
        }
        out.println("RESULT: FAIL — see /tmp/jb-2day-ad-MA.log + /tmp/webadmin-" + HTTP_PORT + ".log");
        System.exit(1);
    }
}
