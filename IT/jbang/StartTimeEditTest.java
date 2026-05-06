///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT
//DEPS fi.pirila:pirila-udp:1.0.0-SNAPSHOT
//DEPS io.netty:netty-all:4.2.12.Final

/**
 * Webadmin → server start-time edit. Drives the Competitor List edit form via
 * Playwright (Lähtöaika TextField), expects a KILPPVT to appear at the
 * synthetic peer with the modified tlahto field at cpv offset 124.
 *
 * (We cannot use VAIN_TULOST splitIndex=-1: C++ tark_kilp(cn,0) routes that
 * to tall_tulos which only writes va[0].vatulos and not pv.tlahto.)
 *
 *   browser ─UI─▶ webadmin ─UDP/KILPPVT(cpv[124]=tlahto)─▶ synthetic peer
 */

import fi.pirila.tulospalvelu.MessageListener;
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
import java.util.concurrent.atomic.AtomicInteger;

import static java.lang.System.out;

public class StartTimeEditTest {

    static final String KILPNO = "88";
    /** Picked deliberately past noon so the wire value is positive (= ms after noon). */
    static final String START_TIME_HMS = "13:30:00";
    /** 13:30 → 1.5h after noon → +5_400_000 ms in the noon-relative encoding. */
    static final int    START_TIME_MS  = 5_400_000;
    /** TMAALI0: C++'s "not set" sentinel returned when the form field is empty. */
    static final int    CLEARED_WIRE   = -24 * 3_600_000;

    static final int PORT_PEER = 46921;
    static final int PORT_WB   = 46922;
    static final int HTTP_PORT = 48099;

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(70));
        out.println("TEST: Webadmin form → KILPPVT (cpv[124] = tlahto)");
        out.println("=".repeat(70));

        Path src = Harness.preRaceSourceData();
        Path dirWb = Harness.setupDataDir("jb_stedit_WB", "WB", null, src,
                new Harness.Connection(PORT_WB, "127.0.0.1", PORT_PEER));

        AtomicInteger setHits = new AtomicInteger();
        AtomicInteger clearHits = new AtomicInteger();
        AtomicInteger lastTlahto = new AtomicInteger(Integer.MIN_VALUE);
        AtomicInteger lastDk = new AtomicInteger(-1);

        TulospalveluConnection peer = TulospalveluConnection.passive("MA", 21);
        peer.setListener(new MessageListener() {
            @Override
            public void onCompetitorUpdate(int dk, int pv, byte[] cpvData) {
                if (cpvData.length < 128) return;
                int tlahto = TulospalveluProtocol.readInt32LE(cpvData, 124);
                lastDk.set(dk); lastTlahto.set(tlahto);
                if (tlahto == CLEARED_WIRE) clearHits.incrementAndGet();
                else setHits.incrementAndGet();
                out.printf("    peer captured KILPPVT: dk=%d pv=%d tlahto=%d%n",
                        dk, pv, tlahto);
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

            // --- Phase A: set start time via Playwright ---
            out.printf("%n[3] Browser → set kilpno=%s lähtöaika=%s%n", KILPNO, START_TIME_HMS);
            int setBefore = setHits.get();
            boolean submitted = wb.editStartTime(KILPNO, START_TIME_HMS);
            out.println("    Playwright submit: " + (submitted ? "ok" : "FAIL"));
            if (!submitted) System.exit(1);
            Thread.sleep(2000);
            int gotTime = lastTlahto.get();
            int newSets = setHits.get() - setBefore;
            out.printf("    set hits: %d, last tlahto=%d (expect %d)%n",
                    newSets, gotTime, START_TIME_MS);
            okSet = newSets >= 1 && gotTime == START_TIME_MS;

            // --- Phase B: clear start time ---
            out.printf("%n[4] Browser → clear lähtöaika%n");
            int clearBefore = clearHits.get();
            boolean submitted2 = wb.editStartTime(KILPNO, "");
            out.println("    Playwright submit: " + (submitted2 ? "ok" : "FAIL"));
            if (!submitted2) System.exit(1);
            Thread.sleep(2000);
            int newClears = clearHits.get() - clearBefore;
            int gotTime2 = lastTlahto.get();
            out.printf("    clear hits: %d, last tlahto=%d (expect %d = TMAALI0)%n",
                    newClears, gotTime2, CLEARED_WIRE);
            okCleared = newClears >= 1 && gotTime2 == CLEARED_WIRE;
        } finally {
            try { chan.close().sync(); } catch (Exception ignored) {}
            elg.shutdownGracefully();
            wb.close();
        }

        boolean success = okSet && okCleared;
        out.println("\n" + "=".repeat(70));
        out.println("    set via form:    " + Harness.tag(okSet));
        out.println("    clear via form:  " + Harness.tag(okCleared));
        out.println("    " + (success ? "PASS ✓" : "FAIL — see /tmp/webadmin-" + HTTP_PORT + ".log"));
        if (success) Harness.deleteRecursive(dirWb);
        System.exit(success ? 0 : 1);
    }
}
