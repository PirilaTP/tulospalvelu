package fi.pirila.tulospalvelu;

import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioDatagramChannel;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Scanner;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

/**
 * Interactive emit card changer with persistent server connection.
 * Reads configuration from laskenta.cfg and competitors from KILP.DAT.
 * Listens for server-initiated KILPPVT messages to keep local data in sync.
 */
public class InteractiveEmitChanger implements MessageListener {

    private static final Logger log = LoggerFactory.getLogger(InteractiveEmitChanger.class);

    private final String serverHost;
    private final int serverPort;
    private final EventLoopGroup group;
    private Channel channel;
    private TulospalveluConnection connection;
    private Scanner scanner;
    private List<Competitor> competitors;
    private String machineId = "J1";
    private int nrec = 0;
    private int srvPort = 0; // local port to bind to (from config, 0 = random)
    private Path kilpFile;

    public InteractiveEmitChanger(String serverHost, int serverPort) {
        this.serverHost = serverHost;
        this.serverPort = serverPort;
        this.group = new NioEventLoopGroup();
        this.scanner = new Scanner(System.in);
    }

    public void start() throws Exception {
        System.out.println("Tulospalvelu Interactive Emit Card Changer");
        System.out.println("==========================================");
        System.out.println("Target: " + serverHost + ":" + serverPort);
        System.out.println();

        // Connect and perform ALKUT handshake
        if (!connect()) {
            System.out.println("Failed to connect to server. Exiting.");
            shutdown();
            return;
        }

        System.out.println("Connected! Enter 'quit' to exit, 'list' to show competitors");
        System.out.println();

        while (true) {
            System.out.print("Enter competitor number (or 'quit'/'list'): ");
            String input = scanner.nextLine().trim();

            if (input.equalsIgnoreCase("quit") || input.equalsIgnoreCase("exit")) {
                break;
            }

            if (input.equalsIgnoreCase("list")) {
                listCompetitors();
                continue;
            }

            try {
                int competitorNumber = Integer.parseInt(input);

                Competitor comp = findCompetitor(competitorNumber);
                if (comp == null) {
                    System.out.println("Competitor " + competitorNumber + " not found");
                    continue;
                }

                System.out.println("  " + comp.sukunimi + " " + comp.etunimi
                        + " (" + comp.seura + ")"
                        + (comp.badge > 0 ? ", current emit: " + comp.badge : ""));

                System.out.print("Enter new emit card number: ");
                String newEmitCard = scanner.nextLine().trim();

                sendEmitCardChange(comp, newEmitCard);

            } catch (NumberFormatException e) {
                System.out.println("Invalid input: " + input);
            }
        }

        shutdown();
        System.out.println("Application terminated.");
    }

    private boolean connect() throws Exception {
        connection = new TulospalveluConnection(serverHost, serverPort, machineId, nrec);
        connection.setListener(this);

        Bootstrap bootstrap = new Bootstrap();
        bootstrap.group(group)
                .channel(NioDatagramChannel.class)
                .handler(new ChannelInitializer<NioDatagramChannel>() {
                    @Override
                    protected void initChannel(NioDatagramChannel ch) {
                        ch.pipeline().addLast(connection);
                    }
                });

        channel = bootstrap.bind(srvPort).sync().channel();
        log.info("UDP channel bound to {}", channel.localAddress());

        System.out.println("Connecting to " + serverHost + ":" + serverPort + "...");
        boolean ok = connection.awaitConnected(10, TimeUnit.SECONDS);
        if (!ok) {
            System.out.println("TIMEOUT waiting for ALKUT handshake");
        }
        return ok && connection.isConnected();
    }

    private void sendEmitCardChange(Competitor comp, String newEmitCard) {
        try {
            byte[] pvData = KilpReader.readPvData(kilpFile, comp.recordIndex);
            int kilppvtpsize = KilpReader.getKilppvtpsize();
            int badge = (int) Long.parseLong(newEmitCard);

            System.out.println("Sending KILPPVT: competitor=" + comp.kilpno
                    + " dk=" + comp.recordIndex + " newEmit=" + newEmitCard);

            CompletableFuture<Boolean> result = connection.sendKilppvt(
                    comp.recordIndex, pvData, kilppvtpsize, badge);

            Boolean success = result.get(10, TimeUnit.SECONDS);
            if (Boolean.TRUE.equals(success)) {
                System.out.println("OK - emit card change accepted");
                comp.badge = badge;
                KilpReader.writeBadge(kilpFile, comp.recordIndex, badge);
            } else {
                System.out.println("FAILED - server rejected request");
            }
        } catch (Exception e) {
            System.out.println("ERROR - " + e.getMessage());
            log.error("Error sending emit card change", e);
        }
    }

    // --- KilppvtListener: server-initiated updates ---

    @Override
    public void onCompetitorUpdate(int dk, int pv, byte[] cpvData) {
        if (cpvData.length < 76) return;

        int badge = (cpvData[68] & 0xFF) | ((cpvData[69] & 0xFF) << 8)
                | ((cpvData[70] & 0xFF) << 16) | ((cpvData[71] & 0xFF) << 24);

        Competitor comp = findByRecordIndex(dk);
        if (comp != null) {
            int oldBadge = comp.badge;
            comp.badge = badge;
            if (kilpFile != null) {
                try { KilpReader.writeBadge(kilpFile, dk, badge); } catch (Exception e) {
                    log.warn("Failed to update local KILP.DAT: {}", e.getMessage());
                }
            }
            System.out.println();
            System.out.println(">> Server updated: " + comp.sukunimi + " " + comp.etunimi
                    + " (no " + comp.kilpno + ") emit: " + oldBadge + " -> " + badge);
            System.out.print("Enter competitor number (or 'quit'/'list'): ");
        } else {
            System.out.println();
            System.out.println(">> Server updated record dk=" + dk + " badge=" + badge);
            System.out.print("Enter competitor number (or 'quit'/'list'): ");
        }
    }

    // --- Helpers ---

    private void listCompetitors() {
        if (competitors == null || competitors.isEmpty()) {
            System.out.println("No competitor data loaded");
            return;
        }
        System.out.printf("%4s  %-20s %-15s %-20s %s%n", "No", "Sukunimi", "Etunimi", "Seura", "Emit");
        System.out.println("-".repeat(75));
        for (Competitor c : competitors) {
            System.out.println(c);
        }
        System.out.println(competitors.size() + " competitors");
    }

    private Competitor findCompetitor(int kilpno) {
        if (competitors == null) return null;
        for (Competitor c : competitors) {
            if (c.kilpno == kilpno) return c;
        }
        return null;
    }

    private Competitor findByRecordIndex(int dk) {
        if (competitors == null) return null;
        for (Competitor c : competitors) {
            if (c.recordIndex == dk) return c;
        }
        return null;
    }

    private void shutdown() {
        if (channel != null) {
            channel.close();
        }
        group.shutdownGracefully();
        if (scanner != null) {
            scanner.close();
        }
    }

    public static void main(String[] args) {
        String serverHost = null;
        int serverPort = 0;
        Path dataDir = null;

        for (String candidate : new String[]{"kisat/J1Data", "kisat/HkMaaliData",
                "../kisat/J1Data", "../kisat/HkMaaliData"}) {
            Path p = Path.of(candidate);
            if (Files.isDirectory(p)) {
                dataDir = p;
                break;
            }
        }

        if (args.length >= 2) {
            serverHost = args[0];
            serverPort = Integer.parseInt(args[1]);
        } else if (args.length == 1 && Files.isDirectory(Path.of(args[0]))) {
            dataDir = Path.of(args[0]);
        }

        ConfigReader config = null;
        if (dataDir != null) {
            Path cfgFile = dataDir.resolve("laskenta.cfg");
            if (Files.exists(cfgFile)) {
                try {
                    config = new ConfigReader();
                    config.read(cfgFile);
                    System.out.println("Config loaded: machine=" + config.getMachineId()
                            + ", emit=" + config.isEmitEnabled());
                    for (Connection c : config.getConnections()) {
                        System.out.println("  " + c);
                    }
                } catch (Exception e) {
                    System.err.println("Warning: could not read " + cfgFile + ": " + e.getMessage());
                }
            }
        }

        if (serverHost == null && config != null) {
            Connection conn = config.getEmitConnection();
            if (conn != null) {
                serverHost = conn.destAddr();
                serverPort = conn.destPort();
            }
        }

        // Get srvPort for local binding (so server can send back to us)
        int localSrvPort = 0;
        if (config != null) {
            Connection conn = config.getEmitConnection();
            if (conn != null) {
                localSrvPort = conn.srvPort();
            }
        }

        if (serverHost == null) {
            System.out.println("Usage: java -jar emit-changer.jar [<server_host> <server_port>]");
            System.out.println("   or: java -jar emit-changer.jar [<data_dir>]");
            System.out.println("Config auto-detected from kisat/J1Data/laskenta.cfg if present.");
            System.exit(1);
        }

        try {
            InteractiveEmitChanger changer = new InteractiveEmitChanger(serverHost, serverPort);

            if (config != null && config.getMachineId() != null) {
                changer.machineId = config.getMachineId();
            }
            changer.srvPort = localSrvPort;

            if (dataDir != null) {
                Path kf = dataDir.resolve("KILP.DAT");
                if (Files.exists(kf)) {
                    try {
                        changer.kilpFile = kf;
                        changer.competitors = KilpReader.read(kf);
                        changer.nrec = KilpReader.readNumrec(kf);
                        System.out.println("Loaded " + changer.competitors.size()
                                + " competitors from KILP.DAT (nrec=" + changer.nrec + ")");
                    } catch (Exception e) {
                        System.err.println("Warning: could not read KILP.DAT: " + e.getMessage());
                    }
                }
            }

            System.out.println();
            changer.start();
        } catch (Exception e) {
            LoggerFactory.getLogger(InteractiveEmitChanger.class)
                    .error("Fatal error", e);
            System.exit(1);
        }
    }
}
