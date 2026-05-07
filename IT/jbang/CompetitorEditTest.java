///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * 2-koneen IT: webadminin Competitor List -näkymän edit-formi.
 * Otetaan vakant kilpailija (kilpno >= 9980 nikonserver-datassa, sarja=51 = VAKANTIT),
 * vaihdetaan se "oikeaksi" kilpailijaksi (etunimi+sukunimi+seura+sarja) ja
 * varmistetaan että muutos välittyy MA:n KILP.DAT:iin KILPT-viestillä.
 */

import fi.pirila.tulospalvelu.KilpReader;
import fi.pirila.tulospalvelu.Competitor;

import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.List;

import static java.lang.System.out;

public class CompetitorEditTest {

    static final String VAKANT_KILPNO = "9995";
    static final String NEW_ETUNIMI   = "Testi";
    static final String NEW_SUKUNIMI  = "Henkilo";
    static final String NEW_SEURA     = "Testi Klubi";
    static final String NEW_SARJA     = "H21A";      // must exist in nikonserver KilpSrj.xml
    static final String NEW_BADGE     = "777111";    // optional badge change

    static final int PORT_MA = 45901;
    static final int PORT_WB = 45902;
    static final int HTTP_PORT = 48095;

    static final Path NIKON_SRC = Harness.PROJECT_ROOT.resolve("kisat/nikonserver/data");

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(70));
        out.println("TEST: Competitor List edit form — vakant → real competitor");
        out.println("=".repeat(70));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found");
            System.exit(1);
        }
        if (!Files.exists(NIKON_SRC.resolve("KILP.DAT"))) {
            out.println("FAIL: nikonserver dataset missing");
            System.exit(1);
        }

        Path dirMa = Harness.setupDataDir("jb_edit_MA", "MA", 1, NIKON_SRC,
                new Harness.Connection(PORT_MA, "127.0.0.1", PORT_WB));
        Path dirWb = Harness.setupDataDir("jb_edit_WB", "WB", 1, NIKON_SRC,
                new Harness.Connection(PORT_WB, "127.0.0.1", PORT_MA));

        Path kilpMa = dirMa.resolve("KILP.DAT");
        int recordIdx = Harness.findRecordByKilpno(kilpMa, Integer.parseInt(VAKANT_KILPNO));
        out.printf("%n  Vakant competitor kilpno=%s at record_index=%d%n",
                VAKANT_KILPNO, recordIdx);

        // Verify it's actually vakant (sarja = 51 in nikonserver, the VAKANTIT class)
        var competitors = KilpReader.read(kilpMa);
        Competitor vakant = competitors.stream()
                .filter(c -> c.recordIndex == recordIdx).findFirst().orElseThrow();
        out.printf("  Before: sarja=%d sukunimi=%s etunimi=%s seura=%s%n",
                vakant.sarja, repr(vakant.sukunimi), repr(vakant.etunimi), repr(vakant.seura));

        // Resolve the target sarja's index from KilpSrj.xml so we can verify after
        fi.pirila.tulospalvelu.KilpSrjReader srj = new fi.pirila.tulospalvelu.KilpSrjReader();
        srj.read(NIKON_SRC.resolve("KilpSrj.xml"));
        Integer targetSarjaIdx = srj.getAllClasses().entrySet().stream()
                .filter(e -> NEW_SARJA.equals(e.getValue()))
                .map(java.util.Map.Entry::getKey).findFirst().orElse(null);
        if (targetSarjaIdx == null) {
            out.println("FAIL: target sarja '" + NEW_SARJA + "' not in KilpSrj.xml");
            System.exit(1);
        }
        out.printf("  Target sarja '%s' = sarja index %d%n", NEW_SARJA, targetSarjaIdx);

        boolean success = false;
        try (Harness.HkMaali ma = new Harness.HkMaali(dirMa);
             Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT)) {

            out.println("\n[1] Starting MA...");
            ma.start();
            try { ma.acceptAndWait(120); }
            catch (Exception e) { ma.writeLog(Path.of("/tmp/jb-edit-MA.log")); throw e; }
            out.println("    MA at main menu.");

            out.println("\n[2] Starting webadmin...");
            wb.start();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 2 * 1000L);
            ma.read(2.0);

            out.printf("%n[3] Edit kilpno=%s → %s %s, %s, %s, badge=%s%n",
                    VAKANT_KILPNO, NEW_ETUNIMI, NEW_SUKUNIMI, NEW_SEURA, NEW_SARJA, NEW_BADGE);
            boolean ok = wb.editCompetitor(VAKANT_KILPNO, NEW_ETUNIMI, NEW_SUKUNIMI,
                    NEW_SEURA, NEW_SARJA, NEW_BADGE);
            out.println("    Playwright submit: " + (ok ? "ok" : "FAIL"));
            if (!ok) System.exit(1);

            Harness.sleep((Harness.SYNC_WAIT_SEC + 2) * 1000L);
            ma.read(2.0);

            out.println("\n[4] Verifying MA's KILP.DAT...");
            var after = KilpReader.read(kilpMa).stream()
                    .filter(c -> c.recordIndex == recordIdx).findFirst().orElseThrow();
            int afterBadge = Harness.readPvBadge(kilpMa, recordIdx, 0);
            char afterKeskhyl = Harness.readPvStatus(kilpMa, recordIdx, 0).keskhyl();
            out.printf("    sukunimi: %s → %s%n", repr(vakant.sukunimi), repr(after.sukunimi));
            out.printf("    etunimi:  %s → %s%n", repr(vakant.etunimi), repr(after.etunimi));
            out.printf("    seura:    %s → %s%n", repr(vakant.seura), repr(after.seura));
            out.printf("    sarja:    %d → %d%n", vakant.sarja, after.sarja);
            out.printf("    badge:    %d → %d%n", vakant.badge, afterBadge);
            out.printf("    keskhyl:  '%s' → '%s' (V → open expected)%n",
                    vakant.keskhyl == 0 ? "" : String.valueOf(vakant.keskhyl),
                    afterKeskhyl == 0 ? "" : String.valueOf(afterKeskhyl));

            boolean okSukunimi = NEW_SUKUNIMI.equals(after.sukunimi);
            boolean okEtunimi = NEW_ETUNIMI.equals(after.etunimi);
            boolean okSeura = NEW_SEURA.equals(after.seura);
            boolean okSarja = after.sarja == targetSarjaIdx;
            boolean okBadge = afterBadge == Integer.parseInt(NEW_BADGE);
            // Vakantti ('V') should auto-clear to '-' (open) once real
            // competitor data is entered. Some C++ versions leave it as NUL.
            boolean okKeskhyl = afterKeskhyl == '-' || afterKeskhyl == 0;
            out.println("    " + Harness.tag(okSukunimi) + " sukunimi");
            out.println("    " + Harness.tag(okEtunimi) + " etunimi");
            out.println("    " + Harness.tag(okSeura) + " seura");
            out.println("    " + Harness.tag(okSarja) + " sarja");
            out.println("    " + Harness.tag(okBadge) + " badge");
            out.println("    " + Harness.tag(okKeskhyl) + " keskhyl auto-cleared");
            success = okSukunimi && okEtunimi && okSeura && okSarja && okBadge && okKeskhyl;
            ma.writeLog(Path.of("/tmp/jb-edit-MA.log"));
        }

        out.println("\n" + "=".repeat(70));
        if (success) {
            out.println("RESULT: PASS ✓ — vakant assigned via edit form, MA's KILP.DAT updated");
            Harness.deleteRecursive(dirMa);
            Harness.deleteRecursive(dirWb);
            System.exit(0);
        }
        out.println("RESULT: FAIL — see /tmp/jb-edit-MA.log + /tmp/webadmin-" + HTTP_PORT + ".log");
        System.exit(1);
    }

    private static String repr(String s) { return s == null ? "null" : "'" + s + "'"; }
}
