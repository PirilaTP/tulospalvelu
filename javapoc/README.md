# Tulospalvelu Java POC

Proof of Concept for communicating with Pekka Pirilä's tulospalvelu system using Java.

## Purpose

This POC demonstrates how to change a competitor's emit card using UDP protocol, enabling web/mobile interfaces for specific tulospalvelu functionalities.

## Features

- UDP communication with tulospalvelu server
- Emit card change functionality
- Netty-based network stack
- Simple CLI interface

## Protocol Details

The application uses a custom UDP protocol:

- **Port**: Typically 15900 (configurable)
- **Message Format**:
  - `SOH` (0x01) - Start of message
  - Message type (0x45 for emit card change)
  - Competitor number (4 bytes, big endian)
  - Emit card (12 bytes, space-padded)
  - `ETX` (0x03) - End of message

- **Responses**:
  - `ACK` (0x06) - Success
  - `NAK` (0x15) - Error

## Building

```bash
mvn clean package
```

## Usage

```bash
java -jar target/tulospalvelu-java-1.0.0-SNAPSHOT.jar <competitor_number> <new_emit_card> [host] [port]
```

**Examples:**

```bash
# Basic usage (localhost:15900)
java -jar tulospalvelu-java.jar 123 ABC123456

# Custom host and port
java -jar tulospalvelu-java.jar 123 ABC123456 192.168.1.100 15901
```

## Next Steps

1. Test with actual tulospalvelu server
2. Analyze real protocol traffic to refine message format
3. Add more functionality (results query, competitor info)
4. Build web interface using this as backend

## License

GPLv3 - Same as original tulospalvelu project