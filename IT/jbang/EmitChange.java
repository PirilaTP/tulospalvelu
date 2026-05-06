///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * Single-instance emit change → restart → verify persistence.
 * Mirrors test_emit_change.py.
 */

import java.nio.file.*;
import java.security.MessageDigest;

import static java.lang.System.out;

public class EmitChange {

    static final String COMPETITOR = "88";
    static final String OLD_EMIT = "15676";
    static final String NEW_EMIT = "123456";

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(60));
        out.println("TEST: Emit card change persists across restart");
        out.println("=".repeat(60));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found at " + Harness.HKMAALI);
            System.exit(1);
        }

        Path datadir = Harness.setupDataDir("jb_emit_tmp", "MA");
        Path kilpdat = datadir.resolve("KILP.DAT");
        String md5Before = md5(Files.readAllBytes(kilpdat));
        out.printf("%n1. Data: %s (md5: %s)%n", datadir, md5Before);

        // Phase 1: open, change emit
        out.printf("%n2. Changing emit %s -> %s...%n", OLD_EMIT, NEW_EMIT);
        try (Harness.HkMaali ma = new Harness.HkMaali(datadir)) {
            ma.start();
            ma.acceptAndWait();
            ma.navigateToKorjaa(COMPETITOR);
            ma.changeEmit(NEW_EMIT);
            out.println("   Edit complete.");
        }
        String md5After = md5(Files.readAllBytes(kilpdat));
        out.println("   KILP.DAT " + (!md5After.equals(md5Before) ? "modified" : "UNCHANGED"));

        // Phase 2: restart, verify
        out.println("\n3. Restarting to verify...");
        boolean startupOk = true;
        boolean emitFound = false;
        try (Harness.HkMaali ma2 = new Harness.HkMaali(datadir)) {
            ma2.start();
            ma2.acceptAndWait();
            if (ma2.hasStartupErrors()) {
                out.println("   FAIL: Startup error!");
                startupOk = false;
            } else {
                out.println("   Startup OK");
                ma2.navigateToKorjaa(COMPETITOR);
                String text = ma2.outputText();
                if (text.contains(NEW_EMIT)) {
                    emitFound = true;
                    out.println("   Emit card: " + NEW_EMIT + " ✓");
                } else if (text.contains(OLD_EMIT)) {
                    out.println("   Emit card: " + OLD_EMIT + " (unchanged)");
                } else {
                    out.println("   Could not verify emit value");
                }
            }
        }

        boolean success = startupOk && emitFound;
        out.println("\n" + "=".repeat(60));
        if (success) {
            out.println("RESULT: PASS ✓");
            Harness.deleteRecursive(datadir);
            System.exit(0);
        }
        out.println("RESULT: FAIL");
        System.exit(1);
    }

    private static String md5(byte[] data) throws Exception {
        byte[] digest = MessageDigest.getInstance("MD5").digest(data);
        StringBuilder sb = new StringBuilder();
        for (byte b : digest) sb.append(String.format("%02x", b));
        return sb.toString();
    }
}
