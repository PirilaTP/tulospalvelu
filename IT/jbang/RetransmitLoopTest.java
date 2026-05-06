///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT
//DEPS fi.pirila:pirila-udp:1.0.0-SNAPSHOT
//DEPS io.netty:netty-all:4.2.12.Final

/**
 * Reproduces the "C++ server stuck in retransmit loop" scenario reported in
 * the Windows test: HkMaali keeps sending KILPPVT id=N every ~2s because it
 * never sees the ACK from webadmin.
 *
 *   synthetic active peer (Netty) ───KILPPVT───▶  webadmin (passive)
 *                                ◀─────ACK───────
 *
 * Topology mirrors the production bug:
 *   - webadmin in passive mode (yhteys1=UDP, no destination)
 *   - synthetic peer in ACTIVE mode (sends ALKUT, then KILPPVT)
 *
 * Phase A: send 3 successive KILPPVT updates with new IDs each — verifies
 * webadmin ACKs every message under normal flow.
 * Phase B: bypass TulospalveluConnection's outPacketId management and write
 * a KILPPVT frame with a previously-used id directly to the channel, twice.
 * Counts incoming ACKs at a sniffer handler to verify webadmin ACKs duplicates
 * (the duplicate-id path) just like fresh ones.
 */

import fi.pirila.tulospalvelu.Competitor;
import fi.pirila.tulospalvelu.KilpReader;
import fi.pirila.tulospalvelu.TulospalveluConnection;
import fi.pirila.tulospalvelu.TulospalveluProtocol;
import io.netty.bootstrap.Bootstrap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.Channel;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.DatagramPacket;
import io.netty.channel.socket.nio.NioDatagramChannel;
import io.netty.util.CharsetUtil;

import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static java.lang.System.out;

public class RetransmitLoopTest {

    static final int KILPNO    = 88;
    static final int PORT_PEER = 47001;     // synthetic peer's bind port
    static final int PORT_WB   = 47002;     // webadmin's UDP listen port
    static final int HTTP_PORT = 48097;

    /** Bumped by the sniffer handler whenever we see an ACK on the wire. */
    static final AtomicInteger ackCount = new AtomicInteger();
    static final AtomicReference<byte[]> lastAckBytes = new AtomicReference<>();

    public static void main(String[] args) throws Exception {
        out.println("=".repeat(70));
        out.println("TEST: KILPPVT retransmit-loop (duplicate id ACK)");
        out.println("=".repeat(70));

        Path src = Harness.preRaceSourceData();

        // Webadmin in PASSIVE mode (custom srvPort, AUTO destAddr → passive).
        Path dirWb = Harness.setupDataDirRaw("jb_retx_WB",
                "Kone=WB\nEmit\nyhteys1=udp:" + PORT_WB + "/AUTO:0\nlähemit1\n", src);

        int recordIdx = Harness.findRecordByKilpno(dirWb.resolve("KILP.DAT"), KILPNO);
        if (recordIdx < 0) { out.println("FAIL: kilpno=" + KILPNO + " not found"); System.exit(1); }
        out.printf("%n  kilpno=%d at record_index=%d%n", KILPNO, recordIdx);

        Harness.Webadmin wb = new Harness.Webadmin(dirWb, HTTP_PORT);
        EventLoopGroup elg = new NioEventLoopGroup();
        Channel chan = null;
        boolean okFresh = false, okDuplicate = false;
        try {
            // Start webadmin FIRST so it's listening before peer sends ALKUT
            // (TulospalveluConnection sends ALKUT once on channelActive; no retry).
            out.println("\n[1] Starting webadmin (passive)...");
            wb.start();
            // UDP server takes a moment to bind after HTTP is up.
            Thread.sleep(2000);

            // Synthetic ACTIVE peer (sends ALKUT, then KILPPVT).
            TulospalveluConnection peer = new TulospalveluConnection(
                    "127.0.0.1", PORT_WB, "MA", 21);
            Bootstrap bs = new Bootstrap();
            bs.group(elg)
              .channel(NioDatagramChannel.class)
              .handler(new ChannelInitializer<NioDatagramChannel>() {
                  @Override protected void initChannel(NioDatagramChannel ch) {
                      ch.pipeline().addLast(new AckSniffer());
                      ch.pipeline().addLast(peer);
                  }
              });
            chan = bs.bind(PORT_PEER).sync().channel();
            out.printf("%n  Synthetic active peer on UDP %d -> webadmin %d%n",
                    PORT_PEER, PORT_WB);

            out.println("\n[2] Waiting for ALKUT handshake...");
            if (!peer.awaitConnected(15, TimeUnit.SECONDS)) {
                out.println("FAIL: handshake timed out");
                System.exit(1);
            }
            out.println("    Handshake done.");

            // --- Phase A: three rapid KILPPVT, each with a new id ---
            out.println("\n[3] Phase A: 3 sequential KILPPVT (status='-' = no-op tag) ");
            byte[] pvData = KilpReader.readPvData(dirWb.resolve("KILP.DAT"), recordIdx);
            int kpvSize = KilpReader.getKilppvtpsize();
            for (int i = 0; i < 3; i++) {
                Boolean ack = peer.sendKilppvt(recordIdx, 0, pvData, kpvSize, 100000 + i)
                        .get(10, TimeUnit.SECONDS);
                out.printf("    KILPPVT #%d ACK: %s%n", i + 1, ack);
                if (ack == null || !ack) { out.println("FAIL: ACK missing"); System.exit(1); }
            }
            okFresh = true;

            // --- Phase B: raw duplicate id frame ---
            out.println("\n[4] Phase B: raw KILPPVT with reused id (simulates retransmit)");
            int ackBeforeDup = ackCount.get();
            byte[] frame = buildKilppvtFrame((byte) 0x77,         // arbitrary id
                    recordIdx, 0, pvData, kpvSize, 200001, PORT_PEER, "MA");

            // Send the same exact wire bytes twice, ~200 ms apart, like C++ retry.
            chan.writeAndFlush(new DatagramPacket(
                    Unpooled.wrappedBuffer(frame),
                    new InetSocketAddress("127.0.0.1", PORT_WB))).sync();
            Thread.sleep(200);
            chan.writeAndFlush(new DatagramPacket(
                    Unpooled.wrappedBuffer(frame),
                    new InetSocketAddress("127.0.0.1", PORT_WB))).sync();
            Thread.sleep(800);
            int ackAfterDup = ackCount.get();
            int dupAcks = ackAfterDup - ackBeforeDup;
            out.printf("    ACKs received for duplicate frame: %d (expect ≥ 2)%n", dupAcks);
            if (lastAckBytes.get() != null) {
                out.println("    last ACK wire bytes: " + hex(lastAckBytes.get()));
            }
            okDuplicate = dupAcks >= 2;
        } finally {
            if (chan != null) try { chan.close().sync(); } catch (Exception ignored) {}
            elg.shutdownGracefully();
            wb.close();
        }

        boolean success = okFresh && okDuplicate;
        out.println("\n" + "=".repeat(70));
        out.println("    Phase A (3 sequential):     " + Harness.tag(okFresh));
        out.println("    Phase B (duplicate id ACK): " + Harness.tag(okDuplicate));
        out.println("    " + (success ? "PASS ✓" : "FAIL — see /tmp/webadmin-" + HTTP_PORT + ".log"));
        if (success) Harness.deleteRecursive(dirWb);
        System.exit(success ? 0 : 1);
    }

    /** Assemble the same wire bytes TulospalveluConnection.sendProtocolMessage produces. */
    static byte[] buildKilppvtFrame(byte id, int recordIdx, int pvIndex,
                                    byte[] pvData, int kpvSize, int newBadge,
                                    int srcPort, String machineId) {
        byte[] payload = TulospalveluProtocol.buildKilppvtData(
                recordIdx, pvIndex, pvData, kpvSize, (Integer) newBadge, null);
        int chk = TulospalveluProtocol.checksum(payload);
        ByteBuf buf = Unpooled.buffer(5 + 8 + payload.length);
        buf.writeByte(TulospalveluProtocol.STX);
        buf.writeShortLE(srcPort);
        buf.writeBytes(machineId.getBytes(CharsetUtil.US_ASCII));
        buf.writeByte(TulospalveluProtocol.SOH);
        buf.writeByte(id);
        buf.writeByte((byte) (255 - (id & 0xFF)));
        buf.writeByte(TulospalveluProtocol.PKGCLASS_KILPPVT);
        buf.writeShortLE(payload.length);
        buf.writeShortLE(chk);
        buf.writeBytes(payload);
        byte[] out = new byte[buf.readableBytes()];
        buf.readBytes(out);
        return out;
    }

    static String hex(byte[] b) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < Math.min(b.length, 24); i++) sb.append(String.format("%02X ", b[i]));
        return sb.toString().trim();
    }

    /**
     * Sniffer placed BEFORE TulospalveluConnection in the pipeline. It counts
     * ACK packets (4-byte UDPsrv wrapper + ACK + id + ~id + id) and re-fires
     * the message so the real handler still sees it.
     */
    static class AckSniffer extends ChannelInboundHandlerAdapter {
        @Override
        public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
            if (msg instanceof DatagramPacket dp) {
                ByteBuf b = dp.content();
                int n = b.readableBytes();
                if (n >= 5) {
                    byte[] raw = new byte[n];
                    b.getBytes(b.readerIndex(), raw);
                    // ACK wrapper: 00 00 + machID(2) + ACK + id + ~id + id  → 8 bytes
                    if (n >= 8 && raw[0] == 0 && raw[1] == 0
                            && raw[4] == TulospalveluProtocol.ACK) {
                        ackCount.incrementAndGet();
                        lastAckBytes.set(raw);
                    }
                }
            }
            super.channelRead(ctx, msg);
        }
    }
}
