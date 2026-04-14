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
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * Handler for interactive emit card changes.
 * Implements the tulospalvelu UDP protocol:
 * 1. Sends ALKUT (handshake) message
 * 2. Waits for ACK
 * 3. Sends EMITVA (emit card change) message
 * 4. Waits for ACK
 *
 * Wire format (client -> server):
 *   UDP wrapper:  STX(1) + port(2,LE) + machineID(2) = 5 bytes
 *   Protocol msg: SOH(1) + id(1) + iid(1) + pkgclass(1) + len(2,LE) + checksum(2,LE) + data(len)
 *
 * Wire format (server -> client):
 *   Wrapper: 0x0000(2) + machineID(2) = 4 bytes
 *   ACK:     ACK(1) + id(1) + (255-id)(1) + id(1)
 *   NAK:     NAK(1)
 *
 * All multi-byte fields are little-endian (x86 host byte order).
 * All structs use #pragma pack(1) - no alignment padding.
 */
public class InteractiveEmitHandler extends SimpleChannelInboundHandler<DatagramPacket> {

    private static final Logger log = LoggerFactory.getLogger(InteractiveEmitHandler.class);

    // Protocol control characters (TpDef.h)
    private static final byte SOH = 0x01;
    private static final byte STX = 0x02;
    private static final byte ACK = 0x06;
    private static final byte NAK = 0x15;

    // Message types (HkCom32.cpp)
    private static final byte PKGCLASS_ALKUT = 0;
    private static final byte PKGCLASS_KILPPVT = 2;

    // ALKUT data size: tunn(1) + konetunn(2) + vaihe(1) + nrec(2) + flags(4) = 10
    private static final int ALKUT_DATA_SIZE = 10;

    // kilppvtp base size (pack0): 152 bytes (pv_fields without va)
    private static final int KILPPVTPSIZE0 = 152;
    // Badge field offset within packed kilppvtp
    private static final int PV_OFF_BADGE = 68;

    private final int competitorNumber;
    private final String newEmitCard;
    private final String serverHost;
    private final int serverPort;
    private final String machineId;
    private final int nrec;
    private final int recordIndex;     // dk - position in KILP.DAT
    private final byte[] pvData;       // current pv[0] packed data from KILP.DAT
    private final int kilppvtpsize;    // full kilppvtpsize including va slots

    private final CountDownLatch completionLatch = new CountDownLatch(1);
    private volatile boolean success = false;
    private volatile String errorMessage = null;

    // Start at 1: server resets inpakid when it sees ALKUT with id==1
    private byte packetId = 1;

    private enum State { SENDING_ALKUT, WAITING_KILPPVT_ACK, DONE }
    private State state = State.SENDING_ALKUT;

    /**
     * @param recordIndex  dk value (record position in KILP.DAT, 1-based)
     * @param pvData       raw pv[0] bytes from KILP.DAT (kilppvtpsize bytes)
     * @param kilppvtpsize full stage record size (base + intermediate times)
     */
    public InteractiveEmitHandler(int competitorNumber, String newEmitCard,
                                  String serverHost, int serverPort,
                                  String machineId, int nrec,
                                  int recordIndex, byte[] pvData, int kilppvtpsize) {
        this.competitorNumber = competitorNumber;
        this.newEmitCard = newEmitCard;
        this.serverHost = serverHost;
        this.serverPort = serverPort;
        this.machineId = machineId;
        this.nrec = nrec;
        this.recordIndex = recordIndex;
        this.pvData = pvData;
        this.kilppvtpsize = kilppvtpsize;
    }

    @Override
    public void handlerAdded(ChannelHandlerContext ctx) throws Exception {
        if (ctx.channel().isActive()) {
            sendAlkut(ctx);
        }
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        sendAlkut(ctx);
    }

    private InetSocketAddress remoteAddress() {
        return new InetSocketAddress(serverHost, serverPort);
    }

    /**
     * Build and send the ALKUT (handshake) message.
     * ALKUT struct (10 bytes, pack(1)):
     *   tunn(1)=0x01, konetunn(2), vaihe(1), nrec(2,LE), flags(4,LE)
     */
    private void sendAlkut(ChannelHandlerContext ctx) {
        byte[] data = new byte[ALKUT_DATA_SIZE];
        data[0] = 1;                              // tunn = 1 (handshake marker)
        data[1] = (byte) machineId.charAt(0);    // konetunn[0]
        data[2] = (byte) machineId.charAt(1);    // konetunn[1]
        data[3] = 1;                              // vaihe = k_pv+1 (stage 0 -> 1)
        // nrec (2 bytes LE) - must match server's KILP.DAT record count
        writeInt16LE(data, 4, (short) nrec);
        // flags (4 bytes LE) = 0

        sendProtocolMessage(ctx, PKGCLASS_ALKUT, data, "ALKUT");
    }

    /**
     * Build and send the KILPPVT (competitor stage data) message.
     * This is the actual mechanism to update badge assignments on the server.
     *
     * KILPPVT data layout:
     *   tarf(1) + pakota(1) + dk(INT16) + pv(INT16) + valuku(INT16) + cpv(kilppvtpsize)
     *   len = kilppvtpsize + 8
     *
     * The server's tark_kilp(cn, 2) unpacks cpv into kilp.pv[pv] and calls tallenna().
     */
    private void sendKilppvt(ChannelHandlerContext ctx) {
        int badge = (int) Long.parseLong(newEmitCard);

        // Copy pvData and modify the badge field
        byte[] cpv = new byte[kilppvtpsize]; // full size, zeros for va part
        System.arraycopy(pvData, 0, cpv, 0, Math.min(pvData.length, kilppvtpsize));
        writeInt32LE(cpv, PV_OFF_BADGE, badge);

        // Build KILPPVT message: header(8) + cpv(kilppvtpsize)
        int dataLen = 8 + kilppvtpsize;
        byte[] data = new byte[dataLen];
        data[0] = 0;                                  // tarf
        data[1] = 1;                                  // pakota = 1 (force save)
        writeInt16LE(data, 2, (short) recordIndex);   // dk (record position)
        writeInt16LE(data, 4, (short) 0);             // pv = 0 (first stage)
        writeInt16LE(data, 6, (short) 0);             // valuku = 0 (base data only)
        System.arraycopy(cpv, 0, data, 8, kilppvtpsize);

        sendProtocolMessage(ctx, PKGCLASS_KILPPVT, data, "KILPPVT");
        log.info("  competitor={}, dk={}, newBadge={}", competitorNumber, recordIndex, badge);
    }

    /**
     * Wrap data in protocol header and UDP wrapper, then send.
     *
     * Protocol: SOH(1) + id(1) + iid(1) + pkgclass(1) + len(2,LE) + checksum(2,LE) + data
     * UDP wrap: STX(1) + port(2,LE) + machineID(2) + protocol_message
     */
    private void sendProtocolMessage(ChannelHandlerContext ctx, byte pkgclass, byte[] data, String label) {
        int chk = checksum(data);

        ByteBuf buf = Unpooled.buffer(5 + 8 + data.length);

        // UDP wrapper (5 bytes)
        buf.writeByte(STX);
        buf.writeShortLE(serverPort);
        buf.writeBytes(machineId.getBytes(CharsetUtil.US_ASCII));

        // Protocol header (8 bytes)
        buf.writeByte(SOH);
        buf.writeByte(packetId);
        buf.writeByte((byte) (255 - (packetId & 0xFF)));
        buf.writeByte(pkgclass);
        buf.writeShortLE(data.length);
        buf.writeShortLE(chk);

        // Data
        buf.writeBytes(data);

        byte[] raw = new byte[buf.readableBytes()];
        buf.getBytes(0, raw);
        log.info("TX {} ({} bytes, id={}, len={}, chk=0x{}) -> {}",
                label, raw.length, packetId & 0xFF, data.length,
                String.format("%04X", chk), remoteAddress());
        log.debug("TX hex: {}", bytesToHex(raw));

        ctx.writeAndFlush(new DatagramPacket(buf, remoteAddress()));
    }

    @Override
    protected void channelRead0(ChannelHandlerContext ctx, DatagramPacket packet) throws Exception {
        ByteBuf buffer = packet.content();
        byte[] raw = new byte[buffer.readableBytes()];
        buffer.readBytes(raw);

        log.info("RX {} bytes from {}", raw.length, packet.sender());
        log.debug("RX hex: {}", bytesToHex(raw));

        // Server response wrapper: 0x0000(2) + machineID(2) = 4 bytes
        if (raw.length < 4) {
            log.warn("Response too short ({} bytes)", raw.length);
            return;
        }

        String serverMachId = new String(raw, 2, 2, CharsetUtil.US_ASCII);
        log.info("Server machine ID: '{}'", serverMachId);

        // Strip 4-byte wrapper
        int payloadLen = raw.length - 4;
        if (payloadLen == 0) {
            log.warn("Empty payload after wrapper");
            return;
        }
        byte firstByte = raw[4];

        if (firstByte == ACK) {
            handleAck(ctx, raw);
        } else if (firstByte == NAK) {
            log.warn("NAK received - server rejected request");
            errorMessage = "Server rejected the request (NAK)";
            completionLatch.countDown();
        } else if (firstByte == SOH && payloadLen >= 8) {
            // Incoming protocol message from server
            int msgPkgclass = raw[4 + 3] & 0xFF;
            log.info("Received protocol message, pkgclass={}", msgPkgclass);
        } else {
            log.warn("Unknown response, first payload byte: 0x{}",
                    String.format("%02X", firstByte));
        }
    }

    /**
     * Handle ACK response: wrapper(4) + ACK(1) + id(1) + (255-id)(1) + id(1) = 8 bytes total
     */
    private void handleAck(ChannelHandlerContext ctx, byte[] raw) {
        if (raw.length < 8) {
            log.warn("ACK too short ({} bytes, need 8)", raw.length);
            return;
        }

        byte ackId = raw[5];
        byte ackIid = raw[6];
        byte ackId2 = raw[7];

        log.info("ACK: id={}, iid={}, id2={}", ackId & 0xFF, ackIid & 0xFF, ackId2 & 0xFF);

        // Validate: id + iid must equal 255, and id must equal id2
        if (((ackId & 0xFF) + (ackIid & 0xFF)) != 255 || ackId != ackId2) {
            log.warn("Invalid ACK format (id+iid={}, id==id2? {})",
                    (ackId & 0xFF) + (ackIid & 0xFF), ackId == ackId2);
            return;
        }

        // Verify ACK matches our sent packet ID
        if (ackId != packetId) {
            log.warn("ACK id mismatch: expected {}, got {}", packetId & 0xFF, ackId & 0xFF);
            return;
        }

        packetId++;

        switch (state) {
            case SENDING_ALKUT:
                log.info("ALKUT acknowledged, waiting for server to process before sending KILPPVT");
                state = State.WAITING_KILPPVT_ACK;
                // Server's input buffer holds only 1 message at a time for UDP (addseur requires
                // inbseur==inbens). Delay to let tarkcom consume the ALKUT before we send KILPPVT.
                ctx.executor().schedule(() -> sendKilppvt(ctx), 500, java.util.concurrent.TimeUnit.MILLISECONDS);
                break;
            case WAITING_KILPPVT_ACK:
                log.info("KILPPVT acknowledged - emit card change saved on server");
                state = State.DONE;
                success = true;
                completionLatch.countDown();
                break;
            default:
                log.warn("Unexpected ACK in state {}", state);
                break;
        }
    }

    /**
     * Calculate checksum: sum of 16-bit little-endian words, matching the C++ chksum() function.
     * If data length is odd, the last byte is added as-is.
     */
    static int checksum(byte[] data) {
        int sum = 0;
        int i;
        for (i = 0; i + 1 < data.length; i += 2) {
            int word = (data[i] & 0xFF) | ((data[i + 1] & 0xFF) << 8);
            sum += word;
        }
        if (i < data.length) {
            sum += data[i] & 0xFF;
        }
        return sum & 0xFFFF;
    }

    private static void writeInt16LE(byte[] buf, int offset, short value) {
        buf[offset] = (byte) (value & 0xFF);
        buf[offset + 1] = (byte) ((value >> 8) & 0xFF);
    }

    private static void writeInt32LE(byte[] buf, int offset, int value) {
        buf[offset] = (byte) (value & 0xFF);
        buf[offset + 1] = (byte) ((value >> 8) & 0xFF);
        buf[offset + 2] = (byte) ((value >> 16) & 0xFF);
        buf[offset + 3] = (byte) ((value >> 24) & 0xFF);
    }

    public boolean awaitCompletion(long timeout, TimeUnit unit) throws InterruptedException {
        return completionLatch.await(timeout, unit);
    }

    public boolean isSuccess() {
        return success;
    }

    public String getErrorMessage() {
        return errorMessage;
    }

    private String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02X ", b));
        }
        return sb.toString().trim();
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        log.error("Network error", cause);
        errorMessage = cause.getMessage();
        completionLatch.countDown();
    }
}
