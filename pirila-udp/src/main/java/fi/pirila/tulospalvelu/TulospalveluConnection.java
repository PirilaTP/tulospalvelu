package fi.pirila.tulospalvelu;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import io.netty.channel.socket.DatagramPacket;
import io.netty.util.CharsetUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.InetSocketAddress;
import java.time.Instant;
import java.util.ArrayDeque;
import java.util.Deque;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * Persistent connection handler for tulospalvelu UDP protocol.
 * Stays in the Netty pipeline for the application's lifetime.
 * Handles ALKUT handshake once at startup, then bidirectional KILPPVT traffic.
 */
public class TulospalveluConnection extends SimpleChannelInboundHandler<DatagramPacket> {

    private static final Logger log = LoggerFactory.getLogger(TulospalveluConnection.class);

    private static final byte SOH = 0x01;
    private static final byte STX = 0x02;
    private static final byte ACK = 0x06;
    private static final byte NAK = 0x15;

    private static final byte PKGCLASS_ALKUT = 0;
    private static final byte PKGCLASS_KILPPVT = 2;

    private static final int ALKUT_DATA_SIZE = 10;
    private static final int PV_OFF_BADGE = 68;

    private final String serverHost;
    private final int serverPort;
    private final String machineId;
    private final int nrec;
    private int localPort; // our listening port, sent in UDP wrapper so server knows where to reach us

    private final CountDownLatch connectedLatch = new CountDownLatch(1);
    private volatile boolean connected = false;
    private volatile Instant lastMessageTime;

    // Packet ID: starts at 1, wraps 255->1 (skip 0)
    private byte outPacketId = 1;
    // Expected incoming packet ID from server (for ACKing server-initiated messages)
    private byte inPacketId = 0;
    private boolean inPacketIdInit = true;

    // Pending outbound request waiting for ACK
    private CompletableFuture<Boolean> pendingFuture;
    private byte pendingPacketId;

    // Send queue with 500ms pacing (server input buffer holds 1 message)
    private final Deque<Runnable> sendQueue = new ArrayDeque<>();
    private boolean sendInFlight = false;

    private KilppvtListener listener;
    private ChannelHandlerContext ctx;

    public interface KilppvtListener {
        void onCompetitorUpdate(int dk, int pv, byte[] cpvData);
    }

    public TulospalveluConnection(String serverHost, int serverPort, String machineId, int nrec) {
        this.serverHost = serverHost;
        this.serverPort = serverPort;
        this.machineId = machineId;
        this.nrec = nrec;
    }

    public void setListener(KilppvtListener listener) {
        this.listener = listener;
    }

    public boolean awaitConnected(long timeout, TimeUnit unit) throws InterruptedException {
        return connectedLatch.await(timeout, unit);
    }

    public boolean isConnected() {
        return connected;
    }

    /**
     * Returns true if the connection is established and KILPPVT messages
     * have been exchanged (sent or received) at least once.
     */
    public boolean isActive() {
        return connected && lastMessageTime != null;
    }

    /**
     * Returns the time of the last KILPPVT message exchange (sent or received),
     * or null if no messages have been exchanged yet.
     */
    public Instant getLastMessageTime() {
        return lastMessageTime;
    }

    @Override
    public void handlerAdded(ChannelHandlerContext ctx) throws Exception {
        this.ctx = ctx;
        if (ctx.channel().isActive()) {
            resolveLocalPort(ctx);
            sendAlkut(ctx);
        }
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        this.ctx = ctx;
        resolveLocalPort(ctx);
        sendAlkut(ctx);
    }

    private void resolveLocalPort(ChannelHandlerContext ctx) {
        java.net.InetSocketAddress local = (java.net.InetSocketAddress) ctx.channel().localAddress();
        if (local != null) {
            localPort = local.getPort();
        }
    }

    private InetSocketAddress remoteAddress() {
        return new InetSocketAddress(serverHost, serverPort);
    }

    // --- Outbound: user-initiated emit change ---

    public CompletableFuture<Boolean> sendKilppvt(int recordIndex, byte[] pvData,
                                                   int kilppvtpsize, int newBadge) {
        CompletableFuture<Boolean> future = new CompletableFuture<>();

        // Marshal onto event loop
        ctx.executor().execute(() -> {
            byte[] cpv = new byte[kilppvtpsize];
            System.arraycopy(pvData, 0, cpv, 0, Math.min(pvData.length, kilppvtpsize));
            writeInt32LE(cpv, PV_OFF_BADGE, newBadge);

            int dataLen = 8 + kilppvtpsize;
            byte[] data = new byte[dataLen];
            data[0] = 0;                                   // tarf
            data[1] = 1;                                   // pakota = force
            writeInt16LE(data, 2, (short) recordIndex);    // dk
            writeInt16LE(data, 4, (short) 0);              // pv = 0
            writeInt16LE(data, 6, (short) 0);              // valuku = 0
            System.arraycopy(cpv, 0, data, 8, kilppvtpsize);

            enqueueSend(() -> {
                pendingFuture = future;
                pendingPacketId = outPacketId;
                sendProtocolMessage(ctx, PKGCLASS_KILPPVT, data, "KILPPVT");
                log.info("  dk={}, newBadge={}", recordIndex, newBadge);
                advancePacketId();
            });
        });

        return future;
    }

    // --- Send queue with 500ms pacing ---

    private void enqueueSend(Runnable sendAction) {
        sendQueue.add(sendAction);
        if (!sendInFlight) {
            drainQueue();
        }
    }

    private void drainQueue() {
        if (sendQueue.isEmpty()) {
            sendInFlight = false;
            return;
        }
        sendInFlight = true;
        Runnable next = sendQueue.poll();
        next.run();
    }

    private void onSendCompleted() {
        // Schedule next send after 500ms to let server process
        ctx.executor().schedule(this::drainQueue, 500, TimeUnit.MILLISECONDS);
    }

    // --- Inbound message handling ---

    @Override
    protected void channelRead0(ChannelHandlerContext ctx, DatagramPacket packet) throws Exception {
        ByteBuf buffer = packet.content();
        byte[] raw = new byte[buffer.readableBytes()];
        buffer.readBytes(raw);

        if (raw.length < 5) return;

        // Detect wrapper type:
        // - Server ACK/NAK (wrt_st_UDPsrv): 4-byte wrapper [0x0000 + machID] + payload
        // - Server data (wrt_st_UDP):        5-byte wrapper [STX + port + machID] + payload
        int payloadOffset;
        if (raw[0] == STX) {
            // Drop server send-retry NAKs (UDPvarmistamaton auto-ACK causes our ACK
            // to appear stale, server retries and eventually NAKs - harmless noise)
            if (raw.length <= 6 && raw[raw.length - 1] == NAK) return;
            payloadOffset = 5;
        } else {
            payloadOffset = 4;
        }

        if (raw.length <= payloadOffset) return;

        log.debug("RX {} bytes from {}: {}", raw.length, packet.sender(), bytesToHex(raw));

        byte firstByte = raw[payloadOffset];

        if (firstByte == ACK && raw.length >= payloadOffset + 4) {
            handleAck(raw, payloadOffset);
        } else if (firstByte == NAK) {
            handleNak();
        } else if (firstByte == SOH && raw.length >= payloadOffset + 8) {
            // Server data comes from sockcli (different port than socksrv)
            // ACK must go back to the sender's address, not our configured serverHost:serverPort
            handleIncomingMessage(ctx, raw, payloadOffset, packet.sender());
        }
    }

    private void handleAck(byte[] raw, int off) {
        byte ackId = raw[off + 1];
        byte ackIid = raw[off + 2];
        byte ackId2 = raw[off + 3];

        if (((ackId & 0xFF) + (ackIid & 0xFF)) != 255 || ackId != ackId2) {
            log.warn("Invalid ACK format");
            return;
        }

        log.info("ACK for id={}", ackId & 0xFF);

        if (!connected) {
            // ALKUT ACK
            connected = true;
            connectedLatch.countDown();
            log.info("Connected to server (ALKUT acknowledged)");
            onSendCompleted();
            return;
        }

        // KILPPVT ACK
        if (pendingFuture != null && ackId == pendingPacketId) {
            lastMessageTime = Instant.now();
            pendingFuture.complete(true);
            pendingFuture = null;
        }
        onSendCompleted();
    }

    private void handleNak() {
        if (!connected) {
            log.error("ALKUT rejected by server");
            connectedLatch.countDown();
            return;
        }
        if (pendingFuture != null) {
            log.warn("NAK received for pending request");
            pendingFuture.complete(false);
            pendingFuture = null;
            onSendCompleted();
        }
        // Ignore NAK when no operation pending (server retransmission noise)
    }

    private void handleIncomingMessage(ChannelHandlerContext ctx, byte[] raw, int off,
                                       InetSocketAddress sender) {
        // Parse: SOH(1) + id(1) + iid(1) + pkgclass(1) + len(2) + checksum(2) + data
        byte msgId = raw[off + 1];
        byte msgIid = raw[off + 2];
        byte pkgclass = raw[off + 3];
        int len = (raw[off + 4] & 0xFF) | ((raw[off + 5] & 0xFF) << 8);

        log.info("Incoming message: pkgclass={}, id={}, len={}", pkgclass & 0xFF, msgId & 0xFF, len);

        // Validate id+iid
        if (((msgId & 0xFF) + (msgIid & 0xFF)) != 255) {
            log.warn("Invalid message ID checksum");
            sendNak(ctx, sender);
            return;
        }

        // Verify data checksum
        int dataStart = off + 8; // SOH(1) + id(1) + iid(1) + pkgclass(1) + len(2) + chk(2)
        if (raw.length < dataStart + len) {
            log.warn("Message too short");
            sendNak(ctx, sender);
            return;
        }
        byte[] data = new byte[len];
        System.arraycopy(raw, dataStart, data, 0, len);

        int expectedChk = (raw[off + 6] & 0xFF) | ((raw[off + 7] & 0xFF) << 8);
        int actualChk = checksum(data);
        if (expectedChk != actualChk) {
            log.warn("Checksum mismatch: expected 0x{}, got 0x{}",
                    String.format("%04X", expectedChk), String.format("%04X", actualChk));
            sendNak(ctx, sender);
            return;
        }

        // Track incoming packet ID and detect retransmissions
        if (inPacketIdInit) {
            inPacketId = (byte) (msgId - 1);
            inPacketIdInit = false;
        }

        // Always ACK (even retransmissions)
        sendAck(ctx, msgId, sender);

        // Skip duplicate messages (server retransmits when our ACK doesn't arrive)
        if (msgId == inPacketId) {
            log.debug("Duplicate message id={}, ACKed but not reprocessed", msgId & 0xFF);
            return;
        }
        inPacketId = msgId;

        // Process KILPPVT
        if (pkgclass == PKGCLASS_KILPPVT) {
            lastMessageTime = Instant.now();
        }
        if (pkgclass == PKGCLASS_KILPPVT && listener != null) {
            // KILPPVT data: tarf(1) + pakota(1) + dk(2) + pv(2) + valuku(2) + cpv(...)
            if (data.length >= 8) {
                int dk = (data[2] & 0xFF) | ((data[3] & 0xFF) << 8);
                int pv = (data[4] & 0xFF) | ((data[5] & 0xFF) << 8);
                byte[] cpv = new byte[data.length - 8];
                System.arraycopy(data, 8, cpv, 0, cpv.length);
                log.info("Incoming KILPPVT: dk={}, pv={}", dk, pv);
                listener.onCompetitorUpdate(dk, pv, cpv);
            }
        }
    }

    // --- Send helpers ---

    private void sendAlkut(ChannelHandlerContext ctx) {
        byte[] data = new byte[ALKUT_DATA_SIZE];
        data[0] = 1;
        data[1] = (byte) machineId.charAt(0);
        data[2] = (byte) machineId.charAt(1);
        data[3] = 1; // vaihe
        writeInt16LE(data, 4, (short) nrec);

        enqueueSend(() -> {
            sendProtocolMessage(ctx, PKGCLASS_ALKUT, data, "ALKUT");
            advancePacketId();
        });
    }

    private void sendProtocolMessage(ChannelHandlerContext ctx, byte pkgclass, byte[] data, String label) {
        int chk = checksum(data);
        byte id = outPacketId;

        ByteBuf buf = Unpooled.buffer(5 + 8 + data.length);
        buf.writeByte(STX);
        buf.writeShortLE(localPort);  // OUR port, so server knows where to send back
        buf.writeBytes(machineId.getBytes(CharsetUtil.US_ASCII));
        buf.writeByte(SOH);
        buf.writeByte(id);
        buf.writeByte((byte) (255 - (id & 0xFF)));
        buf.writeByte(pkgclass);
        buf.writeShortLE(data.length);
        buf.writeShortLE(chk);
        buf.writeBytes(data);

        log.info("TX {} ({} bytes, id={}, len={}) -> {}", label, 5 + 8 + data.length,
                id & 0xFF, data.length, remoteAddress());

        ctx.writeAndFlush(new DatagramPacket(buf, remoteAddress()));
    }

    /**
     * Send ACK to server's sockcli.
     * Uses 4-byte wrapper format (same as wrt_st_UDPsrv): 0x0000(2) + machineID(2)
     * The server reads ACKs via read_UDPcli which strips this 4-byte wrapper.
     */
    private void sendAck(ChannelHandlerContext ctx, byte msgId, InetSocketAddress target) {
        ByteBuf buf = Unpooled.buffer(4 + 4);
        buf.writeShort(0);  // 0x0000 (2 bytes)
        buf.writeBytes(machineId.getBytes(CharsetUtil.US_ASCII));  // machineID (2 bytes)
        buf.writeByte(ACK);
        buf.writeByte(msgId);
        buf.writeByte((byte) (255 - (msgId & 0xFF)));
        buf.writeByte(msgId);

        log.debug("TX ACK for id={} -> {}", msgId & 0xFF, target);
        ctx.writeAndFlush(new DatagramPacket(buf, target));
    }

    private void sendNak(ChannelHandlerContext ctx, InetSocketAddress target) {
        ByteBuf buf = Unpooled.buffer(4 + 1);
        buf.writeShort(0);
        buf.writeBytes(machineId.getBytes(CharsetUtil.US_ASCII));
        buf.writeByte(NAK);

        log.debug("TX NAK -> {}", target);
        ctx.writeAndFlush(new DatagramPacket(buf, target));
    }

    private void advancePacketId() {
        outPacketId++;
        if (outPacketId == 0) outPacketId = 1; // skip 0, id=1 reserved for ALKUT reset
    }

    // --- Utility ---

    static int checksum(byte[] data) {
        int sum = 0;
        int i;
        for (i = 0; i + 1 < data.length; i += 2) {
            sum += (data[i] & 0xFF) | ((data[i + 1] & 0xFF) << 8);
        }
        if (i < data.length) {
            sum += data[i] & 0xFF;
        }
        return sum & 0xFFFF;
    }

    static void writeInt16LE(byte[] buf, int offset, short value) {
        buf[offset] = (byte) (value & 0xFF);
        buf[offset + 1] = (byte) ((value >> 8) & 0xFF);
    }

    static void writeInt32LE(byte[] buf, int offset, int value) {
        buf[offset] = (byte) (value & 0xFF);
        buf[offset + 1] = (byte) ((value >> 8) & 0xFF);
        buf[offset + 2] = (byte) ((value >> 16) & 0xFF);
        buf[offset + 3] = (byte) ((value >> 24) & 0xFF);
    }

    private String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) sb.append(String.format("%02X ", b));
        return sb.toString().trim();
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        log.error("Network error", cause);
        if (pendingFuture != null) {
            pendingFuture.completeExceptionally(cause);
            pendingFuture = null;
        }
    }
}
