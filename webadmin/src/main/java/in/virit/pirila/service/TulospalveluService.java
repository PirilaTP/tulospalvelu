package in.virit.pirila.service;

import fi.pirila.tulospalvelu.ConfigReader;
import fi.pirila.tulospalvelu.KilpReader;
import fi.pirila.tulospalvelu.KilpSrjReader;
import fi.pirila.tulospalvelu.MessageListener;
import fi.pirila.tulospalvelu.TulospalveluConnection;
import fi.pirila.tulospalvelu.TulospalveluTcpConnection;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioDatagramChannel;
import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

/**
 * Manages the pirila-udp connection lifecycle and competitor data.
 * Reads KILP.DAT for competitor data and establishes UDP connection
 * to the tulospalvelu server for real-time message exchange.
 */
@Service
public class TulospalveluService implements MessageListener {

    private static final Logger log = LoggerFactory.getLogger(TulospalveluService.class);

    @Value("${tulospalvelu.data-dir:}")
    private String defaultDataDir;

    @Value("${tulospalvelu.auto-start:false}")
    private boolean autoStart;

    private volatile boolean started = false;
    private volatile String password;
    private volatile List<fi.pirila.tulospalvelu.Competitor> competitors = List.of();
    private KilpSrjReader kilpSrjReader;
    private TulospalveluConnection udpConnection;
    private TulospalveluTcpConnection tcpConnection;
    private EventLoopGroup eventLoopGroup;
    private Channel channel;
    private Path kilpFile;

    @PostConstruct
    public void init() {
        if (autoStart && defaultDataDir != null && !defaultDataDir.isBlank()) {
            log.info("Auto-starting with data-dir: {}", defaultDataDir);
            try {
                start(defaultDataDir, null);
            } catch (Exception e) {
                log.error("Auto-start failed", e);
            }
        }
    }

    /**
     * Starts the service by reading data from the given directory.
     * @param dataDir path to directory containing KILP.DAT and laskenta.cfg
     * @param password optional password required for card changes, null or blank to disable
     */
    public void start(String dataDir, String password) {
        if (started) {
            log.warn("Service already started, ignoring");
            return;
        }

        this.password = (password != null && !password.isBlank()) ? password : null;

        Path dir = Path.of(dataDir);
        if (!Files.isDirectory(dir)) {
            throw new IllegalArgumentException("Hakemistoa ei löydy: " + dir.toAbsolutePath());
        }
        log.info("Using data directory: {}", dir);

        kilpFile = dir.resolve("KILP.DAT");
        if (Files.exists(kilpFile)) {
            try {
                competitors = KilpReader.read(kilpFile);
                log.info("Loaded {} competitors from KILP.DAT", competitors.size());
            } catch (IOException e) {
                throw new RuntimeException("KILP.DAT lukeminen epäonnistui", e);
            }
        } else {
            throw new IllegalArgumentException("KILP.DAT ei löydy hakemistosta: " + dir.toAbsolutePath());
        }

        Path srjFile = dir.resolve("KilpSrj.xml");
        if (!Files.exists(srjFile)) {
            srjFile = dir.getParent() != null ? dir.getParent().resolve("KilpSrj.xml") : null;
        }
        if (srjFile != null && Files.exists(srjFile)) {
            try {
                kilpSrjReader = new KilpSrjReader();
                kilpSrjReader.read(srjFile);
                log.info("Loaded {} classes from KilpSrj.xml", kilpSrjReader.getClassNames().size());
            } catch (IOException e) {
                log.warn("Failed to read KilpSrj.xml: {}", e.getMessage());
            }
        }

        Path cfgFile = dir.resolve("laskenta.cfg");
        if (Files.exists(cfgFile)) {
            try {
                ConfigReader config = new ConfigReader();
                config.read(cfgFile);
                log.info("Config loaded: machine={}, emit={}", config.getMachineId(), config.isEmitEnabled());

                log.info("laskenta.cfg connections: {}", config.getConnections());
                fi.pirila.tulospalvelu.Connection conn = config.getEmitConnection();
                if (conn != null) {
                    int nrec = KilpReader.readNumrec(kilpFile);
                    String machineId = config.getMachineId() != null ? config.getMachineId() : "W1";
                    log.info("Selected connection: protocol={}, dest={}:{}, srvPort={}, machineId={}, nrec={}",
                            conn.protocol(), conn.destAddr(), conn.destPort(), conn.srvPort(), machineId, nrec);
                    if (conn.isTcp()) {
                        setupTcpConnection(conn.destAddr(), conn.destPort(), machineId, nrec);
                    } else {
                        setupUdpConnection(conn.destAddr(), conn.destPort(), conn.srvPort(), machineId, nrec);
                    }
                } else {
                    log.warn("No suitable connection found in laskenta.cfg, available: {}", config.getConnections());
                }
            } catch (RuntimeException e) {
                throw e;
            } catch (Exception e) {
                throw new RuntimeException("Yhteyden muodostus epäonnistui", e);
            }
        }

        started = true;
        log.info("TulospalveluService started, competitors={}, password={}", competitors.size(), this.password != null ? "set" : "not set");
    }

    public boolean isStarted() {
        return started;
    }

    public boolean checkPassword(String input) {
        if (password == null) return true;
        return password.equals(input);
    }

    public boolean isPasswordRequired() {
        return password != null;
    }

    private void setupUdpConnection(String host, int port, int srvPort, String machineId, int nrec) throws Exception {
        log.info("Setting up UDP connection: host={}, port={}, srvPort={}, machineId={}, nrec={}",
                host, port, srvPort, machineId, nrec);
        udpConnection = new TulospalveluConnection(host, port, machineId, nrec);
        udpConnection.setListener(this);
        eventLoopGroup = new NioEventLoopGroup();

        Bootstrap bootstrap = new Bootstrap();
        bootstrap.group(eventLoopGroup)
                .channel(NioDatagramChannel.class)
                .handler(new ChannelInitializer<NioDatagramChannel>() {
                    @Override
                    protected void initChannel(NioDatagramChannel ch) {
                        log.info("Netty channel initialized: local={}", ch.localAddress());
                        ch.pipeline().addLast(udpConnection);
                    }
                });

        channel = bootstrap.bind(srvPort).sync().channel();
        log.info("UDP channel bound to {}, channel.isActive={}, channel.isOpen={}",
                channel.localAddress(), channel.isActive(), channel.isOpen());

        log.info("Waiting for ALKUT handshake (timeout 5s)...");
        boolean connected = udpConnection.awaitConnected(5, TimeUnit.SECONDS);
        if (connected) {
            log.info("ALKUT handshake OK - connected to tulospalvelu server at {}:{}", host, port);
        } else {
            log.warn("ALKUT handshake timed out after 5s - server at {}:{} did not respond", host, port);
            log.warn("Channel state after timeout: isActive={}, isOpen={}", channel.isActive(), channel.isOpen());
        }
    }

    private void setupTcpConnection(String host, int port, String machineId, int nrec) throws Exception {
        log.info("Setting up TCP connection: host={}, port={}, machineId={}, nrec={}",
                host, port, machineId, nrec);
        tcpConnection = new TulospalveluTcpConnection(host, port, machineId, nrec);
        tcpConnection.setListener(this);
        eventLoopGroup = new NioEventLoopGroup();

        tcpConnection.connect(eventLoopGroup);

        log.info("Waiting for TCP ALKUT handshake (timeout 5s)...");
        boolean connected = tcpConnection.awaitConnected(5, TimeUnit.SECONDS);
        if (connected) {
            log.info("TCP ALKUT handshake OK - connected to tulospalvelu server at {}:{}", host, port);
        } else {
            log.warn("TCP ALKUT handshake timed out after 5s - server at {}:{} did not respond", host, port);
        }
    }

    @PreDestroy
    public void shutdown() {
        log.info("Shutting down TulospalveluService, connected={}", isConnected());
        if (tcpConnection != null) tcpConnection.shutdown();
        if (channel != null) channel.close();
        if (eventLoopGroup != null) eventLoopGroup.shutdownGracefully();
    }

    // --- Public API ---

    public List<fi.pirila.tulospalvelu.Competitor> getCompetitors() {
        return competitors;
    }

    public String getClassName(int sarja) {
        return kilpSrjReader != null ? kilpSrjReader.getClassName(sarja) : String.valueOf(sarja);
    }

    public fi.pirila.tulospalvelu.Competitor getCompetitorByRecordIndex(int recordIndex) {
        for (fi.pirila.tulospalvelu.Competitor c : competitors) {
            if (c.recordIndex == recordIndex) return c;
        }
        return null;
    }

    public boolean isConnected() {
        if (tcpConnection != null) return tcpConnection.isConnected();
        return udpConnection != null && udpConnection.isConnected();
    }

    public boolean isActive() {
        if (tcpConnection != null) return tcpConnection.isActive();
        return udpConnection != null && udpConnection.isActive();
    }

    public boolean sendCardChange(int recordIndex, int newBadge) {
        log.info("sendCardChange called: recordIndex={}, newBadge={}, connected={}",
                recordIndex, newBadge, isConnected());
        if (!isConnected()) {
            log.warn("Cannot send card change - not connected to server");
            return false;
        }
        if (kilpFile == null) {
            log.warn("Cannot send card change - KILP.DAT not available");
            return false;
        }

        try {
            byte[] pvData = KilpReader.readPvData(kilpFile, recordIndex);
            int kilppvtpsize = KilpReader.getKilppvtpsize();

            CompletableFuture<Boolean> result;
            if (tcpConnection != null) {
                result = tcpConnection.sendKilppvt(recordIndex, pvData, kilppvtpsize, newBadge);
            } else {
                result = udpConnection.sendKilppvt(recordIndex, pvData, kilppvtpsize, newBadge);
            }

            Boolean success = result.get(10, TimeUnit.SECONDS);
            if (Boolean.TRUE.equals(success)) {
                KilpReader.writeBadge(kilpFile, recordIndex, newBadge);
                fi.pirila.tulospalvelu.Competitor comp = getCompetitorByRecordIndex(recordIndex);
                if (comp != null) comp.badge = newBadge;
                log.info("Card change successful: record={}, newBadge={}", recordIndex, newBadge);
                return true;
            }
            log.warn("Card change rejected by server: record={}", recordIndex);
            return false;
        } catch (Exception e) {
            log.error("Card change failed for record={}", recordIndex, e);
            return false;
        }
    }

    // --- Server-initiated updates ---

    @Override
    public void onCompetitorUpdate(int dk, int pv, byte[] cpvData) {
        if (cpvData.length < 76) return;

        int badge = (cpvData[68] & 0xFF) | ((cpvData[69] & 0xFF) << 8)
                | ((cpvData[70] & 0xFF) << 16) | ((cpvData[71] & 0xFF) << 24);

        fi.pirila.tulospalvelu.Competitor comp = getCompetitorByRecordIndex(dk);
        if (comp != null) {
            comp.badge = badge;
            if (kilpFile != null) {
                try {
                    KilpReader.writeBadge(kilpFile, dk, badge);
                } catch (Exception e) {
                    log.warn("Failed to update local KILP.DAT: {}", e.getMessage());
                }
            }
            log.info("Server updated: {} {} emit -> {}", comp.sukunimi, comp.etunimi, badge);
        }
    }
}
