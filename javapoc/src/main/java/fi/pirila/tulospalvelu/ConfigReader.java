package fi.pirila.tulospalvelu;

import java.io.BufferedReader;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

/**
 * Reads tulospalvelu laskenta.cfg configuration file.
 *
 * Connection string format: yhteys<N>=<type>:<srvport>/<destaddr>:<destport>
 * Port can be numeric or Y<n> which means PORTBASE + n.
 * PORTBASE default is 15900.
 */
public class ConfigReader {

    private static final int PORTBASE = 15900;

    private String machineId;
    private final List<Connection> connections = new ArrayList<>();
    private boolean emitEnabled;

    public static class Connection {
        public final int index;
        public final String destAddr;
        public final int destPort;
        public final int srvPort;
        public final boolean sendEmit;

        Connection(int index, String destAddr, int destPort, int srvPort, boolean sendEmit) {
            this.index = index;
            this.destAddr = destAddr;
            this.destPort = destPort;
            this.srvPort = srvPort;
            this.sendEmit = sendEmit;
        }

        @Override
        public String toString() {
            return String.format("yhteys%d: %s:%d (srv:%d, emit:%s)",
                    index, destAddr, destPort, srvPort, sendEmit);
        }
    }

    public void read(Path cfgFile) throws IOException {
        // laskenta.cfg may be UTF-8 (with or without BOM) or Windows-1252
        List<String> lines = Files.readAllLines(cfgFile, java.nio.charset.StandardCharsets.UTF_8);
        // Strip UTF-8 BOM if present
        if (!lines.isEmpty() && lines.get(0).startsWith("\uFEFF")) {
            lines.set(0, lines.get(0).substring(1));
        }

        // First pass: read connections
        for (String line : lines) {
            line = line.trim();
            if (line.isEmpty()) continue;

            if (line.startsWith("Kone=")) {
                machineId = line.substring(5).trim();
            } else if (line.equalsIgnoreCase("Emit")) {
                emitEnabled = true;
            } else if (line.toLowerCase().startsWith("yhteys")) {
                parseConnection(line);
            }
        }

        // Second pass: read emit flags (lähemit<N>)
        for (String line : lines) {
            line = line.trim();
            String lower = line.toLowerCase();
            if (lower.startsWith("l\u00e4hemit") || lower.startsWith("lahemit")
                    || lower.startsWith("l&auml;hemit")) {
                // Extract connection number from "lähemit1" etc.
                String numStr = line.replaceAll("[^0-9]", "");
                if (!numStr.isEmpty()) {
                    int idx = Integer.parseInt(numStr);
                    for (Connection c : connections) {
                        if (c.index == idx) {
                            // Mark as emit-sending (reconstruct since fields are final)
                            connections.set(connections.indexOf(c),
                                    new Connection(c.index, c.destAddr, c.destPort, c.srvPort, true));
                        }
                    }
                }
            }
        }
    }

    private void parseConnection(String line) {
        // Format: yhteys<N>=udp:<srvport>/<destaddr>:<destport>
        int eqIdx = line.indexOf('=');
        if (eqIdx < 0) return;

        // Extract connection number from "yhteys1", "yhteys3" etc.
        String prefix = line.substring(0, eqIdx);
        String numStr = prefix.replaceAll("[^0-9]", "");
        if (numStr.isEmpty()) return;
        int connIdx = Integer.parseInt(numStr);

        String value = line.substring(eqIdx + 1).trim();

        // Strip protocol prefix (udp:, tcp:, etc.)
        int colonIdx = value.indexOf(':');
        if (colonIdx < 0) return;
        value = value.substring(colonIdx + 1);

        // Parse: <srvport>/<destaddr>:<destport>
        // Internal connection index is 0-based: yhteys1 = index 0
        int ny = connIdx - 1;
        int srvPort = PORTBASE + 1 + ny;
        int destPort = PORTBASE + 1;
        String destAddr = "localhost";

        String[] parts = value.split("[/:]");
        if (parts.length >= 1) {
            int sp = parseIntSafe(parts[0]);
            if (sp > 0) srvPort = sp;
        }
        if (parts.length >= 2) {
            destAddr = parts[1];
        }
        if (parts.length >= 3) {
            destPort = parsePort(parts[2], ny);
        }

        connections.add(new Connection(connIdx, destAddr, destPort, srvPort, false));
    }

    private int parsePort(String portStr, int ny) {
        portStr = portStr.trim();
        // Y<n> or y<n> means PORTBASE + n
        if (portStr.toLowerCase().startsWith("y")) {
            String numPart = portStr.substring(1);
            int n = parseIntSafe(numPart);
            if (n > 0) return PORTBASE + n;
            return PORTBASE + 1;
        }
        int port = parseIntSafe(portStr);
        return port > 0 ? port : PORTBASE + 1;
    }

    private static int parseIntSafe(String s) {
        try {
            return Integer.parseInt(s.trim());
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    public String getMachineId() {
        return machineId;
    }

    public boolean isEmitEnabled() {
        return emitEnabled;
    }

    public List<Connection> getConnections() {
        return connections;
    }

    /** Find the first connection that sends emit data (lähemit). */
    public Connection getEmitConnection() {
        for (Connection c : connections) {
            if (c.sendEmit) return c;
        }
        return connections.isEmpty() ? null : connections.get(0);
    }
}
