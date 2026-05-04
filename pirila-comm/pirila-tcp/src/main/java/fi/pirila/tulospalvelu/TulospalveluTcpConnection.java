package fi.pirila.tulospalvelu;

import static fi.pirila.tulospalvelu.TulospalveluProtocol.*;

import io.netty.bootstrap.Bootstrap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.time.Instant;
import java.util.ArrayDeque;
import java.util.Deque;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * TCP transport for the Tulospalvelu protocol.
 *
 * Key differences from UDP ({@link fi.pirila.tulospalvelu.TulospalveluConnection}):
 * <ul>
 *   <li>No UDP wrapper (STX+port+machineId) — SOH frames sent directly on the TCP stream</li>
 *   <li>ACK/NAK sent as raw bytes on the stream (no wrapper)</li>
 *   <li>Automatic reconnection on disconnect</li>
 *   <li>Same 500ms send pacing (server's single-slot input buffer constraint)</li>
 * </ul>
 *
 * TCP is better suited for cloud/remote deployments: single outbound connection,
 * NAT-friendly, firewall-friendly. See NETWORK_COMMUNICATION.md for details.
 */
public class TulospalveluTcpConnection extends SimpleChannelInboundHandler<byte[]> {

    private static final Logger log = LoggerFactory.getLogger(TulospalveluTcpConnection.class);

    private final String serverHost;
    private final int serverPort;
    private final String machineId;
    private final int nrec;

    private final CountDownLatch connectedLatch = new CountDownLatch(1);
    private volatile boolean connected = false;
    private volatile Instant lastMessageTime;
    private volatile boolean shutdownRequested = false;

    private byte outPacketId = 1;
    private byte inPacketId = 0;
    private boolean inPacketIdInit = true;

    private CompletableFuture<Boolean> pendingFuture;
    private byte pendingPacketId;

    private final Deque<Runnable> sendQueue = new ArrayDeque<>();
    private boolean sendInFlight = false;

    private MessageListener listener;
    private ChannelHandlerContext ctx;
    private EventLoopGroup group;
    private Channel channel;

    public TulospalveluTcpConnection(String serverHost, int serverPort, String machineId, int nrec) {
        this.serverHost = serverHost;
        this.serverPort = serverPort;
        this.machineId = machineId;
        this.nrec = nrec;
    }

    public void setListener(MessageListener listener) {
        this.listener = listener;
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

    /**
     * Connect to the server. Call this to initiate the TCP connection.
     * The ALKUT handshake is sent automatically once connected.
     */
    public ChannelFuture connect(EventLoopGroup group) throws InterruptedException {
        this.group = group;
        return doConnect();
    }

    private ChannelFuture doConnect() throws InterruptedException {
        TulospalveluTcpConnection handler = this;
        Bootstrap b = new Bootstrap();
        b.group(group)
         .channel(NioSocketChannel.class)
         .option(ChannelOption.SO_KEEPALIVE, true)
         .option(ChannelOption.CONNECT_TIMEOUT_MILLIS, 5000)
         .handler(new ChannelInitializer<SocketChannel>() {
             @Override
             protected void initChannel(SocketChannel ch) {
                 ch.pipeline().addLast(
                     new TulospalveluTcpFrameDecoder(),
                     handler
                 );
             }
         });

        ChannelFuture future = b.connect(serverHost, serverPort).sync();
        this.channel = future.channel();
        return future;
    }

    public void shutdown() {
        shutdownRequested = true;
        if (channel != null) {
            channel.close();
        }
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        this.ctx = ctx;
        log.info("TCP connected to {}:{}", serverHost, serverPort);
        sendAlkut(ctx);
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        connected = false;
        log.info("TCP disconnected from {}:{}", serverHost, serverPort);

        if (!shutdownRequested && group != null && !group.isShuttingDown()) {
            // Reconnect after 3 seconds
            group.schedule(() -> {
                try {
                    log.info("Attempting TCP reconnect to {}:{}...", serverHost, serverPort);
                    doConnect();
                } catch (Exception e) {
                    log.error("Reconnect failed", e);
                }
            }, 3, TimeUnit.SECONDS);
        }
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
                sendSohFrame(ctx, PKGCLASS_KILPPVT, data, label);
                detailLog.run();
                advancePacketId();
            });
        });

        return future;
    }

    // --- Send queue with 500ms pacing (same server constraint as UDP) ---

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

    // --- Inbound ---

    @Override
    protected void channelRead0(ChannelHandlerContext ctx, byte[] frame) throws Exception {
        if (frame.length == 0) return;

        byte first = frame[0];

        if (first == ACK && frame.length >= 4) {
            AckResponse ack = parseAck(frame, 0);
            if (ack != null) handleAck(ack);
        } else if (first == NAK) {
            handleNak();
        } else if (first == SOH && frame.length >= 8) {
            handleIncomingFrame(ctx, frame);
        }
    }

    private void handleAck(AckResponse ack) {
        log.info("ACK for id={}", ack.id() & 0xFF);

        if (!connected) {
            connected = true;
            connectedLatch.countDown();
            log.info("TCP connected to server (ALKUT acknowledged)");
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

    private void handleIncomingFrame(ChannelHandlerContext ctx, byte[] frame) {
        SohFrame sohFrame = parseSohFrame(frame, 0);
        if (sohFrame == null) {
            log.warn("Invalid SOH frame on TCP");
            sendNak(ctx);
            return;
        }

        log.info("Incoming message: pkgclass={}, id={}, len={}",
                sohFrame.pkgclass() & 0xFF, sohFrame.id() & 0xFF, sohFrame.data().length);

        if (inPacketIdInit) {
            inPacketId = (byte) (sohFrame.id() - 1);
            inPacketIdInit = false;
        }

        // ACK the message (C++ server expects ACK even on TCP)
        sendAck(ctx, sohFrame.id());

        if (sohFrame.id() == inPacketId) {
            log.debug("Duplicate message id={}, ACKed but not reprocessed", sohFrame.id() & 0xFF);
            return;
        }
        inPacketId = sohFrame.id();

        dispatchMessage(sohFrame);
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
                log.debug("ALKUT from server");
            default ->
                log.debug("Unhandled message type: {}", frame.pkgclass() & 0xFF);
        }
    }

    // --- Send helpers ---

    private void sendAlkut(ChannelHandlerContext ctx) {
        byte[] data = buildAlkutData(machineId, nrec);
        enqueueSend(() -> {
            sendSohFrame(ctx, PKGCLASS_ALKUT, data, "ALKUT");
            advancePacketId();
        });
    }

    /** TCP sends SOH frame directly on the stream — no STX/port/machineId wrapper. */
    private void sendSohFrame(ChannelHandlerContext ctx, byte pkgclass, byte[] data, String label) {
        int chk = checksum(data);
        byte id = outPacketId;

        ByteBuf buf = Unpooled.buffer(8 + data.length);
        buf.writeByte(SOH);
        buf.writeByte(id);
        buf.writeByte((byte) (255 - (id & 0xFF)));
        buf.writeByte(pkgclass);
        buf.writeShortLE(data.length);
        buf.writeShortLE(chk);
        buf.writeBytes(data);

        log.info("TX {} ({} bytes, id={}, len={}) -> {}:{}",
                label, 8 + data.length, id & 0xFF, data.length, serverHost, serverPort);

        ctx.writeAndFlush(buf);
    }

    /** TCP ACK: 4 bytes directly on stream (no wrapper). */
    private void sendAck(ChannelHandlerContext ctx, byte msgId) {
        ByteBuf buf = Unpooled.buffer(4);
        buf.writeByte(ACK);
        buf.writeByte(msgId);
        buf.writeByte((byte) (255 - (msgId & 0xFF)));
        buf.writeByte(msgId);

        log.debug("TX ACK for id={}", msgId & 0xFF);
        ctx.writeAndFlush(buf);
    }

    private void sendNak(ChannelHandlerContext ctx) {
        ByteBuf buf = Unpooled.buffer(1);
        buf.writeByte(NAK);

        log.debug("TX NAK");
        ctx.writeAndFlush(buf);
    }

    private void advancePacketId() {
        outPacketId++;
        if (outPacketId == 0) outPacketId = 1;
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        log.error("TCP error", cause);
        if (pendingFuture != null) {
            pendingFuture.completeExceptionally(cause);
            pendingFuture = null;
        }
        ctx.close();
    }
}
