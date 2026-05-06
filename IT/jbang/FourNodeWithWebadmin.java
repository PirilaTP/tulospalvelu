///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * 4-node test: 3x C++ HkMaali (MA, WI, BE-lähemit=O) + Java webadmin.
 * Verifies card change propagates in all 12 directions even with a
 * leimantarkastus node (BE) that uses lähemit=O (one-way emit transport).
 * Mirrors test_four_node_with_webadmin.py.
 */

import java.nio.file.*;
import java.util.*;

import static java.lang.System.out;

public class FourNodeWithWebadmin {

    static final String COMPETITOR = "88";
    static final String EMIT_FROM_MA = "100001";
    static final String EMIT_FROM_WI = "200002";
    static final String EMIT_FROM_BE = "300003";
    static final String EMIT_FROM_WB = "400004";

    static final int PORT_MA_Y1 = 43901;
    static final int PORT_WI    = 43902;
    static final int PORT_MA_Y2 = 43911;
    static final int PORT_BE    = 43903;
    static final int PORT_MA_Y3 = 43921;
    static final int PORT_WB    = 43904;
    static final int HTTP_PORT  = 48092;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(64));
        out.println("TEST: 4-node sync — 3× HkMaali + webadmin (BE: lähemit=O)");
        out.println("=".repeat(64));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found");
            System.exit(1);
        }

        Path src = Harness.preRaceSourceData();
        Path dirMa = Harness.setupDataDir("jb_4nw_MA", "MA", null, src,
                new Harness.Connection(PORT_MA_Y1, "127.0.0.1", PORT_WI),
                new Harness.Connection(PORT_MA_Y2, "127.0.0.1", PORT_BE),
                new Harness.Connection(PORT_MA_Y3, "127.0.0.1", PORT_WB));
        Path dirWi = Harness.setupDataDir("jb_4nw_WI", "WI", null, src,
                new Harness.Connection(PORT_WI, "127.0.0.1", PORT_MA_Y1));
        Path dirBe = Harness.setupDataDir("jb_4nw_BE", "BE", null, src,
                new Harness.Connection(PORT_BE, "127.0.0.1", PORT_MA_Y2, "O"));
        Path dirWb = Harness.setupDataDir("jb_4nw_WB", "WB", null, src,
                new Harness.Connection(PORT_WB, "127.0.0.1", PORT_MA_Y3));

        out.println("\nInstances:");
        out.printf("  MA (hub):         UDP %d + %d + %d%n", PORT_MA_Y1, PORT_MA_Y2, PORT_MA_Y3);
        out.printf("  WI (kisalaskuri): UDP %d%n", PORT_WI);
        out.printf("  BE (lähemit=O):   UDP %d%n", PORT_BE);
        out.printf("  WB (webadmin):    UDP %d, HTTP %d%n", PORT_WB, HTTP_PORT);

        Map<String, Boolean> results = new LinkedHashMap<>();
        try (Harness.HkMaali ma = new Harness.HkMaali(dirMa);
             Harness.HkMaali wi = new Harness.HkMaali(dirWi);
             Harness.HkMaali be = new Harness.HkMaali(dirBe);
             Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT)) {
            out.println("\n[1] Starting C++ instances...");
            ma.start(); wi.start(); be.start();
            ma.acceptAndWait(); wi.acceptAndWait(); be.acceptAndWait();
            out.println("    C++ running. Starting webadmin...");
            wb.start();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(1.0); wi.read(1.0); be.read(1.0);
            out.println("    All running.");

            // Phase A: MA → WI + BE + WB
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

            // Phase B: WI → MA + BE + WB
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

            // Phase C: BE → MA + WI + WB
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

            // Phase D: WB (webadmin) → MA + WI + BE
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

            // Dump logs for diagnostics
            ma.writeLog(Path.of("/tmp/jb-4nw-MA.log"));
            wi.writeLog(Path.of("/tmp/jb-4nw-WI.log"));
            be.writeLog(Path.of("/tmp/jb-4nw-BE.log"));
        }

        boolean allPass = results.values().stream().allMatch(Boolean::booleanValue);
        out.println("\n" + "=".repeat(64));
        out.println("RESULTS:");
        for (var e : results.entrySet()) {
            String[] parts = e.getKey().split("_to_");
            out.printf("  %s -> %s: %s%n", parts[0].toUpperCase(),
                    parts[1].toUpperCase(), Harness.tag(e.getValue()));
        }
        out.println("\n" + (allPass ? "PASS ✓ (12/12 sync directions)"
                : "FAIL — see /tmp/jb-4nw-*.log"));
        if (allPass) {
            Harness.deleteRecursive(dirMa);
            Harness.deleteRecursive(dirWi);
            Harness.deleteRecursive(dirBe);
            Harness.deleteRecursive(dirWb);
        }
        System.exit(allPass ? 0 : 1);
    }
}
