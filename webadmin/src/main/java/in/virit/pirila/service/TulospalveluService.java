package in.virit.pirila.service;

import fi.pirila.tulospalvelu.ConfigReader;
import fi.pirila.tulospalvelu.KilpReader;
import fi.pirila.tulospalvelu.TulospalveluConnection;
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
public class TulospalveluService implements TulospalveluConnection.KilppvtListener {

    private static final Logger log = LoggerFactory.getLogger(TulospalveluService.class);

    @Value("${tulospalvelu.data-dir:}")
    private String dataDir;

    @Value("${tulospalvelu.connection-index:0}")
    private int connectionIndex;

    private volatile List<KilpReader.Competitor> competitors = List.of();
    private TulospalveluConnection connection;
    private EventLoopGroup eventLoopGroup;
    private Channel channel;
    private Path kilpFile;

    @PostConstruct
    public void init() {
        Path dir = resolveDataDir();
        if (dir == null) {
            log.warn("Tulospalvelu data directory not found - running without data");
            return;
        }
        log.info("Using data directory: {}", dir);

        kilpFile = dir.resolve("KILP.DAT");
        if (Files.exists(kilpFile)) {
            try {
                competitors = KilpReader.read(kilpFile);
                log.info("Loaded {} competitors from KILP.DAT", competitors.size());
            } catch (IOException e) {
                log.error("Failed to read KILP.DAT", e);
            }
        } else {
            log.warn("KILP.DAT not found in {}", dir);
        }

        Path cfgFile = dir.resolve("laskenta.cfg");
        if (Files.exists(cfgFile)) {
            try {
                ConfigReader config = new ConfigReader();
                config.read(cfgFile);
                log.info("Config loaded: machine={}, emit={}", config.getMachineId(), config.isEmitEnabled());

                ConfigReader.Connection conn;
                if (connectionIndex > 0) {
                    conn = config.getConnections().stream()
                            .filter(c -> c.index == connectionIndex)
                            .findFirst()
                            .orElse(null);
                    if (conn == null) {
                        log.warn("Configured connection index {} not found in laskenta.cfg, available: {}",
                                connectionIndex, config.getConnections());
                    } else {
                        log.info("Using configured connection: {}", conn);
                    }
                } else {
                    conn = config.getEmitConnection();
                    if (conn != null) {
                        log.info("Using first emit connection: {}", conn);
                    }
                }
                if (conn != null) {
                    int nrec = Files.exists(kilpFile) ? KilpReader.readNumrec(kilpFile) : 0;
                    String machineId = config.getMachineId() != null ? config.getMachineId() : "W1";
                    setupConnection(conn.destAddr, conn.destPort, conn.srvPort, machineId, nrec);
                }
            } catch (Exception e) {
                log.error("Failed to setup UDP connection", e);
            }
        }
    }

    private Path resolveDataDir() {
        if (dataDir != null && !dataDir.isBlank()) {
            Path dir = Path.of(dataDir);
            if (Files.isDirectory(dir)) return dir;
            log.warn("Configured data-dir not found: {}", dataDir);
        }

        // Auto-detect from common paths
        for (String candidate : new String[]{
                "kisat/J1Data", "kisat/HkMaaliData",
                "../kisat/J1Data", "../kisat/HkMaaliData",
                "kisat", "../kisat"
        }) {
            Path p = Path.of(candidate);
            if (Files.isDirectory(p) && Files.exists(p.resolve("KILP.DAT"))) {
                return p;
            }
        }
        return null;
    }

    private void setupConnection(String host, int port, int srvPort, String machineId, int nrec) throws Exception {
        log.info("Setting up UDP connection: host={}, port={}, srvPort={}, machineId={}, nrec={}",
                host, port, srvPort, machineId, nrec);
        connection = new TulospalveluConnection(host, port, machineId, nrec);
        connection.setListener(this);
        eventLoopGroup = new NioEventLoopGroup();

        Bootstrap bootstrap = new Bootstrap();
        bootstrap.group(eventLoopGroup)
                .channel(NioDatagramChannel.class)
                .handler(new ChannelInitializer<NioDatagramChannel>() {
                    @Override
                    protected void initChannel(NioDatagramChannel ch) {
                        log.info("Netty channel initialized: local={}", ch.localAddress());
                        ch.pipeline().addLast(connection);
                    }
                });

        channel = bootstrap.bind(srvPort).sync().channel();
        log.info("UDP channel bound to {}, channel.isActive={}, channel.isOpen={}",
                channel.localAddress(), channel.isActive(), channel.isOpen());

        log.info("Waiting for ALKUT handshake (timeout 5s)...");
        boolean connected = connection.awaitConnected(5, TimeUnit.SECONDS);
        if (connected) {
            log.info("ALKUT handshake OK - connected to tulospalvelu server at {}:{}", host, port);
        } else {
            log.warn("ALKUT handshake timed out after 5s - server at {}:{} did not respond", host, port);
            log.warn("Channel state after timeout: isActive={}, isOpen={}", channel.isActive(), channel.isOpen());
        }
    }

    @PreDestroy
    public void shutdown() {
        log.info("Shutting down TulospalveluService, connected={}", connection != null ? connection.isConnected() : "null");
        if (channel != null) channel.close();
        if (eventLoopGroup != null) eventLoopGroup.shutdownGracefully();
    }

    // --- Public API ---

    public List<KilpReader.Competitor> getCompetitors() {
        return competitors;
    }

    public KilpReader.Competitor getCompetitorByRecordIndex(int recordIndex) {
        for (KilpReader.Competitor c : competitors) {
            if (c.recordIndex == recordIndex) return c;
        }
        return null;
    }

    public boolean isConnected() {
        boolean result = connection != null && connection.isConnected();
        log.debug("isConnected() = {} (connection={}, channel.isActive={}, channel.isOpen={})",
                result,
                connection != null ? connection.isConnected() : "null",
                channel != null ? channel.isActive() : "null",
                channel != null ? channel.isOpen() : "null");
        return result;
    }

    public boolean isActive() {
        return connection != null && connection.isActive();
    }

    public boolean sendCardChange(int recordIndex, int newBadge) {
        log.info("sendCardChange called: recordIndex={}, newBadge={}, connected={}, channelActive={}, channelOpen={}",
                recordIndex, newBadge,
                connection != null ? connection.isConnected() : "null",
                channel != null ? channel.isActive() : "null",
                channel != null ? channel.isOpen() : "null");
        if (connection == null || !connection.isConnected()) {
            log.warn("Cannot send card change - not connected to server (connection={}, isConnected={})",
                    connection != null ? "exists" : "null",
                    connection != null ? connection.isConnected() : "N/A");
            return false;
        }
        if (kilpFile == null) {
            log.warn("Cannot send card change - KILP.DAT not available");
            return false;
        }

        try {
            byte[] pvData = KilpReader.readPvData(kilpFile, recordIndex);
            int kilppvtpsize = KilpReader.getKilppvtpsize();

            CompletableFuture<Boolean> result = connection.sendKilppvt(
                    recordIndex, pvData, kilppvtpsize, newBadge);

            Boolean success = result.get(10, TimeUnit.SECONDS);
            if (Boolean.TRUE.equals(success)) {
                KilpReader.writeBadge(kilpFile, recordIndex, newBadge);
                KilpReader.Competitor comp = getCompetitorByRecordIndex(recordIndex);
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

        KilpReader.Competitor comp = getCompetitorByRecordIndex(dk);
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
