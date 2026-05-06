///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * Same topology as FourNodeWithWebadmin, but with real production data
 * (kisat/nikondataa, 763 kilpailijaa). Mirrors test_four_node_nikondata.py.
 *
 * KNOWN FAILING — and the cause is a test-data problem, NOT a Java client bug.
 *
 * Diagnosis (single-instance MA test, no peers):
 *  - kisat/nikondataa contains only KILP.DAT + KilpSrj.xml. No radat1.xml.
 *  - Without radat1.xml, HkMaali aborts at "Ratatietojen lukeminen ei
 *    onnistunut" before the main menu draws.
 *  - Borrowing radat1.xml from windowskonekonffit/HkMaaliData (the Python
 *    test's workaround) lets startup proceed past course loading, but the
 *    courses there don't match nikondata's classes. HkMaali then enters
 *    its "VIRHE: Kilpailijan X sarjaa Y tai rataa Z ei radoissa" emit-
 *    validation loop for nearly every competitor. The "J)atka, L)opeta"
 *    prompt fires once, we send L, the flood is suppressed — but valikko()
 *    is never reached: PÄÄVALIKKO, M)aali, Y)hteys etc. never appear in
 *    the PTY output even at 180 s timeout. Only the status panel redraws.
 *  - Phase A's K-K-416-ENTER is therefore sent into a dead screen. Phases
 *    B-D succeed because by then peers are forwarding KILPPVTs that MA
 *    accepts via the network handler regardless of UI state.
 *
 * Conclusion: the Java client (webadmin + pirila-comm) does not exhibit
 * any fault here. The same code paths are exercised end-to-end by
 * FourNodeWithWebadmin on HkKisaWinData. To make this test green, the
 * repo would need nikondata's actual radat1.xml committed alongside
 * KILP.DAT and KilpSrj.xml.
 */

import java.nio.file.*;
import java.util.*;

import static java.lang.System.out;

public class FourNodeNikondata {

    static final String COMPETITOR = "416";
    static final String EMIT_FROM_MA = "100001";
    static final String EMIT_FROM_WI = "200002";
    static final String EMIT_FROM_BE = "300003";
    static final String EMIT_FROM_WB = "400004";

    static final int PORT_MA_Y1 = 44901;
    static final int PORT_WI    = 44902;
    static final int PORT_MA_Y2 = 44911;
    static final int PORT_BE    = 44903;
    static final int PORT_MA_Y3 = 44921;
    static final int PORT_WB    = 44904;
    static final int HTTP_PORT  = 48093;

    static final Path NIKON_SRC = Harness.PROJECT_ROOT.resolve("kisat/nikondataa");
    static final Path NIKON_RADAT = Harness.PROJECT_ROOT.resolve(
            "windowskonekonffit/HkMaaliData/radat1.xml");
    static final Path NIKON_MERGED = Path.of("/tmp/nikon-merged");

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(64));
        out.println("TEST: 4-node sync with REAL production data (nikondataa)");
        out.println("=".repeat(64));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found");
            System.exit(1);
        }
        if (!Files.exists(NIKON_SRC.resolve("KILP.DAT"))) {
            out.println("FAIL: nikondataa missing at " + NIKON_SRC);
            System.exit(1);
        }

        // Stage a merged source dir: nikondataa + radat1.xml from
        // windowskonekonffit (HkMaali requires radat1.xml to start).
        Files.createDirectories(NIKON_MERGED);
        Files.copy(NIKON_SRC.resolve("KILP.DAT"), NIKON_MERGED.resolve("KILP.DAT"),
                StandardCopyOption.REPLACE_EXISTING);
        Files.copy(NIKON_SRC.resolve("KilpSrj.xml"), NIKON_MERGED.resolve("KilpSrj.xml"),
                StandardCopyOption.REPLACE_EXISTING);
        if (Files.exists(NIKON_RADAT)) {
            Files.copy(NIKON_RADAT, NIKON_MERGED.resolve("radat1.xml"),
                    StandardCopyOption.REPLACE_EXISTING);
        }

        Path dirMa = Harness.setupDataDir("jb_nk_MA", "MA", 1, NIKON_MERGED,
                new Harness.Connection(PORT_MA_Y1, "127.0.0.1", PORT_WI),
                new Harness.Connection(PORT_MA_Y2, "127.0.0.1", PORT_BE),
                new Harness.Connection(PORT_MA_Y3, "127.0.0.1", PORT_WB));
        Path dirWi = Harness.setupDataDir("jb_nk_WI", "WI", 1, NIKON_MERGED,
                new Harness.Connection(PORT_WI, "127.0.0.1", PORT_MA_Y1));
        Path dirBe = Harness.setupDataDir("jb_nk_BE", "BE", 1, NIKON_MERGED,
                new Harness.Connection(PORT_BE, "127.0.0.1", PORT_MA_Y2, "O"));
        Path dirWb = Harness.setupDataDir("jb_nk_WB", "WB", 1, NIKON_MERGED,
                new Harness.Connection(PORT_WB, "127.0.0.1", PORT_MA_Y3));

        out.printf("%nInstances (production data, kilpailija %s):%n", COMPETITOR);
        out.printf("  MA (hub): UDP %d + %d + %d%n", PORT_MA_Y1, PORT_MA_Y2, PORT_MA_Y3);
        out.printf("  WI: UDP %d%n", PORT_WI);
        out.printf("  BE (lähemit=O): UDP %d%n", PORT_BE);
        out.printf("  WB (webadmin): UDP %d, HTTP %d%n", PORT_WB, HTTP_PORT);

        Map<String, Boolean> results = new LinkedHashMap<>();
        try (Harness.HkMaali ma = new Harness.HkMaali(dirMa);
             Harness.HkMaali wi = new Harness.HkMaali(dirWi);
             Harness.HkMaali be = new Harness.HkMaali(dirBe);
             Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT)) {

            out.println("\n[1] Starting C++ instances...");
            ma.start(); wi.start(); be.start();
            try {
                ma.acceptAndWait(120); wi.acceptAndWait(120); be.acceptAndWait(120);
            } catch (Exception e) {
                ma.writeLog(Path.of("/tmp/jb-nk-MA.log"));
                wi.writeLog(Path.of("/tmp/jb-nk-WI.log"));
                be.writeLog(Path.of("/tmp/jb-nk-BE.log"));
                throw e;
            }
            out.println("    C++ running. Starting webadmin...");
            wb.start();
            // 763 kilpailijaa needs more settling time than the demo data —
            // status-panel redraws and per-record validation churn well past
            // acceptAndWait. Without this the very first navigateToKorjaa often
            // races against tail prompt activity.
            Harness.sleep(30_000);
            ma.read(2.0); wi.read(2.0); be.read(2.0);
            out.println("    All running.");

            // Phase A
            out.printf("%n[2] MA: emit %s → %s%n", COMPETITOR, EMIT_FROM_MA);
            ma.navigateToKorjaa(COMPETITOR);
            ma.changeEmit(EMIT_FROM_MA);
            ma.escapeToMain();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            wi.read(2.0); be.read(2.0);
            results.put("ma_to_wi", wi.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_MA));
            results.put("ma_to_be", be.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_MA));
            results.put("ma_to_wb", wb.checkEmit(COMPETITOR, EMIT_FROM_MA));
            for (var k : new String[]{"wi", "be", "wb"})
                out.printf("    %s: %s%n", k.toUpperCase(),
                        Harness.tag(results.get("ma_to_" + k)));

            // Phase B
            out.printf("%n[3] WI: emit %s → %s%n", COMPETITOR, EMIT_FROM_WI);
            wi.navigateToKorjaa(COMPETITOR);
            wi.changeEmit(EMIT_FROM_WI);
            wi.escapeToMain();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(2.0); be.read(2.0);
            results.put("wi_to_ma", ma.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_WI));
            results.put("wi_to_be", be.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_WI));
            results.put("wi_to_wb", wb.checkEmit(COMPETITOR, EMIT_FROM_WI));
            for (var k : new String[]{"ma", "be", "wb"})
                out.printf("    %s: %s%n", k.toUpperCase(),
                        Harness.tag(results.get("wi_to_" + k)));

            // Phase C
            out.printf("%n[4] BE: emit %s → %s%n", COMPETITOR, EMIT_FROM_BE);
            be.navigateToKorjaa(COMPETITOR);
            be.changeEmit(EMIT_FROM_BE);
            be.escapeToMain();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(2.0); wi.read(2.0);
            results.put("be_to_ma", ma.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_BE));
            results.put("be_to_wi", wi.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_BE));
            results.put("be_to_wb", wb.checkEmit(COMPETITOR, EMIT_FROM_BE));
            for (var k : new String[]{"ma", "wi", "wb"})
                out.printf("    %s: %s%n", k.toUpperCase(),
                        Harness.tag(results.get("be_to_" + k)));

            // Phase D
            out.printf("%n[5] WB: emit %s → %s via Playwright%n", COMPETITOR, EMIT_FROM_WB);
            boolean wbOk = wb.changeEmit(COMPETITOR, EMIT_FROM_WB);
            out.println("    Playwright submit: " + (wbOk ? "ok" : "FAIL"));
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(2.0); wi.read(2.0); be.read(2.0);
            results.put("wb_to_ma", ma.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_WB));
            results.put("wb_to_wi", wi.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_WB));
            results.put("wb_to_be", be.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_WB));
            for (var k : new String[]{"ma", "wi", "be"})
                out.printf("    %s: %s%n", k.toUpperCase(),
                        Harness.tag(results.get("wb_to_" + k)));

            ma.writeLog(Path.of("/tmp/jb-nk-MA.log"));
            wi.writeLog(Path.of("/tmp/jb-nk-WI.log"));
            be.writeLog(Path.of("/tmp/jb-nk-BE.log"));
        }

        boolean allPass = results.values().stream().allMatch(Boolean::booleanValue);
        out.println("\n" + "=".repeat(64));
        out.println("RESULTS:");
        for (var e : results.entrySet()) {
            String[] parts = e.getKey().split("_to_");
            out.printf("  %s -> %s: %s%n", parts[0].toUpperCase(),
                    parts[1].toUpperCase(), Harness.tag(e.getValue()));
        }
        out.println("\n" + (allPass ? "PASS ✓ (12/12 on production data)"
                : "FAIL — see /tmp/jb-nk-*.log"));
        if (allPass) {
            Harness.deleteRecursive(dirMa);
            Harness.deleteRecursive(dirWi);
            Harness.deleteRecursive(dirBe);
            Harness.deleteRecursive(dirWb);
        }
        System.exit(allPass ? 0 : 1);
    }
}
