///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * 4-node C++-only sync test with a `lähemit=O` stamp-check node (BE).
 * Topology: MA hub talks to WI and BE. Verifies that BE's lähemit=O setting
 * does not block KILPPVT receipt — only EMITT/EMITVA. Mirrors
 * test_four_node_lahemit_o.py.
 */

import java.nio.file.*;
import java.util.*;

import static java.lang.System.out;

public class FourNodeLahemitO {

    static final String COMPETITOR = "88";
    static final String EMIT_FROM_MA = "111111";
    static final String EMIT_FROM_WI = "222222";
    static final String EMIT_FROM_BE = "333333";

    static final int PORT_MA_Y1 = 42901;
    static final int PORT_WI    = 42902;
    static final int PORT_MA_Y2 = 42911;
    static final int PORT_BE    = 42903;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(64));
        out.println("TEST: 4-node sync with lähemit=O stamp-check (BE)");
        out.println("=".repeat(64));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found");
            System.exit(1);
        }

        Path dirMa = Harness.setupDataDir("jb_4node_MA", "MA",
                new Harness.Connection(PORT_MA_Y1, "127.0.0.1", PORT_WI),
                new Harness.Connection(PORT_MA_Y2, "127.0.0.1", PORT_BE));
        Path dirWi = Harness.setupDataDir("jb_4node_WI", "WI",
                new Harness.Connection(PORT_WI, "127.0.0.1", PORT_MA_Y1));
        Path dirBe = Harness.setupDataDir("jb_4node_BE", "BE",
                new Harness.Connection(PORT_BE, "127.0.0.1", PORT_MA_Y2, "O"));

        out.println("\nInstances:");
        out.printf("  MA (hub): UDP %d + %d%n", PORT_MA_Y1, PORT_MA_Y2);
        out.printf("  WI: UDP %d%n", PORT_WI);
        out.printf("  BE: UDP %d, lähemit=O%n", PORT_BE);

        Map<String, Boolean> results = new LinkedHashMap<>();
        try (Harness.HkMaali ma = new Harness.HkMaali(dirMa);
             Harness.HkMaali wi = new Harness.HkMaali(dirWi);
             Harness.HkMaali be = new Harness.HkMaali(dirBe)) {
            out.println("\n[1] Starting instances...");
            ma.start(); wi.start(); be.start();
            ma.acceptAndWait(); wi.acceptAndWait(); be.acceptAndWait();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(1.0); wi.read(1.0); be.read(1.0);
            out.println("    Running.");

            // Phase A: MA → WI + BE
            out.printf("%n[2] MA: emit %s → %s%n", COMPETITOR, EMIT_FROM_MA);
            ma.navigateToKorjaa(COMPETITOR);
            ma.changeEmit(EMIT_FROM_MA);
            ma.escapeToMain();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            wi.read(2.0); be.read(2.0);
            results.put("ma_to_wi", wi.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_MA));
            results.put("ma_to_be", be.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_MA));
            out.println("    WI: " + Harness.tag(results.get("ma_to_wi")));
            out.println("    BE: " + Harness.tag(results.get("ma_to_be")));

            // Phase B: WI → MA + BE
            out.printf("%n[3] WI: emit %s → %s%n", COMPETITOR, EMIT_FROM_WI);
            wi.navigateToKorjaa(COMPETITOR);
            wi.changeEmit(EMIT_FROM_WI);
            wi.escapeToMain();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(2.0); be.read(2.0);
            results.put("wi_to_ma", ma.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_WI));
            results.put("wi_to_be", be.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_WI));
            out.println("    MA: " + Harness.tag(results.get("wi_to_ma")));
            out.println("    BE: " + Harness.tag(results.get("wi_to_be")));

            // Phase C: BE → MA + WI (lähemit=O does not block BE's outgoing KILPPVT)
            out.printf("%n[4] BE: emit %s → %s%n", COMPETITOR, EMIT_FROM_BE);
            be.navigateToKorjaa(COMPETITOR);
            be.changeEmit(EMIT_FROM_BE);
            be.escapeToMain();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(2.0); wi.read(2.0);
            results.put("be_to_ma", ma.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_BE));
            results.put("be_to_wi", wi.readCompetitorEmit(COMPETITOR).contains(EMIT_FROM_BE));
            out.println("    MA: " + Harness.tag(results.get("be_to_ma")));
            out.println("    WI: " + Harness.tag(results.get("be_to_wi")));

            ma.writeLog(Path.of("/tmp/jb-4node-MA.log"));
            wi.writeLog(Path.of("/tmp/jb-4node-WI.log"));
            be.writeLog(Path.of("/tmp/jb-4node-BE.log"));
        }

        boolean allPass = results.values().stream().allMatch(Boolean::booleanValue);
        out.println("\n" + "=".repeat(64));
        out.println("RESULTS:");
        for (var e : results.entrySet()) {
            String[] parts = e.getKey().split("_to_");
            out.printf("  %s -> %s: %s%n", parts[0].toUpperCase(),
                    parts[1].toUpperCase(), Harness.tag(e.getValue()));
        }
        out.println("\n" + (allPass ? "PASS ✓" : "FAIL — see /tmp/jb-4node-*.log"));
        if (allPass) {
            Harness.deleteRecursive(dirMa);
            Harness.deleteRecursive(dirWi);
            Harness.deleteRecursive(dirBe);
        }
        System.exit(allPass ? 0 : 1);
    }
}
