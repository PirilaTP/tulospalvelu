///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT

/**
 * End-to-end regression test for webadmin's DNS REST API against a real
 * HkMaali. One C++ HkMaali (server, Kone=SE) + webadmin (Kone=J1) connected
 * over localhost UDP, both pinned to stage 1 (PÄIVÄ=1) on all-open pre-race
 * data.
 *
 * Verifies, addressing competitors by kilpno:
 *   - GET  /api/v1/ping returns connected:true (handshake works)
 *   - missing api key -> 401, unknown kilpno -> 404
 *   - POST /dns marks the competitor and the status 'E' reaches the C++
 *     server's KILP.DAT
 *   - POST /open reverts it and '-' reaches the server's KILP.DAT
 *
 * NOTE: the C++ UDP comm currently works on Linux but not on macOS (Apple
 * Silicon/Rosetta), so run this in the Linux dev container.
 */

import fi.pirila.tulospalvelu.Competitor;
import fi.pirila.tulospalvelu.KilpReader;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.file.Path;
import java.time.Duration;
import java.util.LinkedHashMap;
import java.util.Map;

import static java.lang.System.out;

public class DnsApiE2E {

    static final int PORT_SERVER = 43951;
    static final int PORT_J1     = 43952;
    static final int HTTP_PORT   = 48096;
    static final String API_KEY  = "e2ekey";

    static final HttpClient HC = HttpClient.newHttpClient();

    static HttpResponse<String> api(String method, String path, boolean withKey) throws Exception {
        HttpRequest.Builder b = HttpRequest.newBuilder(
                        URI.create("http://localhost:" + HTTP_PORT + path))
                .timeout(Duration.ofSeconds(10));
        if (withKey) b.header("X-API-Key", API_KEY);
        b = method.equals("GET") ? b.GET() : b.POST(HttpRequest.BodyPublishers.noBody());
        return HC.send(b.build(), HttpResponse.BodyHandlers.ofString());
    }

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(64));
        out.println("TEST: DNS REST API end-to-end (webadmin + real HkMaali)");
        out.println("=".repeat(64));

        if (!java.nio.file.Files.exists(Harness.HKMAALI)) {
            out.println("FAIL: HkMaali not found — build it: cd TPsource/V52 && make");
            System.exit(1);
        }

        Path src = Harness.preRaceSourceData();
        Path dirServer = Harness.setupDataDir("jb_dnsapi_server", "SE", 1, src,
                new Harness.Connection(PORT_SERVER, "127.0.0.1", PORT_J1));
        Path dirJ1 = Harness.setupDataDir("jb_dnsapi_J1", "J1", 1, src,
                new Harness.Connection(PORT_J1, "127.0.0.1", PORT_SERVER));
        Path kSrv = dirServer.resolve("KILP.DAT");
        Path kJ1 = dirJ1.resolve("KILP.DAT");

        // Pick the first real competitor; pre-race data has it open.
        int kilpno = -1, rec = -1;
        for (Competitor c : KilpReader.read(kJ1)) {
            if (c.kilpno > 0 && c.sukunimi != null && !c.sukunimi.isBlank()) {
                kilpno = c.kilpno;
                rec = c.recordIndex;
                break;
            }
        }
        out.println("Test competitor: kilpno=" + kilpno + " (record " + rec + ")");

        Map<String, Boolean> results = new LinkedHashMap<>();
        try (Harness.HkMaali se = new Harness.HkMaali(dirServer);
             Harness.Webadmin wb = new Harness.Webadmin(dirJ1, HTTP_PORT, API_KEY)) {

            out.println("\n[1] Start HkMaali (server) and navigate to main menu...");
            se.start();
            se.acceptAndWait();
            out.println("    Starting webadmin...");
            wb.start();
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);

            out.println("\n[2] Auth + lookup guards");
            results.put("no_key_401", api("GET", "/api/v1/ping", false).statusCode() == 401);
            results.put("unknown_404",
                    api("POST", "/api/v1/competitors/99999/dns", true).statusCode() == 404);

            out.println("[3] Handshake: GET /ping");
            HttpResponse<String> ping = api("GET", "/api/v1/ping", true);
            out.println("    " + ping.body());
            results.put("ping_connected",
                    ping.statusCode() == 200 && ping.body().contains("\"connected\":true"));

            out.println("[4] POST /dns -> mark ei lähtenyt");
            HttpResponse<String> dns = api("POST", "/api/v1/competitors/" + kilpno + "/dns", true);
            out.println("    " + dns.statusCode() + " " + dns.body());
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            char afterDns = Harness.readPvStatus(kSrv, rec, 0).keskhyl();
            out.println("    server KILP.DAT keskhyl = '" + afterDns + "' (expect E)");
            results.put("dns_response", dns.statusCode() == 200 && dns.body().contains("\"status\":\"E\""));
            results.put("dns_propagated", afterDns == 'E');

            out.println("[5] POST /open -> revert to avoin");
            HttpResponse<String> open = api("POST", "/api/v1/competitors/" + kilpno + "/open", true);
            out.println("    " + open.statusCode() + " " + open.body());
            Harness.sleep(Harness.SYNC_WAIT_SEC * 1000L);
            char afterOpen = Harness.readPvStatus(kSrv, rec, 0).keskhyl();
            out.println("    server KILP.DAT keskhyl = '" + afterOpen + "' (expect - or 0)");
            results.put("open_response", open.statusCode() == 200 && open.body().contains("\"status\":\"-\""));
            results.put("open_propagated", afterOpen == '-' || afterOpen == 0);
        }

        boolean allPass = results.values().stream().allMatch(Boolean::booleanValue);
        out.println("\n" + "=".repeat(64));
        out.println("RESULTS:");
        for (Map.Entry<String, Boolean> e : results.entrySet()) {
            out.printf("  %-18s %s%n", e.getKey(), Harness.tag(e.getValue()));
        }
        out.println("\n" + (allPass ? "PASS ✓ (DNS API round-trip via real HkMaali)"
                : "FAIL — see /tmp/webadmin-" + HTTP_PORT + ".log"));
        if (allPass) {
            Harness.deleteRecursive(dirServer);
            Harness.deleteRecursive(dirJ1);
        }
        System.exit(allPass ? 0 : 1);
    }
}
