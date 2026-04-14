package fi.pirila.tulospalvelu;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import io.netty.channel.socket.DatagramPacket;
import io.netty.util.CharsetUtil;
import io.netty.util.AttributeKey;

import java.net.InetSocketAddress;

public class EmitCardChangeHandler extends SimpleChannelInboundHandler<DatagramPacket> {
    
    private final int competitorNumber;
    private final String newEmitCard;
    
    // Protocol constants from the C++ code
    private static final byte SOH = 0x01;
    private static final byte STX = 0x02;
    private static final byte ETX = 0x03;
    private static final byte ACK = 0x06;
    private static final byte NAK = 0x15;

    public EmitCardChangeHandler(int competitorNumber, String newEmitCard) {
        this.competitorNumber = competitorNumber;
        this.newEmitCard = newEmitCard;
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        // Connect to the server first
        InetSocketAddress remoteAddress = new InetSocketAddress(
            System.getProperty("server.host", "localhost"),
            Integer.parseInt(System.getProperty("server.port", "15900"))
        );
        
        // Store remote address for later use
        ctx.channel().attr(REMOTE_ADDRESS_KEY).set(remoteAddress);
        
        // First send machine ID handshake and wait for response
        sendMachineId(ctx);
        // The emit card change will be sent after we receive the machine ID response
    }
    
    private void sendEmitCardChangeRequest(ChannelHandlerContext ctx) {
        // First send our machine ID (J1) as handshake
        sendMachineId(ctx);
        
        // Then send the actual emit card change request using correct protocol format
        ByteBuf buffer = Unpooled.buffer();
        
        // UDP wrapper: STX + port + machineID
        buffer.writeByte(STX);  // Start of text
        buffer.writeShort(0);   // Port number (will be filled by system)
        buffer.writeBytes("J1".getBytes(CharsetUtil.US_ASCII));  // Our machine ID
        
        // Actual message starts here
        int messageStart = buffer.writerIndex();
        
        // Start of heading
        buffer.writeByte(SOH);
        
        // Message ID and internal ID (can be 0 for simple requests)
        buffer.writeByte(0);    // ID
        buffer.writeByte(0);    // Internal ID
        
        // Message type - Try original observed type 0x45
        buffer.writeByte(0x45);
        
        // Length placeholder (will be calculated)
        int lengthPosition = buffer.writerIndex();
        buffer.writeShort(0);  // Length (will be filled later)
        
        // Skip checksum for now (set to 0)
        buffer.writeShort(0);  // Checksum (set to 0 for testing)
        
        // EMITVA data structure (ev)
        buffer.writeShort(0);          // ekno (old competitor number) - 0 for new assignment
        buffer.writeShort(0);          // eos (old leg) - 0 for new assignment
        buffer.writeShort(competitorNumber);  // kno (new competitor number)
        buffer.writeShort(0);          // os (new leg) - 0 for main competitor
        buffer.writeInt((int) Long.parseLong(newEmitCard));  // badge (new emit card number)
        
        // Calculate length (data only, from after checksum field)
        int dataLength = buffer.writerIndex() - (lengthPosition + 2);
        buffer.setShort(lengthPosition, dataLength);
        
        // Get the remote address
        InetSocketAddress remoteAddress = ctx.channel().attr(REMOTE_ADDRESS_KEY).get();
        
        // Debug: Log the complete message bytes
        byte[] messageBytes = new byte[buffer.readableBytes()];
        buffer.getBytes(0, messageBytes);
        System.out.println("Complete message (hex): " + bytesToHex(messageBytes));
        
        // Reset reader index after debugging
        buffer.resetReaderIndex();
        
        // Create and send the packet
        DatagramPacket packet = new DatagramPacket(buffer, remoteAddress);
        ctx.writeAndFlush(packet);
        
        System.out.println("Sent emit card change request (EMITVA) to " + remoteAddress);
        System.out.println("Debug: Competitor " + competitorNumber + " -> Emit card " + newEmitCard);
    }
    
    private void sendMachineId(ChannelHandlerContext ctx) {
        // Build UDP packet for machine ID handshake
        ByteBuf buffer = Unpooled.buffer();
        
        // Machine ID message format
        buffer.writeByte(STX);  // Start of text
        buffer.writeShort(0);   // Port number (will be filled by system)
        buffer.writeBytes("J1".getBytes(CharsetUtil.US_ASCII));  // Our machine ID
        
        // Get the remote address
        InetSocketAddress remoteAddress = ctx.channel().attr(REMOTE_ADDRESS_KEY).get();
        
        // Create and send the packet
        DatagramPacket packet = new DatagramPacket(buffer, remoteAddress);
        ctx.writeAndFlush(packet);
        
        System.out.println("Sent machine ID handshake: J1");
    }
    
    // Attribute key for storing remote address
    private static final AttributeKey<InetSocketAddress> REMOTE_ADDRESS_KEY = 
        AttributeKey.valueOf("remote.address");

    @Override
    protected void channelRead0(ChannelHandlerContext ctx, DatagramPacket packet) throws Exception {
        ByteBuf buffer = packet.content();
        
        // Debug: Log raw response bytes
        byte[] responseBytes = new byte[buffer.readableBytes()];
        buffer.readBytes(responseBytes);
        System.out.println("Raw response (hex): " + bytesToHex(responseBytes));
        
        // Reset reader index to process the response
        buffer.resetReaderIndex();
        
        // Check if this is a machine ID response (starts with 00 00 and contains machine ID)
        if (responseBytes.length >= 5 && responseBytes[0] == 0x00 && responseBytes[1] == 0x00) {
            // This is a machine ID response, extract the machine ID
            String machineId = new String(responseBytes, 2, 2, CharsetUtil.US_ASCII);
            System.out.println("✅ Received machine ID response: " + machineId);
            
            // Store the remote machine ID for future reference
            ctx.channel().attr(REMOTE_MACHINE_ID_KEY).set(machineId);
            
            // Now send the actual emit card change request
            sendEmitCardChangeRequest(ctx);
            return;
        }
        
        // Read response as emit card change acknowledgment
        if (buffer.readableBytes() > 0) {
            byte responseType = buffer.readByte();
            
            if (responseType == ACK) {
                System.out.println("✅ Success: Emit card change acknowledged");
            } else if (responseType == NAK) {
                System.out.println("❌ Error: Emit card change rejected");
                if (buffer.readableBytes() > 0) {
                    String errorMsg = buffer.toString(CharsetUtil.US_ASCII);
                    System.out.println("Error details: " + errorMsg);
                }
            } else {
                System.out.println("⚠️  Unknown response type: 0x" + Integer.toHexString(responseType));
                System.out.println("Response data: " + buffer.toString(CharsetUtil.US_ASCII));
            }
        }
        
        // Close connection after response
        ctx.close();
    }
    
    // Attribute key for storing remote machine ID
    private static final AttributeKey<String> REMOTE_MACHINE_ID_KEY = 
        AttributeKey.valueOf("remote.machine.id");
    
    // Helper method to convert bytes to hex string
    private String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02X ", b));
        }
        return sb.toString();
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        System.err.println("Network error: " + cause.getMessage());
        ctx.close();
    }
}