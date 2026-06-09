///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * Full-stack E2E: the DNS start-line app (https://github.com/mstahv/dns,
 * branch pirila-sync) marks non-starters in a real Pirilä network through
 * webadmin's REST API.
 *
 * Topology, all on localhost:
 *   HkMaali (C++ server, Kone=SE) <-UDP-> webadmin (Kone=J1, REST API on HTTP)
 *   DNS app (Spring Boot + Vaadin, Testcontainers Postgres via Docker)
 *       -- reads an IOF XML start list it serves itself as a static file
 *       -- POSTs /api/v1/competitors/{bib}/dns to webadmin
 *
 * The start list is built so ~half the field's start time is already in the
 * past; DNS's once-a-minute reconcile then marks exactly those as "ei lähtenyt"
 * in webadmin, which propagates to the C++ server's KILP.DAT. Playwright drives
 * the DNS UI to create the competition and configure the Pirilä connection.
 *
 * Requires: Docker (DinD) for DNS's Postgres, HkMaali built (TPsource/V52),
 * webadmin jar packaged, pirila-comm installed. Run in the Linux container —
 * the C++ UDP comm does not work on macOS.
 */

import com.microsoft.playwright.*;
import com.microsoft.playwright.options.AriaRole;
import com.microsoft.playwright.options.LoadState;
import fi.pirila.tulospalvelu.Competitor;
import fi.pirila.tulospalvelu.KilpReader;

import java.io.File;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.time.LocalTime;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.regex.Pattern;

import static java.lang.System.out;

public class DnsPirilaE2E {

    static final int PORT_SERVER = 43961;
    static final int PORT_J1     = 43962;
    static final int WEBADMIN_HTTP = 48097;
    static final int DNS_PORT     = 48098;
    static final String API_KEY = "dns-e2e-key";

    // Unique per run so DNS's on-disk start-list cache (onlinecache/) from a
    // previous run can never be reused for this competition.
    static final String STAMP = Long.toString(System.currentTimeMillis() % 1_000_000);
    static final String COMP_ID = "e2e_" + STAMP;
    static final String COMP_PW = "pw_" + STAMP;
    static final String DNS_REPO = "https://github.com/mstahv/dns.git";
    static final String DNS_BRANCH = "pirila-sync";
    static final Path DNS_CHECKOUT = Path.of("/tmp/dns-e2e-checkout");

    // DNS forces this zone (Application.main) and compares wall-clock LocalTime,
    // so our start times must be computed in the same zone, not the container's UTC.
    static final ZoneId DNS_ZONE = ZoneId.of("Europe/Helsinki");

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(70));
        out.println("TEST: DNS app -> webadmin REST API -> HkMaali (full Pirilä E2E)");
        out.println("=".repeat(70));

        if (!Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not built (cd TPsource/V52 && make)");
            System.exit(1);
        }

        // DNS compares only the time-of-day; a now±offset window that crosses
        // midnight would wrap and misclassify runners. Skip in that narrow window.
        LocalTime nowZone = LocalTime.now(DNS_ZONE);
        if (nowZone.isAfter(LocalTime.of(23, 20)) || nowZone.isBefore(LocalTime.of(0, 20))) {
            out.println("SKIP: " + nowZone + " " + DNS_ZONE + " is too close to midnight "
                    + "(start-time window would wrap). Re-run later.");
            System.exit(0);
        }

        // 1) Two-machine Pirilä net, both pinned to stage 1, all competitors open.
        Path src = Harness.preRaceSourceData();
        Path dirServer = Harness.setupDataDir("jb_dnspirila_server", "SE", 1, src,
                new Harness.Connection(PORT_SERVER, "127.0.0.1", PORT_J1));
        Path dirJ1 = Harness.setupDataDir("jb_dnspirila_J1", "J1", 1, src,
                new Harness.Connection(PORT_J1, "127.0.0.1", PORT_SERVER));
        Path kSrv = dirServer.resolve("KILP.DAT");
        Path kJ1 = dirJ1.resolve("KILP.DAT");

        // Real competitors -> split into "already started" (start time passed)
        // and "not yet" halves. bib == kilpno.
        List<Competitor> pool = new ArrayList<>();
        for (Competitor c : KilpReader.read(kJ1)) {
            if (c.kilpno > 0 && c.sukunimi != null && !c.sukunimi.isBlank()) pool.add(c);
        }
        int half = pool.size() / 2;
        List<Competitor> passed = pool.subList(0, half);          // expect -> DNS ('E')
        List<Competitor> future = pool.subList(half, pool.size()); // expect -> stay open
        Competitor reopenTarget = passed.get(0); // a late starter we register via the DNS UI
        out.printf("%nCompetitors: %d total, %d with passed start (expect DNS), %d future%n",
                pool.size(), passed.size(), future.size());

        // Start times computed once, in DNS's zone (it compares wall-clock LocalTime).
        // All "passed" share one slot, all "future" another — so the DNS view has two
        // time slots and we can navigate to the passed one to register a late starter.
        ZonedDateTime nowZdt = ZonedDateTime.now(DNS_ZONE).withSecond(0).withNano(0);
        ZonedDateTime passedStart = nowZdt.minusMinutes(15);
        ZonedDateTime futureStart = nowZdt.plusMinutes(40);
        String passedHhmm = passedStart.format(DateTimeFormatter.ofPattern("HH:mm"));

        // 2) Check out DNS (pirila-sync) and drop the IOF XML start list into its
        //    static resources so DNS serves it at /startlist.xml and reads it back.
        checkoutDns();
        Path staticDir = DNS_CHECKOUT.resolve("server/src/main/resources/static");
        Files.createDirectories(staticDir);
        Files.writeString(staticDir.resolve("startlist.xml"),
                buildIofStartList(passed, future, passedStart, futureStart), StandardCharsets.UTF_8);
        String startListUrl = "http://localhost:" + DNS_PORT + "/startlist.xml";

        Process dns = null;
        boolean pass = false;
        Map<String, Boolean> results = new LinkedHashMap<>();
        try (Harness.HkMaali se = new Harness.HkMaali(dirServer);
             Harness.Webadmin wb = new Harness.Webadmin(dirJ1, WEBADMIN_HTTP, API_KEY)) {

            out.println("\n[1] Start HkMaali (server) + webadmin (J1)...");
            se.start();
            se.acceptAndWait();
            wb.start();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);

            out.println("[2] Start DNS app via mvn spring-boot:test-run (Testcontainers Postgres)...");
            dns = startDns();
            waitForHttp("http://localhost:" + DNS_PORT + "/", 300);
            out.println("    DNS up on " + DNS_PORT);

            out.println("[3] Playwright: create competition + configure Pirilä connection...");
            drivePlaywright(startListUrl);

            out.println("[4] Wait for DNS scheduled reconcile to push DNS marks (<=150s)...");
            boolean dnsApplied = waitForStatus(kSrv, passed, 'E', 150);
            results.put("passed_marked_dns", dnsApplied);

            out.printf("[5] Register late starter bib %d via the DNS view "
                    + "(expect reopen -> '-')...%n", reopenTarget.kilpno);
            long t0 = System.currentTimeMillis();
            markStartedViaUi(COMP_PW, passedHhmm, reopenTarget.kilpno);
            boolean reopened = waitForOpen(kSrv, reopenTarget, 80);
            long secs = (System.currentTimeMillis() - t0) / 1000;
            out.printf("    reopen propagated in ~%ds (arrives via the 60s reconcile today; "
                    + "near-instant once DNS sends on the started-event — see description)%n", secs);
            results.put("late_starter_reopened", reopened);

            out.println("[6] Verify future starters were left open...");
            boolean futureOpen = true;
            for (Competitor c : future) {
                char st = Harness.readPvStatus(kSrv, c.recordIndex, 0).keskhyl();
                if (st != '-' && st != 0) { futureOpen = false; out.println("    bib " + c.kilpno + " unexpectedly '" + st + "'"); }
            }
            results.put("future_stay_open", futureOpen);

            for (Competitor c : passed) {
                char st = Harness.readPvStatus(kSrv, c.recordIndex, 0).keskhyl();
                out.printf("    bib %-4d (%s) server keskhyl='%s'%s%n", c.kilpno, c.sukunimi, st,
                        c == reopenTarget ? "  <- late starter (reopened)" : "");
            }
        } finally {
            if (dns != null) dns.destroy();
            stopDns();
        }

        pass = results.values().stream().allMatch(Boolean::booleanValue) && !results.isEmpty();
        out.println("\n" + "=".repeat(70));
        out.println("RESULTS:");
        results.forEach((k, v) -> out.printf("  %-20s %s%n", k, Harness.tag(v)));
        out.println("\n" + (pass ? "PASS ✓ (DNS -> webadmin -> HkMaali)"
                : "FAIL — DNS log: /tmp/dns-e2e.log, webadmin: /tmp/webadmin-" + WEBADMIN_HTTP + ".log"));
        if (pass) {
            Harness.deleteRecursive(dirServer);
            Harness.deleteRecursive(dirJ1);
        }
        System.exit(pass ? 0 : 1);
    }

    // --- DNS app lifecycle ---------------------------------------------------

    static void checkoutDns() throws Exception {
        if (Files.isDirectory(DNS_CHECKOUT.resolve(".git"))) {
            out.println("[0] DNS checkout exists, reusing " + DNS_CHECKOUT);
            return;
        }
        out.println("[0] Cloning DNS (" + DNS_BRANCH + ")...");
        run(Path.of("/tmp"), "git", "clone", "--depth", "1", "-b", DNS_BRANCH, DNS_REPO,
                DNS_CHECKOUT.getFileName().toString());
    }

    static Process startDns() throws Exception {
        Path server = DNS_CHECKOUT.resolve("server");
        // Wipe DNS's on-disk start-list cache so it always re-fetches our URL.
        Harness.deleteRecursive(server.resolve("onlinecache"));
        ProcessBuilder pb = new ProcessBuilder(
                "mvn", "-q", "spring-boot:test-run",
                "-Dspring-boot.run.arguments=--server.port=" + DNS_PORT);
        pb.directory(server.toFile());
        pb.redirectErrorStream(true);
        pb.redirectOutput(new File("/tmp/dns-e2e.log"));
        return pb.start();
    }

    static void stopDns() {
        // test-run forks a JVM; kill any leftover DNS process on our port.
        try {
            new ProcessBuilder("bash", "-c",
                    "pkill -f 'spring-boot:test-run' 2>/dev/null; "
                    + "pkill -f 'server.port=" + DNS_PORT + "' 2>/dev/null; true")
                    .inheritIO().start().waitFor();
        } catch (Exception ignored) {}
    }

    // --- Playwright ----------------------------------------------------------

    static void drivePlaywright(String startListUrl) {
        try (Playwright pw = Playwright.create();
             Browser browser = pw.chromium().launch(
                     new BrowserType.LaunchOptions().setHeadless(true))) {
            Page page = browser.newPage();

            // Create competition from the IOF XML URL (MainView, CreateFromUrlPanel)
            page.navigate("http://localhost:" + DNS_PORT + "/",
                    new Page.NavigateOptions().setTimeout(30000));
            page.waitForLoadState(LoadState.NETWORKIDLE,
                    new Page.WaitForLoadStateOptions().setTimeout(30000));

            page.getByLabel("Nimesi").fill("E2E Tester");
            page.getByLabel("IOF XML -tiedoston URL").fill(startListUrl);
            page.getByLabel("Kisan tunniste").fill(COMP_ID);
            // Two "Keksi kisasalasana" fields; the URL panel's is the last one.
            page.getByLabel("Keksi kisasalasana").last().fill(COMP_PW);
            page.getByRole(AriaRole.BUTTON,
                    new Page.GetByRoleOptions().setName("Luo kilpailu linkistä")).click();
            // Wait until the create actually navigated to DnsView — only then is the
            // competition password in the server session, which /pirila requires to
            // render its form (PirilaConnectionView only adds it when password != null).
            page.waitForURL("**/dns", new Page.WaitForURLOptions().setTimeout(30000));
            page.waitForLoadState(LoadState.NETWORKIDLE,
                    new Page.WaitForLoadStateOptions().setTimeout(20000));

            // Configure the Pirilä connection (/pirila)
            page.navigate("http://localhost:" + DNS_PORT + "/pirila",
                    new Page.NavigateOptions().setTimeout(20000));
            page.waitForLoadState(LoadState.NETWORKIDLE,
                    new Page.WaitForLoadStateOptions().setTimeout(20000));
            page.getByLabel("Palvelimen osoite").fill("http://localhost:" + WEBADMIN_HTTP);
            page.getByLabel("API-avain").fill(API_KEY);
            page.getByLabel("Synkronointi käytössä").check();
            // No "Testaa yhteys" click — its notification can overlay and swallow the
            // Tallenna click; the reconcile exercises connectivity for real anyway.
            page.getByRole(AriaRole.BUTTON,
                    new Page.GetByRoleOptions().setName("Tallenna")).click();
            page.waitForTimeout(3000);
            out.println("    Pirilä connection saved (server=http://localhost:" + WEBADMIN_HTTP + ")");
        }
    }

    /**
     * Register a late starter through the DNS view: log in to the competition,
     * navigate to the (past) start-time slot, and click the runner's card to mark
     * them started. That makes DNS send /open for them to webadmin.
     */
    static void markStartedViaUi(String compPw, String passedHhmm, int bib) {
        try (Playwright pw = Playwright.create();
             Browser browser = pw.chromium().launch(
                     new BrowserType.LaunchOptions().setHeadless(true))) {
            Page page = browser.newPage();

            page.navigate("http://localhost:" + DNS_PORT + "/",
                    new Page.NavigateOptions().setTimeout(30000));
            page.waitForLoadState(LoadState.NETWORKIDLE,
                    new Page.WaitForLoadStateOptions().setTimeout(30000));
            page.getByLabel("Nimesi").fill("E2E Tester");
            // Exact: "Kisasalasana" otherwise also matches the two "Keksi kisasalasana" fields.
            page.getByLabel("Kisasalasana", new Page.GetByLabelOptions().setExact(true)).fill(compPw);
            page.getByRole(AriaRole.BUTTON,
                    new Page.GetByRoleOptions().setName("Kirjaudu kisaan")).click();
            page.waitForURL("**/dns", new Page.WaitForURLOptions().setTimeout(30000));
            page.waitForLoadState(LoadState.NETWORKIDLE,
                    new Page.WaitForLoadStateOptions().setTimeout(20000));

            // The view opens on the upcoming slot; jump to the passed slot via the
            // time-navigation popover (the bold "HH:mm (i/n)" toolbar button).
            page.locator("vaadin-button")
                    .filter(new Locator.FilterOptions().setHasText(Pattern.compile("\\d{1,2}:\\d{2}\\s*\\(")))
                    .first().click();
            page.getByLabel("Siirry aikaan").fill(passedHhmm);
            page.getByRole(AriaRole.BUTTON, new Page.GetByRoleOptions().setName("Siirry")).click();
            page.waitForTimeout(2000);

            // Click the late starter's card (matched by its "nro: <bib>" badge) -> marks started.
            page.locator("vaadin-card")
                    .filter(new Locator.FilterOptions().setHasText("nro: " + bib))
                    .first().click();
            page.waitForTimeout(2000);
            out.println("    clicked card for bib " + bib + " at slot " + passedHhmm);
        }
    }

    // --- IOF XML start list --------------------------------------------------

    private static final DateTimeFormatter IOF_DT =
            DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ssxxx");

    /** Minimal IOF v3 StartList; passed[] get a past start time, future[] a future one. */
    static String buildIofStartList(List<Competitor> passed, List<Competitor> future,
                                    ZonedDateTime passedStart, ZonedDateTime futureStart) {
        StringBuilder sb = new StringBuilder();
        sb.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        // createTime is required: DNS's TulospalveluService caches it and NPEs on null.
        sb.append("<StartList xmlns=\"http://www.orienteering.org/datastandard/3.0\" "
                + "iofVersion=\"3.0\" createTime=\"").append(IOF_DT.format(passedStart.minusHours(2)))
                .append("\">\n");
        sb.append("  <ClassStart>\n    <Class><Name>H21</Name></Class>\n");
        appendStarts(sb, passed, passedStart);
        appendStarts(sb, future, futureStart);
        sb.append("  </ClassStart>\n</StartList>\n");
        return sb.toString();
    }

    static void appendStarts(StringBuilder sb, List<Competitor> cs, ZonedDateTime when) {
        String start = IOF_DT.format(when);
        for (Competitor c : cs) {
            sb.append("    <PersonStart>\n");
            sb.append("      <Person><Name><Family>").append(xml(c.sukunimi))
              .append("</Family><Given>").append(xml(c.etunimi == null ? "" : c.etunimi))
              .append("</Given></Name></Person>\n");
            sb.append("      <Start><BibNumber>").append(c.kilpno)
              .append("</BibNumber><StartTime>").append(start).append("</StartTime></Start>\n");
            sb.append("    </PersonStart>\n");
        }
    }

    static String xml(String s) {
        return s == null ? "" : s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
    }

    // --- helpers -------------------------------------------------------------

    static boolean waitForStatus(Path kilp, List<Competitor> cs, char expect, int timeoutSec)
            throws Exception {
        long deadline = System.currentTimeMillis() + timeoutSec * 1000L;
        while (System.currentTimeMillis() < deadline) {
            boolean all = true;
            for (Competitor c : cs) {
                if (Harness.readPvStatus(kilp, c.recordIndex, 0).keskhyl() != expect) { all = false; break; }
            }
            if (all) return true;
            Harness.sleep(3000);
        }
        return false;
    }

    /** Wait until the competitor's stage is open again ('-' or unset) on the server. */
    static boolean waitForOpen(Path kilp, Competitor c, int timeoutSec) throws Exception {
        long deadline = System.currentTimeMillis() + timeoutSec * 1000L;
        while (System.currentTimeMillis() < deadline) {
            char st = Harness.readPvStatus(kilp, c.recordIndex, 0).keskhyl();
            if (st == '-' || st == 0) return true;
            Harness.sleep(3000);
        }
        return false;
    }

    static void waitForHttp(String url, int timeoutSec) throws Exception {
        long deadline = System.currentTimeMillis() + timeoutSec * 1000L;
        while (System.currentTimeMillis() < deadline) {
            try {
                var c = (java.net.HttpURLConnection) java.net.URI.create(url).toURL().openConnection();
                c.setConnectTimeout(2000); c.setReadTimeout(2000);
                int code = c.getResponseCode();
                if (code > 0 && code < 500) return;
            } catch (Exception ignored) {}
            Harness.sleep(2000);
        }
        throw new RuntimeException("DNS did not come up at " + url + " within " + timeoutSec + "s");
    }

    static void run(Path dir, String... cmd) throws Exception {
        Process p = new ProcessBuilder(cmd).directory(dir.toFile()).inheritIO().start();
        if (p.waitFor() != 0) throw new RuntimeException("command failed: " + String.join(" ", cmd));
    }
}
