///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * Three-node sync: 2x C++ HkMaali + 1x Java webadmin (star, MA = hub).
 * All 6 sync directions verified via emit card changes.
 * Mirrors test_three_node_sync.py.
 */

import java.nio.file.*;
import java.util.*;

import static java.lang.System.out;

public class ThreeNodeSync {

    static final String COMPETITOR = "88";
    static final String EMIT_A = "111111";   // MA sets
    static final String EMIT_B = "222222";   // WB sets via Playwright
    static final String EMIT_C = "333333";   // WI sets

    static final int PORT_MA = 41901;
    static final int PORT_WI = 41902;
    static final int PORT_WB = 41903;
    static final int HTTP_PORT = 48080;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(60));
        out.println("TEST: Three-node sync (2x HkMaali + webadmin)");
        out.println("=".repeat(60));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found");
            System.exit(1);
        }

        // Use the "pre-race" source so every competitor is open — matches the
        // real-world card-change scenario (runner shows up at start with the
        // wrong card).
        Path src = Harness.preRaceSourceData();
        Path dirMa = Harness.setupDataDir("jb_3node_MA", "MA", null, src,
                new Harness.Connection(PORT_MA, "127.0.0.1", PORT_WI),
                new Harness.Connection(PORT_MA + 10, "127.0.0.1", PORT_WB));
        Path dirWi = Harness.setupDataDir("jb_3node_WI", "WI", null, src,
                new Harness.Connection(PORT_WI, "127.0.0.1", PORT_MA));
        Path dirWb = Harness.setupDataDir("jb_3node_WB", "WB", null, src,
                new Harness.Connection(PORT_WB, "127.0.0.1", PORT_MA + 10));

        out.printf("%n1. Instances:%n");
        out.printf("   MA: UDP %d + %d%n", PORT_MA, PORT_MA + 10);
        out.printf("   WI: UDP %d%n", PORT_WI);
        out.printf("   WB: UDP %d, HTTP %d%n", PORT_WB, HTTP_PORT);

        Map<String, Boolean> results = new LinkedHashMap<>();
        try (Harness.HkMaali ma = new Harness.HkMaali(dirMa);
             Harness.HkMaali wi = new Harness.HkMaali(dirWi);
             Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT)) {
            out.println("\n2. Starting instances...");
            ma.start(); wi.start();
            ma.acceptAndWait(); wi.acceptAndWait();
            out.println("   Starting webadmin...");
            wb.start();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(1.0); wi.read(1.0);
            out.println("   All running.");

            // Phase A: MA -> WI, WB
            out.printf("%n3. MA: emit -> %s...%n", EMIT_A);
            ma.navigateToKorjaa(COMPETITOR);
            ma.changeEmit(EMIT_A);
            ma.escapeToMain();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            wi.read(2.0);

            out.println("\n4. Checking sync from MA...");
            results.put("ma_to_wi", wi.readCompetitorEmit(COMPETITOR).contains(EMIT_A));
            out.println("   WI: " + (results.get("ma_to_wi") ? EMIT_A + " ✓" : "FAIL"));
            results.put("ma_to_wb", wb.checkEmit(COMPETITOR, EMIT_A));
            out.println("   WB: " + (results.get("ma_to_wb") ? EMIT_A + " ✓" : "FAIL"));

            // Phase B: WB -> MA, WI
            out.printf("%n5. WB: emit -> %s via Playwright...%n", EMIT_B);
            boolean wbOk = wb.changeEmit(COMPETITOR, EMIT_B);
            out.println("   WB: " + (wbOk ? "submitted" : "FAILED!"));
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(2.0); wi.read(2.0);

            out.println("\n6. Checking sync from WB...");
            results.put("wb_to_ma", ma.readCompetitorEmit(COMPETITOR).contains(EMIT_B));
            out.println("   MA: " + (results.get("wb_to_ma") ? EMIT_B + " ✓" : "FAIL"));
            results.put("wb_to_wi", wi.readCompetitorEmit(COMPETITOR).contains(EMIT_B));
            out.println("   WI: " + (results.get("wb_to_wi") ? EMIT_B + " ✓" : "FAIL"));

            // Phase C: WI -> MA, WB
            out.printf("%n7. WI: emit -> %s...%n", EMIT_C);
            wi.navigateToKorjaa(COMPETITOR);
            wi.changeEmit(EMIT_C);
            wi.escapeToMain();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            ma.read(2.0);

            out.println("\n8. Checking sync from WI...");
            results.put("wi_to_ma", ma.readCompetitorEmit(COMPETITOR).contains(EMIT_C));
            out.println("   MA: " + (results.get("wi_to_ma") ? EMIT_C + " ✓" : "FAIL"));
            results.put("wi_to_wb", wb.checkEmit(COMPETITOR, EMIT_C));
            out.println("   WB: " + (results.get("wi_to_wb") ? EMIT_C + " ✓" : "FAIL"));
        }

        boolean allPass = results.values().stream().allMatch(Boolean::booleanValue);
        out.println("\n" + "=".repeat(60));
        out.println("RESULTS:");
        for (var e : results.entrySet()) {
            String[] parts = e.getKey().split("_to_");
            out.printf("  %s -> %s: %s%n", parts[0].toUpperCase(),
                    parts[1].toUpperCase(), Harness.tag(e.getValue()));
        }
        out.println("\n" + (allPass ? "PASS ✓ (all 6 directions)" : "FAIL"));
        if (allPass) {
            Harness.deleteRecursive(dirMa);
            Harness.deleteRecursive(dirWi);
            Harness.deleteRecursive(dirWb);
        }
        System.exit(allPass ? 0 : 1);
    }
}
