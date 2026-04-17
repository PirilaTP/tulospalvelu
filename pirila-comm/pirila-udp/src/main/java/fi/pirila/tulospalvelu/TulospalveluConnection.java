package fi.pirila.tulospalvelu;

import static fi.pirila.tulospalvelu.TulospalveluProtocol.*;

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
 * Handles ALKUT handshake once at startup, then bidirectional message traffic.
 *
 * Supports all incoming message types: KILPT, KILPPVT, VAIN_TULOST, EXTRA.
 */
public class TulospalveluConnection extends SimpleChannelInboundHandler<DatagramPacket> {

    private static final Logger log = LoggerFactory.getLogger(TulospalveluConnection.class);

    private final String serverHost;
    private final int serverPort;
    private final String machineId;
    private final int nrec;
    private int localPort;

    private final CountDownLatch connectedLatch = new CountDownLatch(1);
    private volatile boolean connected = false;
    private volatile Instant lastMessageTime;

    private byte outPacketId = 1;
    private byte inPacketId = 0;
    private boolean inPacketIdInit = true;

    private CompletableFuture<Boolean> pendingFuture;
    private byte pendingPacketId;

    private final Deque<Runnable> sendQueue = new ArrayDeque<>();
    private boolean sendInFlight = false;

    private MessageListener listener;
    private ChannelHandlerContext ctx;

    /**
     * @deprecated Use {@link MessageListener} instead.
     */
    @Deprecated
    public interface KilppvtListener {
        void onCompetitorUpdate(int dk, int pv, byte[] cpvData);
    }

    public TulospalveluConnection(String serverHost, int serverPort, String machineId, int nrec) {
        this.serverHost = serverHost;
        this.serverPort = serverPort;
        this.machineId = machineId;
        this.nrec = nrec;
    }

    public void setListener(MessageListener listener) {
        this.listener = listener;
    }

    /** @deprecated Use {@link #setListener(MessageListener)} instead. */
    @Deprecated
    public void setListener(KilppvtListener legacy) {
        this.listener = new MessageListener() {
            @Override
            public void onCompetitorUpdate(int dk, int pv, byte[] cpvData) {
                legacy.onCompetitorUpdate(dk, pv, cpvData);
            }
        };
    }

    public boolean awaitConnected(long timeout, TimeUnit unit) throws InterruptedException {
        return connectedLatch.await(timeout, unit);
    }

    public boolean isConnected() {
        return connected;
    }

    public boolean isActive() {
        return connected && lastMessageTime != null;
    }

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
        InetSocketAddress local = (InetSocketAddress) ctx.channel().localAddress();
        if (local != null) {
            localPort = local.getPort();
        }
    }

    private InetSocketAddress remoteAddress() {
        return new InetSocketAddress(serverHost, serverPort);
    }

    // --- Outbound ---

    public CompletableFuture<Boolean> sendKilppvt(int recordIndex, byte[] pvData,
                                                   int kilppvtpsize, int newBadge) {
        CompletableFuture<Boolean> future = new CompletableFuture<>();

        ctx.executor().execute(() -> {
            byte[] data = buildKilppvtData(recordIndex, pvData, kilppvtpsize, newBadge);

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
            AckResponse ack = parseAck(raw, payloadOffset);
            if (ack != null) handleAck(ack);
        } else if (firstByte == NAK) {
            handleNak();
        } else if (firstByte == SOH && raw.length >= payloadOffset + 8) {
            handleIncomingMessage(ctx, raw, payloadOffset, packet.sender());
        }
    }

    private void handleAck(AckResponse ack) {
        log.info("ACK for id={}", ack.id() & 0xFF);

        if (!connected) {
            connected = true;
            connectedLatch.countDown();
            log.info("Connected to server (ALKUT acknowledged)");
            onSendCompleted();
            return;
        }

        if (pendingFuture != null && ack.id() == pendingPacketId) {
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
    }

    private void handleIncomingMessage(ChannelHandlerContext ctx, byte[] raw, int off,
                                       InetSocketAddress sender) {
        SohFrame frame = parseSohFrame(raw, off);
        if (frame == null) {
            log.warn("Invalid SOH frame");
            sendNak(ctx, sender);
            return;
        }

        log.info("Incoming message: pkgclass={}, id={}, len={}",
                frame.pkgclass() & 0xFF, frame.id() & 0xFF, frame.data().length);

        if (inPacketIdInit) {
            inPacketId = (byte) (frame.id() - 1);
            inPacketIdInit = false;
        }

        // Always ACK (even retransmissions)
        sendAck(ctx, frame.id(), sender);

        if (frame.id() == inPacketId) {
            log.debug("Duplicate message id={}, ACKed but not reprocessed", frame.id() & 0xFF);
            return;
        }
        inPacketId = frame.id();

        dispatchMessage(frame);
    }

    private void dispatchMessage(SohFrame frame) {
        byte[] data = frame.data();

        switch (frame.pkgclass()) {
            case PKGCLASS_KILPPVT -> {
                lastMessageTime = Instant.now();
                if (listener != null) {
                    KilppvtPayload p = parseKilppvtData(data);
                    if (p != null) {
                        log.info("Incoming KILPPVT: dk={}, pv={}", p.dk(), p.pv());
                        listener.onCompetitorUpdate(p.dk(), p.pv(), p.cpvData());
                    }
                }
            }
            case PKGCLASS_KILPT -> {
                lastMessageTime = Instant.now();
                if (listener != null) {
                    KilptPayload p = parseKilptData(data);
                    if (p != null) {
                        log.info("Incoming KILPT: dk={}, entno={}", p.dk(), p.entno());
                        listener.onFullCompetitorRecord(p.dk(), p.entno(), p.recordData());
                    }
                }
            }
            case PKGCLASS_VAIN_TULOST -> {
                lastMessageTime = Instant.now();
                if (listener != null) {
                    VainTulostPayload p = parseVainTulostData(data);
                    if (p != null) {
                        log.info("Incoming VAIN_TULOST: dk={}, bib={}, split={}, time={}",
                                p.dk(), p.bib(), p.splitIndex(), p.time());
                        listener.onTimeResult(p.dk(), p.bib(), p.stage(), p.splitIndex(), p.time());
                    }
                }
            }
            case PKGCLASS_EXTRA -> {
                if (listener != null) {
                    ExtraPayload p = parseExtraData(data);
                    if (p != null) {
                        switch (p.subType()) {
                            case EXTRA_CHECKPOINT ->
                                listener.onCheckpoint(p.d3(), p.d4(), p.d2(), 0);
                            case EXTRA_SHUTDOWN -> {
                                // d2 contains 2-char machine ID as bytes
                                byte[] tn = new byte[4];
                                writeInt32LE(tn, 0, p.d2());
                                String target = new String(tn, 0, 2).trim();
                                log.warn("Shutdown command received, target: '{}'",
                                        target.isEmpty() ? "ALL" : target);
                                listener.onShutdown(target);
                            }
                            default ->
                                log.debug("EXTRA sub-type {} not handled", p.subType());
                        }
                    }
                }
            }
            case PKGCLASS_ALKUT ->
                log.debug("ALKUT from server (handshake response)");
            default ->
                log.debug("Unhandled message type: {}", frame.pkgclass() & 0xFF);
        }
    }

    // --- Send helpers ---

    private void sendAlkut(ChannelHandlerContext ctx) {
        byte[] data = buildAlkutData(machineId, nrec);
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
        buf.writeShortLE(localPort);
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

    private void sendAck(ChannelHandlerContext ctx, byte msgId, InetSocketAddress target) {
        ByteBuf buf = Unpooled.buffer(4 + 4);
        buf.writeShort(0);
        buf.writeBytes(machineId.getBytes(CharsetUtil.US_ASCII));
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
        if (outPacketId == 0) outPacketId = 1;
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
