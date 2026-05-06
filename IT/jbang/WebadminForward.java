///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * Reproduces the production forwarding bug:
 *     webadmin → MA → { WI, M2 (lähemit=O) }
 * The bug hypothesis was that M2 (a leimantarkastuskone with one-way emit
 * config) might miss webadmin-originated KILPPVT updates. This test confirms
 * the forwarding works on all three branches. Mirrors test_webadmin_forward.py.
 *
 * Uses real production data from windowskonekonffit/HkMaaliData if available.
 */

import java.nio.file.*;

import static java.lang.System.out;

public class WebadminForward {

    // Topology fix vs the original Python test: WB needs its own port on MA.
    // Sharing 41901 between WI and WB caused MA's yhteys1 cliaddr to be hijacked
    // by whichever peer pinged most recently, so MA's KILPPVT forwards never
    // reached WI.
    static final int PORT_MA_Y1 = 41901;   // MA listens, WI sends here
    static final int PORT_WI_Y2 = 41902;   // WI listens, MA sends here
    static final int PORT_MA_Y2 = 41912;   // MA listens for WB (separate from WI's port)
    static final int PORT_MA_Y3 = 41903;   // MA listens for M2
    static final int PORT_M2_Y1 = 41913;   // M2 listens
    static final int PORT_WB_Y1 = 41921;   // webadmin listens
    static final int HTTP_PORT  = 48081;

    static final String COMPETITOR = "94";
    static final String NEW_EMIT   = "77777";

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(70));
        out.println("TEST: webadmin → MA → {WI, M2} forwarding");
        out.println("=".repeat(70));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found");
            System.exit(1);
        }
        // Production layout uses windowskonekonffit/HkMaaliData (which is just a
        // copy of HkKisaWinData — same 17976-byte demo file). For the emit-change
        // path to exercise pv[0] (the only stage the C++ TUI displays in PÄIVÄ=1
        // mode), the runner must have no results yet → use the pre-race source.
        Path src = Harness.preRaceSourceData();
        Path dirMa = Harness.setupDataDirRaw("jb_fwd_MA",
                "Kone=MA\nEmit\nPÄIVÄ=1\n"
                + "yhteys1=udp:" + PORT_MA_Y1 + "/localhost:" + PORT_WI_Y2 + "\n"
                + "lähemit1\n"
                + "yhteys2=udp:" + PORT_MA_Y2 + "/localhost:" + PORT_WB_Y1 + "\n"
                + "lähemit2\n"
                + "yhteys3=udp:" + PORT_MA_Y3 + "/localhost:" + PORT_M2_Y1 + "\n"
                + "lähemit3\n",
                src);
        Path dirWi = Harness.setupDataDirRaw("jb_fwd_WI",
                "Kone=WI\nEmit\nPÄIVÄ=1\n"
                + "yhteys2=udp:" + PORT_WI_Y2 + "/localhost:" + PORT_MA_Y1 + "\n"
                + "lähemit2\n",  // bidirectional: WI is the kisalaskuri, must receive
                src);
        Path dirM2 = Harness.setupDataDirRaw("jb_fwd_M2",
                "Kone=M2\nEmit\nPÄIVÄ=1\n"
                + "yhteys1=udp:" + PORT_M2_Y1 + "/localhost:" + PORT_MA_Y3 + "\n"
                + "lähemit1=O\n",
                src);
        Path dirWb = Harness.setupDataDirRaw("jb_fwd_WB",
                "Kone=WB\nEmit\nPÄIVÄ=1\n"
                + "yhteys1=udp:" + PORT_WB_Y1 + "/localhost:" + PORT_MA_Y2 + "\n"
                + "lähemit1\n",
                src);

        out.printf("%nTopology:%n");
        out.printf("  MA (hub)         listens %d (y1→WI) + %d (y2→WB) + %d (y3→M2)%n",
                PORT_MA_Y1, PORT_MA_Y2, PORT_MA_Y3);
        out.printf("  WI (kisalaskuri) listens %d (y2→MA)%n", PORT_WI_Y2);
        out.printf("  M2 (leimtark.)   listens %d (y1→MA) lähemit1=O%n", PORT_M2_Y1);
        out.printf("  WB (webadmin)    listens %d (y1→MA y2) HTTP %d%n", PORT_WB_Y1, HTTP_PORT);

        boolean maOk = false, wiOk = false, m2Ok = false;
        try (Harness.HkMaali ma = new Harness.HkMaali(dirMa);
             Harness.HkMaali wi = new Harness.HkMaali(dirWi);
             Harness.HkMaali m2 = new Harness.HkMaali(dirM2);
             Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT)) {

            out.println("\n[1] Starting HkMaali instances (MA, WI, M2)...");
            ma.start(); wi.start(); m2.start();
            ma.acceptAndWait(); wi.acceptAndWait(); m2.acceptAndWait();
            out.println("    Accepted initial screens.");

            out.println("[2] Starting webadmin...");
            wb.start();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(1.0); wi.read(1.0); m2.read(1.0);

            out.printf("%n[3] Change emit via webadmin: kilpno=%s new=%s%n", COMPETITOR, NEW_EMIT);
            boolean ok = wb.changeEmit(COMPETITOR, NEW_EMIT);
            out.println("    submitted=" + ok);
            Harness.sleep(Harness.SYNC_WAIT_SEC * 2 * 1000L);
            ma.read(2.0); wi.read(2.0); m2.read(2.0);

            out.println("\n[4] Verify emit value propagated to each instance:");
            String maText = ma.readCompetitorEmit(COMPETITOR);
            String wiText = wi.readCompetitorEmit(COMPETITOR);
            String m2Text = m2.readCompetitorEmit(COMPETITOR);
            maOk = maText.contains(NEW_EMIT);
            wiOk = wiText.contains(NEW_EMIT);
            m2Ok = m2Text.contains(NEW_EMIT);
            out.println("    MA: " + Harness.tag(maOk) + "   (webadmin → MA direct)");
            out.println("    WI: " + Harness.tag(wiOk) + "   (MA forwards to WI)");
            out.println("    M2: " + Harness.tag(m2Ok) + "   (MA forwards to M2 — the bug!)");

            ma.writeLog(Path.of("/tmp/jb-fwd-MA.log"));
            wi.writeLog(Path.of("/tmp/jb-fwd-WI.log"));
            m2.writeLog(Path.of("/tmp/jb-fwd-M2.log"));
        }

        boolean success = maOk && wiOk && m2Ok;
        out.println("\n" + "=".repeat(70));
        out.println(success ? "RESULT: PASS ✓" : "RESULT: FAIL — see /tmp/jb-fwd-*.log");
        if (success) {
            Harness.deleteRecursive(dirMa);
            Harness.deleteRecursive(dirWi);
            Harness.deleteRecursive(dirM2);
            Harness.deleteRecursive(dirWb);
        }
        System.exit(success ? 0 : 1);
    }
}
