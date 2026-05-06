///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT
//DEPS fi.pirila:pirila-udp:1.0.0-SNAPSHOT
//DEPS io.netty:netty-all:4.2.12.Final

/**
 * Result-clear regression: webadmin must reflect VAIN_TULOST messages —
 * both fresh-time (time>0) and clear (time=0) — in its local KILP.DAT and
 * in-memory model. The pre-fix bug: TulospalveluService didn't override
 * MessageListener.onTimeResult so VAIN_TULOST was logged but ignored.
 *
 * Topology:
 *    synthetic peer (Netty + TulospalveluConnection.passive)  ───UDP───▶  webadmin
 *
 * No HkMaali — driving the C++ TUI to clear a time is fragile (different
 * tab counts per dataset). Instead we ACT as the C++ peer and push raw
 * VAIN_TULOST frames; that's the same wire the real C++ side produces.
 */

import fi.pirila.tulospalvelu.Competitor;
import fi.pirila.tulospalvelu.KilpReader;
import fi.pirila.tulospalvelu.TulospalveluConnection;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioDatagramChannel;

import java.nio.file.*;
import java.util.concurrent.TimeUnit;

import static java.lang.System.out;

public class ResultClearTest {

    static final int KILPNO = 88;
    static final int INITIAL_TIME_MS = 4_200_000;     // 1:10:00 baseline
    static final int CLEARED = 0;

    static final int PORT_PEER = 46901;
    static final int PORT_WB   = 46902;
    static final int HTTP_PORT = 48099;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(70));
        out.println("TEST: VAIN_TULOST roundtrip — fresh time and clear");
        out.println("=".repeat(70));

        // Webadmin gets the pre-race-cleared dataset; we'll set a time on the
        // chosen competitor so the "clear" half of the test has something to clear.
        Path src = Harness.preRaceSourceData();
        Path dirWb = Harness.setupDataDir("jb_clr_WB", "WB", null, src,
                new Harness.Connection(PORT_WB, "127.0.0.1", PORT_PEER));
        Path kilpWb = dirWb.resolve("KILP.DAT");

        int recordIdx = Harness.findRecordByKilpno(kilpWb, KILPNO);
        if (recordIdx < 0) { out.println("FAIL: kilpno=" + KILPNO + " not found"); System.exit(1); }
        out.printf("%n  kilpno=%d at record_index=%d%n", KILPNO, recordIdx);

        // Pre-set a finish time so the "fresh" branch starts from a known state.
        // (Pre-race source has time=0; we want the first VAIN_TULOST to set it.)
        int initialBefore = Harness.readPvStatus(kilpWb, recordIdx, 0).finishTime();
        out.printf("  initial pv[0].finishTime: %d%n", initialBefore);

        // Spin up the synthetic peer (passive UDP, listens for webadmin's ALKUT).
        TulospalveluConnection peer = TulospalveluConnection.passive("MA", 21);
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
        boolean okFresh = false, okCleared = false;
        try {
            out.println("\n[1] Starting webadmin...");
            wb.start();

            out.println("\n[2] Waiting for webadmin's ALKUT handshake...");
            if (!peer.awaitConnected(15, TimeUnit.SECONDS)) {
                out.println("FAIL: handshake timed out");
                System.exit(1);
            }
            out.println("    Handshake done.");

            // --- Phase A: fresh time ---
            out.printf("%n[3] Peer → VAIN_TULOST(dk=%d, time=%d) (fresh)%n",
                    recordIdx, INITIAL_TIME_MS);
            Boolean ackA = peer.sendVainTulost(recordIdx, KILPNO, 0, 0, INITIAL_TIME_MS)
                    .get(10, TimeUnit.SECONDS);
            out.println("    ACK: " + ackA);
            Thread.sleep(1000);
            int afterFresh = Harness.readPvStatus(kilpWb, recordIdx, 0).finishTime();
            out.printf("    WB KILP.DAT pv[0].finishTime: %d (expect %d)%n",
                    afterFresh, INITIAL_TIME_MS);
            okFresh = afterFresh == INITIAL_TIME_MS;

            // --- Phase B: clear ---
            out.printf("%n[4] Peer → VAIN_TULOST(dk=%d, time=0) (clear)%n", recordIdx);
            Boolean ackB = peer.sendVainTulost(recordIdx, KILPNO, 0, 0, CLEARED)
                    .get(10, TimeUnit.SECONDS);
            out.println("    ACK: " + ackB);
            Thread.sleep(1000);
            int afterClear = Harness.readPvStatus(kilpWb, recordIdx, 0).finishTime();
            out.printf("    WB KILP.DAT pv[0].finishTime: %d (expect 0)%n", afterClear);
            okCleared = afterClear == 0;
        } finally {
            try { chan.close().sync(); } catch (Exception ignored) {}
            elg.shutdownGracefully();
            wb.close();
        }

        boolean success = okFresh && okCleared;
        out.println("\n" + "=".repeat(70));
        out.println("    fresh-time set:  " + Harness.tag(okFresh));
        out.println("    cleared to zero: " + Harness.tag(okCleared));
        out.println("    " + (success ? "PASS ✓" : "FAIL — see /tmp/webadmin-" + HTTP_PORT + ".log"));
        if (success) Harness.deleteRecursive(dirWb);
        System.exit(success ? 0 : 1);
    }
}
