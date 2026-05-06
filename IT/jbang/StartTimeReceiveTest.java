///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT
//DEPS fi.pirila:pirila-udp:1.0.0-SNAPSHOT
//DEPS io.netty:netty-all:4.2.12.Final

/**
 * Server → webadmin start-time roundtrip via KILPPVT.
 *
 * Why KILPPVT instead of VAIN_TULOST splitIndex=-1: on the C++ side,
 * tall_tulos(-1, tls) writes to pv[k_pv].va[0].vatulos but leaves the
 * primary tlahto field at offset 124 untouched (HkTls.cpp:850). C++
 * displays from tlahto, so VAIN_TULOST changes don't show. Whole-pv
 * KILPPVT is what tark_kilp(cn, 2) actually unpacks.
 *
 *   synthetic peer (Netty + TulospalveluConnection.passive)  ───KILPPVT───▶  webadmin
 */

import fi.pirila.tulospalvelu.KilpReader;
import fi.pirila.tulospalvelu.TulospalveluConnection;
import fi.pirila.tulospalvelu.TulospalveluProtocol;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioDatagramChannel;

import java.nio.file.*;
import java.util.concurrent.TimeUnit;

import static java.lang.System.out;

public class StartTimeReceiveTest {

    static final int KILPNO = 88;
    /** tlahto wire format = ms from noon. 2_700_000 = 12:45:00 (noon + 45 min). */
    static final int START_TIME_MS = 2_700_000;
    /** TMAALI0 from C++ TpDef.h — Pirilä's "not set" sentinel. */
    static final int CLEARED = -24 * 3_600_000;

    static final int PORT_PEER = 46911;
    static final int PORT_WB   = 46912;
    static final int HTTP_PORT = 48098;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(70));
        out.println("TEST: KILPPVT (tlahto via cpv[124]) — set and clear");
        out.println("=".repeat(70));

        Path src = Harness.preRaceSourceData();
        Path dirWb = Harness.setupDataDir("jb_stz_WB", "WB", null, src,
                new Harness.Connection(PORT_WB, "127.0.0.1", PORT_PEER));
        Path kilpWb = dirWb.resolve("KILP.DAT");

        int recordIdx = Harness.findRecordByKilpno(kilpWb, KILPNO);
        if (recordIdx < 0) { out.println("FAIL: kilpno=" + KILPNO + " not found"); System.exit(1); }
        out.printf("%n  kilpno=%d at record_index=%d%n", KILPNO, recordIdx);

        int initialTlahto = Harness.readPvStartTime(kilpWb, recordIdx, 0);
        out.printf("  initial pv[0].tlahto: %d%n", initialTlahto);

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

        // Pre-load: read the peer's pvData (same KILP.DAT) so we can mutate
        // tlahto and resend with the existing badge preserved.
        byte[] pvData = KilpReader.readPvData(kilpWb, recordIdx, 0);
        int kpvSize = KilpReader.getKilppvtpsize();
        int existingBadge = TulospalveluProtocol.readInt32LE(pvData,
                TulospalveluProtocol.PV_OFF_BADGE);

        Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT);
        boolean okSet = false, okCleared = false;
        try {
            out.println("\n[1] Starting webadmin...");
            wb.start();

            out.println("\n[2] Waiting for ALKUT handshake...");
            if (!peer.awaitConnected(15, TimeUnit.SECONDS)) {
                out.println("FAIL: handshake timed out");
                System.exit(1);
            }
            out.println("    Handshake done.");

            // --- Phase A: set tlahto via KILPPVT ---
            out.printf("%n[3] Peer → KILPPVT(dk=%d, pv=0, tlahto=%d)%n", recordIdx, START_TIME_MS);
            TulospalveluProtocol.writeInt32LE(pvData, 124, START_TIME_MS);
            Boolean ackA = peer.sendKilppvt(recordIdx, 0, pvData, kpvSize, existingBadge)
                    .get(10, TimeUnit.SECONDS);
            out.println("    ACK: " + ackA);
            Thread.sleep(1000);
            int afterSet = Harness.readPvStartTime(kilpWb, recordIdx, 0);
            out.printf("    WB KILP.DAT pv[0].tlahto: %d (expect %d)%n",
                    afterSet, START_TIME_MS);
            okSet = afterSet == START_TIME_MS;

            // --- Phase B: clear tlahto (TMAALI0) ---
            out.printf("%n[4] Peer → KILPPVT(dk=%d, pv=0, tlahto=%d) (TMAALI0 clear)%n",
                    recordIdx, CLEARED);
            TulospalveluProtocol.writeInt32LE(pvData, 124, CLEARED);
            Boolean ackB = peer.sendKilppvt(recordIdx, 0, pvData, kpvSize, existingBadge)
                    .get(10, TimeUnit.SECONDS);
            out.println("    ACK: " + ackB);
            Thread.sleep(1000);
            int afterClear = Harness.readPvStartTime(kilpWb, recordIdx, 0);
            out.printf("    WB KILP.DAT pv[0].tlahto: %d (expect %d = TMAALI0)%n",
                    afterClear, CLEARED);
            okCleared = afterClear == CLEARED;
        } finally {
            try { chan.close().sync(); } catch (Exception ignored) {}
            elg.shutdownGracefully();
            wb.close();
        }

        boolean success = okSet && okCleared;
        out.println("\n" + "=".repeat(70));
        out.println("    set tlahto:      " + Harness.tag(okSet));
        out.println("    cleared:         " + Harness.tag(okCleared));
        out.println("    " + (success ? "PASS ✓" : "FAIL — see /tmp/webadmin-" + HTTP_PORT + ".log"));
        if (success) Harness.deleteRecursive(dirWb);
        System.exit(success ? 0 : 1);
    }
}
