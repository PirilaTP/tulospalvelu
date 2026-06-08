///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * Generates a ready-to-run "two-machine competition" for manual testing.
 *
 * Two data directories connected over localhost UDP:
 *   - test_data_server (Kone=SE) — for the C++ HkMaali timing server
 *   - test_data_J1     (Kone=J1) — for the webadmin (reads this dir)
 *
 * The start list is seeded with one randomly chosen starter per minute, from
 * the current minute up to the top of the next hour (override the count with a
 * CLI argument). Every other competitor is left without a start time. The
 * source data is the all-open "pre-race" derivative, so the starters show up as
 * "Avoin" with a start time — exactly the state the DNS app and the
 * /api/v1/competitors/{kilpno}/dns endpoint operate on.
 *
 * Unlike the IT test scripts this does NOT start any process or delete the
 * directories afterwards — it just lays down data for you to launch by hand.
 *
 * Usage:
 *   jbang GenerateTwoMachineRace.java            # now → next full hour
 *   jbang GenerateTwoMachineRace.java 60         # 60 one-minute slots
 */

import fi.pirila.tulospalvelu.Competitor;
import fi.pirila.tulospalvelu.KilpReader;

import java.nio.file.*;
import java.time.Duration;
import java.time.LocalTime;
import java.time.temporal.ChronoUnit;
import java.util.*;

import static java.lang.System.out;

public class GenerateTwoMachineRace {

    static final String KONE_SERVER = "SE";
    static final String KONE_J1 = "J1";

    static final int PORT_SERVER = 45901;
    static final int PORT_J1 = 45902;

    static final int HTTP_PORT = 8080;
    static final String API_KEY = "testkey123";

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(64));
        out.println("Generating two-machine competition (server + J1/webadmin)");
        out.println("=".repeat(64));

        LocalTime now = LocalTime.now();
        LocalTime first = now.truncatedTo(ChronoUnit.MINUTES);
        LocalTime nextHour = now.truncatedTo(ChronoUnit.HOURS).plusHours(1);
        int slots = args.length > 0
                ? Integer.parseInt(args[0])
                : (int) Duration.between(first, nextHour).toMinutes();
        if (slots < 1) slots = 1;

        // Pre-race source: every stage open (keskhyl='-', no result).
        Path src = Harness.preRaceSourceData();

        Path dirServer = Harness.setupDataDir("server", KONE_SERVER, null, src,
                new Harness.Connection(PORT_SERVER, "127.0.0.1", PORT_J1));
        Path dirJ1 = Harness.setupDataDir("J1", KONE_J1, null, src,
                new Harness.Connection(PORT_J1, "127.0.0.1", PORT_SERVER));

        Path kilpServer = dirServer.resolve("KILP.DAT");
        Path kilpJ1 = dirJ1.resolve("KILP.DAT");

        // Real (named) competitors only — skip vacant/empty placeholder records.
        List<Competitor> pool = new ArrayList<>();
        for (Competitor c : KilpReader.read(kilpServer)) {
            if (c.kilpno > 0 && c.sukunimi != null && !c.sukunimi.isBlank()) {
                pool.add(c);
            }
        }
        Collections.shuffle(pool, new Random());

        int count = Math.min(slots, pool.size());
        out.printf("%nStart window: %s → %s  (%d one-minute slots, %d competitors available)%n",
                first, first.plusMinutes(slots), slots, pool.size());
        if (count < slots) {
            out.printf("Only %d named competitors — assigning %d slots.%n", pool.size(), count);
        }

        // The demo data ships with its own start times; wipe them so the only
        // start list is the one we generate (selected starters below).
        for (Competitor c : pool) {
            Harness.setPvStartTime(kilpServer, c.recordIndex, 0, 0);
            Harness.setPvStartTime(kilpJ1, c.recordIndex, 0, 0);
        }

        out.println("\nStart list:");
        for (int i = 0; i < count; i++) {
            Competitor c = pool.get(i);
            LocalTime t = first.plusMinutes(i);
            int startMs = t.toSecondOfDay() * 1000;
            // Same assignment in both files so the two machines start in sync.
            Harness.setPvStartTime(kilpServer, c.recordIndex, 0, startMs);
            Harness.setPvStartTime(kilpJ1, c.recordIndex, 0, startMs);
            out.printf("  %s  #%-4d %s %s%n", t, c.kilpno,
                    c.etunimi == null ? "" : c.etunimi,
                    c.sukunimi == null ? "" : c.sukunimi);
        }

        printInstructions(dirServer, dirJ1, pool, count);
    }

    static void printInstructions(Path dirServer, Path dirJ1, List<Competitor> pool, int count) {
        String hkmaali = Harness.HKMAALI.toString();
        String jarHint = Harness.WEBADMIN_DIR.resolve("target").resolve("webadmin-*.jar").toString();
        int sampleKilpno = count > 0 ? pool.get(0).kilpno : 0;

        out.println("\n" + "=".repeat(64));
        out.println("Data ready:");
        out.println("  server (Kone=" + KONE_SERVER + "):  " + dirServer);
        out.println("  J1     (Kone=" + KONE_J1 + "):  " + dirJ1);
        out.printf("  UDP: server :%d  <->  J1 :%d  (127.0.0.1)%n", PORT_SERVER, PORT_J1);

        out.println("\n1) Start the C++ timing server (HkMaali) in the server dir:");
        out.println("     (cd " + dirServer + " && " + hkmaali + ")");

        out.println("\n2) Start webadmin against the J1 dir (separate terminal):");
        out.println("     java -jar " + jarHint + " \\");
        out.println("       --tulospalvelu.data-dir=" + dirJ1 + " \\");
        out.println("       --tulospalvelu.auto-start=true \\");
        out.println("       --tulospalvelu.api.key=" + API_KEY + " \\");
        out.println("       --server.port=" + HTTP_PORT);
        out.println("   (build it first: cd " + Harness.WEBADMIN_DIR + " && mvn package -DskipTests)");

        out.println("\n3) Exercise the DNS REST API (competitors addressed by kilpno):");
        out.printf("     curl -s http://localhost:%d/api/v1/ping -H 'X-API-Key: %s'%n",
                HTTP_PORT, API_KEY);
        out.printf("     curl -sX POST http://localhost:%d/api/v1/competitors/%d/dns  -H 'X-API-Key: %s'%n",
                HTTP_PORT, sampleKilpno, API_KEY);
        out.printf("     curl -sX POST http://localhost:%d/api/v1/competitors/%d/open -H 'X-API-Key: %s'%n",
                HTTP_PORT, sampleKilpno, API_KEY);
        out.println("=".repeat(64));
    }
}
