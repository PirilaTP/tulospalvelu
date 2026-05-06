///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * Two C++ HkMaali instances sync emit card changes via UDP.
 * Mirrors test_udp_sync.py.
 */

import java.nio.file.*;

import static java.lang.System.out;

public class UdpSync {

    static final String COMPETITOR = "88";
    static final String EMIT_A = "111111";
    static final String EMIT_B = "222222";

    static final int PORT_MA = 41901;
    static final int PORT_WI = 41902;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(60));
        out.println("TEST: Two-instance UDP sync of emit card changes");
        out.println("=".repeat(60));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found");
            System.exit(1);
        }

        Path dirMa = Harness.setupDataDir("jb_udp_MA", "MA",
                PORT_MA, "127.0.0.1", PORT_WI);
        Path dirWi = Harness.setupDataDir("jb_udp_WI", "WI",
                PORT_WI, "127.0.0.1", PORT_MA);
        out.printf("%n1. MA: %s (port %d)%n", dirMa, PORT_MA);
        out.printf("   WI: %s (port %d)%n", dirWi, PORT_WI);

        boolean syncA = false, syncB = false;
        try (Harness.HkMaali ma = new Harness.HkMaali(dirMa);
             Harness.HkMaali wi = new Harness.HkMaali(dirWi)) {
            out.println("\n2. Starting both instances...");
            ma.start(); wi.start();
            ma.acceptAndWait(); wi.acceptAndWait();
            out.println("   Waiting for UDP handshake...");
            Thread.sleep((Harness.SYNC_WAIT_SEC - 1) * 1000L);
            ma.read(1.0); wi.read(1.0);
            out.println("   Both running.");

            out.printf("%n3. MA: Changing emit -> %s...%n", EMIT_A);
            ma.navigateToKorjaa(COMPETITOR);
            ma.changeEmit(EMIT_A);
            ma.escapeToMain();
            Thread.sleep((Harness.SYNC_WAIT_SEC - 1) * 1000L);
            wi.read(2.0);

            out.println("\n4. WI: Checking emit...");
            String textWi = wi.readCompetitorEmit(COMPETITOR);
            syncA = textWi.contains(EMIT_A);
            out.println("   WI: " + (syncA ? EMIT_A + " ✓" : "FAIL"));

            out.printf("%n5. WI: Changing emit -> %s...%n", EMIT_B);
            wi.navigateToKorjaa(COMPETITOR);
            wi.changeEmit(EMIT_B);
            wi.escapeToMain();
            Thread.sleep((Harness.SYNC_WAIT_SEC - 1) * 1000L);
            ma.read(2.0);

            out.println("\n6. MA: Checking emit...");
            String textMa = ma.readCompetitorEmit(COMPETITOR);
            syncB = textMa.contains(EMIT_B);
            out.println("   MA: " + (syncB ? EMIT_B + " ✓" : "FAIL"));
        }

        out.println("\n" + "=".repeat(60));
        if (syncA && syncB) {
            out.println("RESULT: PASS ✓");
            out.println("  - MA -> WI: " + EMIT_A + " ✓");
            out.println("  - WI -> MA: " + EMIT_B + " ✓");
            Harness.deleteRecursive(dirMa);
            Harness.deleteRecursive(dirWi);
            System.exit(0);
        }
        out.println("RESULT: FAIL");
        System.exit(1);
    }
}
