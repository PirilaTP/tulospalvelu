///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT
//DEPS fi.pirila:pirila-udp:1.0.0-SNAPSHOT
//DEPS io.netty:netty-all:4.2.12.Final

/**
 * Webadmin Competitor edit form: ComboBox-driven seura with lyhenne+piiri
 * auto-fill. Test plants a small seurat.csv into the data dir, drives the
 * form via Playwright to pick a known club, and asserts the synthetic peer
 * receives a KILPT whose record contains the matching seuralyh + piiri.
 *
 *   browser ─UI─▶ webadmin ─UDP/KILPT─▶ synthetic peer (captures recordData)
 */

import com.microsoft.playwright.*;
import com.microsoft.playwright.options.AriaRole;
import com.microsoft.playwright.options.LoadState;
import fi.pirila.tulospalvelu.MessageListener;
import fi.pirila.tulospalvelu.TulospalveluConnection;
import fi.pirila.tulospalvelu.TulospalveluProtocol;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioDatagramChannel;

import java.nio.charset.Charset;
import java.nio.file.*;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import static java.lang.System.out;

public class SeuraComboBoxTest {

    static final String KILPNO = "1";   // first real competitor in HkKisaWinData
    static final int    SEURA_PIIRI    = 14;
    static final String SEURA_LYHENNE  = "AOK";
    static final String SEURA_NIMI     = "Akilles OK";

    static final int PORT_PEER = 46931;
    static final int PORT_WB   = 46932;
    static final int HTTP_PORT = 48096;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(70));
        out.println("TEST: Seura ComboBox — pick from catalogue → KILPT carries lyhenne+piiri");
        out.println("=".repeat(70));

        Path src = Harness.preRaceSourceData();
        Path dirWb = Harness.setupDataDir("jb_seura_WB", "WB", null, src,
                new Harness.Connection(PORT_WB, "127.0.0.1", PORT_PEER));

        // Plant the catalogue file. Latin-1 to match real seurat.csv.
        String csv = SEURA_PIIRI + ";" + SEURA_LYHENNE + ";" + SEURA_NIMI + "\n"
                + "1;AlahKi;Alahärmän Kisa\n";
        Files.writeString(dirWb.resolve("seurat.csv"), csv,
                Charset.forName("ISO-8859-1"));

        AtomicReference<byte[]> capturedRecord = new AtomicReference<>();
        AtomicReference<Integer> capturedDk = new AtomicReference<>(-1);

        TulospalveluConnection peer = TulospalveluConnection.passive("MA", 21);
        peer.setListener(new MessageListener() {
            @Override
            public void onFullCompetitorRecord(int dk, int entno, byte[] recordData) {
                capturedDk.set(dk);
                capturedRecord.set(recordData);
                out.printf("    peer captured KILPT: dk=%d entno=%d len=%d%n",
                        dk, entno, recordData.length);
            }
        });

        EventLoopGroup elg = new NioEventLoopGroup();
        Bootstrap bs = new Bootstrap();
        bs.group(elg)
          .channel(NioDatagramChannel.class)
          .handler(new ChannelInitializer<NioDatagramChannel>() {
              @Override protected void initChannel(NioDatagramChannel ch) {
                  ch.pipeline().addLast(peer);
              }
          });
        Channel chan = bs.bind(PORT_PEER).sync().channel();
        out.printf("%n  Synthetic peer listening on UDP %d%n", PORT_PEER);

        Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT);
        boolean ok = false;
        try {
            out.println("\n[1] Starting webadmin...");
            wb.start();

            out.println("\n[2] Waiting for ALKUT handshake...");
            if (!peer.awaitConnected(15, TimeUnit.SECONDS)) {
                out.println("FAIL: handshake timed out");
                System.exit(1);
            }
            out.println("    Handshake done.");

            out.printf("%n[3] Browser → pick seura '%s' from catalogue for kilpno=%s%n",
                    SEURA_NIMI, KILPNO);
            boolean submitted = pickSeuraInForm(KILPNO, SEURA_NIMI);
            out.println("    Playwright submit: " + (submitted ? "ok" : "FAIL"));
            if (!submitted) System.exit(1);
            Thread.sleep(2500);

            byte[] rec = capturedRecord.get();
            if (rec == null) {
                out.println("FAIL: peer did not receive a KILPT");
                System.exit(1);
            }
            String seura     = readWide(rec, 180, 32);
            String seuralyh  = readWide(rec, 244, 16);
            int piiri        = readInt16LE(rec, 34);
            out.printf("    KILPT record: seura='%s' seuralyh='%s' piiri=%d%n",
                    seura, seuralyh, piiri);

            ok = SEURA_NIMI.equals(seura)
                    && SEURA_LYHENNE.equals(seuralyh)
                    && piiri == SEURA_PIIRI;
        } finally {
            try { chan.close().sync(); } catch (Exception ignored) {}
            elg.shutdownGracefully();
            wb.close();
        }

        out.println("\n" + "=".repeat(70));
        out.println("    KILPT seura/lyhenne/piiri match catalogue: " + Harness.tag(ok));
        out.println("    " + (ok ? "PASS ✓" : "FAIL — see /tmp/webadmin-" + HTTP_PORT + ".log"));
        if (ok) Harness.deleteRecursive(dirWb);
        System.exit(ok ? 0 : 1);
    }

    /**
     * Open Competitor List, find kilpno, type the seura name into the
     * combobox, pick the matching option (the catalogue label is
     * "lyhenne — nimi" so we filter and click the first remaining option),
     * and click Tallenna. Returns true on submit.
     */
    static boolean pickSeuraInForm(String kilpno, String seuraNimi) {
        try (Playwright pw = Playwright.create()) {
            try (Browser browser = pw.chromium().launch(
                    new BrowserType.LaunchOptions().setHeadless(true))) {
                Page page = browser.newPage();
                page.navigate("http://localhost:" + HTTP_PORT + "/",
                        new Page.NavigateOptions().setTimeout(15000));
                page.waitForLoadState(LoadState.NETWORKIDLE,
                        new Page.WaitForLoadStateOptions().setTimeout(10000));
                page.getByText("Competitor List").click();
                Thread.sleep(2000);
                page.waitForLoadState(LoadState.NETWORKIDLE,
                        new Page.WaitForLoadStateOptions().setTimeout(10000));

                // Find the row by kilpno and select it.
                page.locator("vaadin-text-field input").first().fill(kilpno);
                Thread.sleep(1500);
                page.locator("vaadin-grid-cell-content").getByText(kilpno).first()
                        .click(new com.microsoft.playwright.Locator.ClickOptions().setForce(true));
                Thread.sleep(1500);

                // Open the Seura combobox, filter to the target name, pick it.
                var combo = page.getByRole(AriaRole.COMBOBOX,
                        new Page.GetByRoleOptions().setName("Seura"));
                combo.click();
                Thread.sleep(300);
                combo.fill(seuraNimi);
                Thread.sleep(800);
                // The dropdown items contain the full label "lyhenne — nimi".
                // After filtering by the unique nimi only one item remains;
                // click it directly.
                page.locator("vaadin-combo-box-item").first().click(
                        new com.microsoft.playwright.Locator.ClickOptions().setTimeout(3000));
                Thread.sleep(400);

                page.getByRole(AriaRole.BUTTON,
                        new Page.GetByRoleOptions().setName("Tallenna")).click();
                Thread.sleep(2500);
                return true;
            }
        } catch (Exception e) {
            System.err.println("Playwright error: " + e);
            return false;
        }
    }

    static String readWide(byte[] rec, int off, int maxChars) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < maxChars; i++) {
            int p = off + i * 2;
            if (p + 1 >= rec.length) break;
            int ch = (rec[p] & 0xFF) | ((rec[p + 1] & 0xFF) << 8);
            if (ch == 0) break;
            sb.append((char) ch);
        }
        return sb.toString();
    }

    static int readInt16LE(byte[] rec, int off) {
        return (rec[off] & 0xFF) | ((rec[off + 1] & 0xFF) << 8);
    }
}
