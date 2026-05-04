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
import java.util.concurrent.ScheduledFuture;
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
    private final boolean passive;
    private int localPort;
    private volatile InetSocketAddress learnedPeer;

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

    /**
     * Heartbeat interval in ms. C++ peers send NAK every nakviive (default 1000ms)
     * as a keep-alive; without it, their yhtfl[] counter drops to 0 after ~5s and
     * the GUI shows the connection as "Ei". We mirror that pulse.
     */
    private static final long HEARTBEAT_INTERVAL_MS = 1000;

    private ScheduledFuture<?> heartbeatTask;
    private long lastSendTimeMs;

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
        this.passive = (serverHost == null || serverHost.isBlank() || "AUTO".equalsIgnoreCase(serverHost));
    }

    /** Listen-only constructor: peer address is learned from the first incoming packet. */
    public static TulospalveluConnection passive(String machineId, int nrec) {
        return new TulospalveluConnection(null, 0, machineId, nrec);
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
            if (!passive) sendAlkut(ctx);
            else log.info("Passive UDP listener active; awaiting peer ALKUT");
        }
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        this.ctx = ctx;
        resolveLocalPort(ctx);
        if (!passive) sendAlkut(ctx);
        else log.info("Passive UDP listener active; awaiting peer ALKUT");
    }

    private void resolveLocalPort(ChannelHandlerContext ctx) {
        InetSocketAddress local = (InetSocketAddress) ctx.channel().localAddress();
        if (local != null) {
            localPort = local.getPort();
        }
    }

    private InetSocketAddress remoteAddress() {
        if (passive) {
            return learnedPeer;
        }
        return new InetSocketAddress(serverHost, serverPort);
    }

    // --- Outbound ---

    public CompletableFuture<Boolean> sendKilppvt(int recordIndex, byte[] pvData,
                                                   int kilppvtpsize, int newBadge) {
        return sendKilppvtModified(recordIndex, pvData, kilppvtpsize, (Integer) newBadge, null,
                "KILPPVT badge", () -> log.info("  dk={}, newBadge={}", recordIndex, newBadge));
    }

    public CompletableFuture<Boolean> sendStatusChange(int recordIndex, byte[] pvData,
                                                        int kilppvtpsize, char newStatus) {
        return sendKilppvtModified(recordIndex, pvData, kilppvtpsize, null, newStatus,
                "KILPPVT status",
                () -> log.info("  dk={}, newStatus='{}'", recordIndex, newStatus));
    }

    private CompletableFuture<Boolean> sendKilppvtModified(int recordIndex, byte[] pvData,
            int kilppvtpsize, Integer newBadge, Character newKeskhyl,
            String label, Runnable detailLog) {
        CompletableFuture<Boolean> future = new CompletableFuture<>();

        ctx.executor().execute(() -> {
            byte[] data = buildKilppvtData(recordIndex, pvData, kilppvtpsize, newBadge, newKeskhyl);

            enqueueSend(() -> {
                pendingFuture = future;
                pendingPacketId = outPacketId;
                sendProtocolMessage(ctx, PKGCLASS_KILPPVT, data, label);
                detailLog.run();
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
            // Wrapper bytes 1-2 = peer's srvport (LE). C++ wrt_st_UDP writes its own
            // servaddr.sin_port here, and read_UDP at line 1163 stores it in cliaddr.
            // We must use the peer's srvport as our outgoing destination; the packet
            // source is its ephemeral sockcli, which only the lahetapaketti ACK-wait
            // window listens on — heartbeats sent there don't refresh yhtfl[].
            int peerSrvPort = (raw[1] & 0xFF) | ((raw[2] & 0xFF) << 8);
            if (passive && peerSrvPort > 0) {
                InetSocketAddress newPeer = new InetSocketAddress(packet.sender().getAddress(), peerSrvPort);
                if (!newPeer.equals(learnedPeer)) {
                    learnedPeer = newPeer;
                    log.info("Learned peer srvaddr from STX wrapper: {}", newPeer);
                }
            }
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
            startHeartbeat();
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

        log.info("Incoming message: pkgclass={} ({}), id={}, len={}, data={}",
                frame.pkgclass() & 0xFF, pkgclassName(frame.pkgclass()),
                frame.id() & 0xFF, frame.data().length,
                bytesToHex(frame.data()));

        if (inPacketIdInit) {
            inPacketId = (byte) (frame.id() - 1);
            inPacketIdInit = false;
        }

        // Passive mode: an incoming ALKUT is the handshake "from the other direction".
        // Mark connected and start the heartbeat. learnedPeer was already set from the
        // STX wrapper in channelRead0 (peer's srvport, not ephemeral sockcli).
        if (passive && frame.pkgclass() == PKGCLASS_ALKUT) {
            if (!connected) {
                connected = true;
                connectedLatch.countDown();
                log.info("Passive: connected to peer {} (ALKUT received)", learnedPeer);
                startHeartbeat();
            }
        }

        // Always ACK (even retransmissions). Send back to the packet's source
        // address — MA's client socket is at an ephemeral port, not serverPort,
        // and NAT state-tracking delivers our reply to the right place.
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
        lastSendTimeMs = System.currentTimeMillis();
    }

    private void sendAck(ChannelHandlerContext ctx, byte msgId, InetSocketAddress target) {
        // Server-side ACK format (peer responding to client): 00 00 + machineID + ACK(+id+~id+id)
        // Matches wrt_st_UDPsrv / read_UDPcli convention in C++ side (WinUDP.cpp:1216).
        ByteBuf buf = Unpooled.buffer(4 + 4);
        buf.writeShort(0);
        buf.writeBytes(machineId.getBytes(CharsetUtil.US_ASCII));
        buf.writeByte(ACK);
        buf.writeByte(msgId);
        buf.writeByte((byte) (255 - (msgId & 0xFF)));
        buf.writeByte(msgId);

        log.debug("TX ACK for id={} -> {}", msgId & 0xFF, target);
        ctx.writeAndFlush(new DatagramPacket(buf, target));
        lastSendTimeMs = System.currentTimeMillis();
    }

    private void sendNak(ChannelHandlerContext ctx, InetSocketAddress target) {
        ByteBuf buf = Unpooled.buffer(4 + 1);
        buf.writeShort(0);
        buf.writeBytes(machineId.getBytes(CharsetUtil.US_ASCII));
        buf.writeByte(NAK);

        log.debug("TX NAK -> {}", target);
        ctx.writeAndFlush(new DatagramPacket(buf, target));
        lastSendTimeMs = System.currentTimeMillis();
    }

    private void startHeartbeat() {
        if (heartbeatTask != null) return;
        heartbeatTask = ctx.executor().scheduleAtFixedRate(this::heartbeatTick,
                HEARTBEAT_INTERVAL_MS, HEARTBEAT_INTERVAL_MS, TimeUnit.MILLISECONDS);
    }

    private void heartbeatTick() {
        if (System.currentTimeMillis() - lastSendTimeMs < HEARTBEAT_INTERVAL_MS) return;
        if (remoteAddress() == null) return; // passive: peer not learned yet
        sendHeartbeatNak();
    }

    /**
     * Send a 1-byte NAK wrapped in the C++ peer-data wrapper (STX | port | machID | NAK).
     * read_UDP() on the peer strips the 5-byte wrapper and recognises the lone NAK
     * payload, refreshing yhtfl[] and naapuri[]. The shorter UDPsrv-style wrapper
     * used by sendNak() (for invalid-frame replies) is read by read_UDPcli only and
     * would arrive at the peer's data socket as a malformed payload — koodi=1 error.
     */
    private void sendHeartbeatNak() {
        ByteBuf buf = Unpooled.buffer(5 + 1);
        buf.writeByte(STX);
        buf.writeShortLE(localPort);
        buf.writeBytes(machineId.getBytes(CharsetUtil.US_ASCII));
        buf.writeByte(NAK);

        log.trace("TX heartbeat NAK -> {}", remoteAddress());
        ctx.writeAndFlush(new DatagramPacket(buf, remoteAddress()));
        lastSendTimeMs = System.currentTimeMillis();
    }

    private void stopHeartbeat() {
        if (heartbeatTask != null) {
            heartbeatTask.cancel(false);
            heartbeatTask = null;
        }
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        stopHeartbeat();
        super.channelInactive(ctx);
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

    private static String pkgclassName(byte pkgclass) {
        return switch (pkgclass) {
            case PKGCLASS_ALKUT -> "ALKUT";
            case PKGCLASS_KILPT -> "KILPT";
            case PKGCLASS_KILPPVT -> "KILPPVT";
            case PKGCLASS_VAIN_TULOST -> "VAIN_TULOST";
            case PKGCLASS_AIKAT -> "AIKAT";
            case PKGCLASS_EMITT -> "EMITT";
            case PKGCLASS_SEURAT -> "SEURAT";
            case PKGCLASS_FILESEND -> "FILESEND";
            case PKGCLASS_EMITVA -> "EMITVA";
            case PKGCLASS_EXTRA -> "EXTRA";
            default -> "?";
        };
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
