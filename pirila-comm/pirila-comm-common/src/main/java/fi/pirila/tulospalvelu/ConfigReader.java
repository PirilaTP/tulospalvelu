package fi.pirila.tulospalvelu;

import java.io.IOException;
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
                    for (int i = 0; i < connections.size(); i++) {
                        Connection c = connections.get(i);
                        if (c.index() == idx) {
                            connections.set(i,
                                    new Connection(c.index(), c.protocol(), c.destAddr(), c.destPort(), c.srvPort(), true));
                        }
                    }
                }
            }
        }
    }

    private void parseConnection(String line) {
        // Full:    yhteys<N>=udp:<srvport>/<destaddr>:<destport>
        // Passive: yhteys<N>=udp                       (listen only; destAddr=AUTO, destPort=0)
        int eqIdx = line.indexOf('=');
        if (eqIdx < 0) return;

        // Extract connection number from "yhteys1", "yhteys3" etc.
        String prefix = line.substring(0, eqIdx);
        String numStr = prefix.replaceAll("[^0-9]", "");
        if (numStr.isEmpty()) return;
        int connIdx = Integer.parseInt(numStr);

        String value = line.substring(eqIdx + 1).trim();
        if (value.isEmpty()) return;

        // C++ HkInit.cpp:393-394 defaults: srvport=PORTBASE+1+ny, destPort=PORTBASE+1, destAddr="AUTO"
        // Internal connection index is 0-based: yhteys1 -> ny=0
        int ny = connIdx - 1;
        int srvPort = PORTBASE + 1 + ny;
        int destPort = PORTBASE + 1;
        String destAddr = "AUTO";

        // Split protocol from rest. Bare "UDP" / "TCP" with no separator is the passive case.
        String protocol;
        String rest;
        int sepIdx = indexOfAny(value, ":/=,");
        if (sepIdx < 0) {
            protocol = value.toLowerCase();
            rest = "";
        } else {
            protocol = value.substring(0, sepIdx).toLowerCase();
            rest = value.substring(sepIdx + 1);
        }

        if (!rest.isEmpty()) {
            String[] parts = rest.split("[/:]");
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
        }

        // C++ HkInit.cpp:424-425: if destAddr stays "AUTO", destPort is forced to 0 (passive)
        if ("AUTO".equalsIgnoreCase(destAddr)) {
            destPort = 0;
        }

        connections.add(new Connection(connIdx, protocol, destAddr, destPort, srvPort, false));
    }

    private static int indexOfAny(String s, String chars) {
        int min = -1;
        for (int i = 0; i < chars.length(); i++) {
            int idx = s.indexOf(chars.charAt(i));
            if (idx >= 0 && (min < 0 || idx < min)) min = idx;
        }
        return min;
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
            if (c.sendEmit()) return c;
        }
        return connections.isEmpty() ? null : connections.get(0);
    }
}
