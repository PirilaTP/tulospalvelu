package fi.pirila.tulospalvelu;

import static fi.pirila.tulospalvelu.TulospalveluProtocol.*;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;

import java.util.List;

/**
 * Decodes the TCP byte stream into individual protocol frames.
 *
 * The C++ server sends three kinds of data on the TCP stream:
 * <ul>
 *   <li>SOH frames: SOH(1) + id(1) + iid(1) + pkgclass(1) + len(2 LE) + chk(2 LE) + data(len)</li>
 *   <li>ACK responses: ACK(1) + id(1) + (255-id)(1) + id(1) = 4 bytes</li>
 *   <li>NAK responses: NAK(1) = 1 byte</li>
 * </ul>
 *
 * This decoder accumulates bytes and emits complete frames as byte[] to the next handler.
 */
public class TulospalveluTcpFrameDecoder extends ByteToMessageDecoder {

    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) throws Exception {
        while (in.readableBytes() > 0) {
            in.markReaderIndex();
            byte first = in.getByte(in.readerIndex());

            if (first == SOH) {
                // SOH frame: need at least 8 bytes for header
                if (in.readableBytes() < 8) {
                    in.resetReaderIndex();
                    return;
                }
                // Read length from offset 4-5 (little-endian)
                int len = (in.getByte(in.readerIndex() + 4) & 0xFF)
                        | ((in.getByte(in.readerIndex() + 5) & 0xFF) << 8);
                int totalLen = 8 + len;
                if (in.readableBytes() < totalLen) {
                    in.resetReaderIndex();
                    return;
                }
                byte[] frame = new byte[totalLen];
                in.readBytes(frame);
                out.add(frame);

            } else if (first == ACK) {
                // ACK: 4 bytes
                if (in.readableBytes() < 4) {
                    in.resetReaderIndex();
                    return;
                }
                byte[] frame = new byte[4];
                in.readBytes(frame);
                out.add(frame);

            } else if (first == NAK) {
                // NAK: 1 byte
                byte[] frame = new byte[1];
                in.readBytes(frame);
                out.add(frame);

            } else {
                // Unknown byte - skip it
                in.readByte();
            }
        }
    }
}
