package in.virit.pirila.service;

import fi.pirila.tulospalvelu.ConfigReader;
import fi.pirila.tulospalvelu.KilpReader;
import fi.pirila.tulospalvelu.KilpSrjReader;
import fi.pirila.tulospalvelu.MessageListener;
import fi.pirila.tulospalvelu.TulospalveluConnection;
import fi.pirila.tulospalvelu.TulospalveluProtocol;
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
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

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
    private final List<Consumer<fi.pirila.tulospalvelu.Competitor>> updateListeners = new CopyOnWriteArrayList<>();
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
        boolean passive = host == null || host.isBlank() || "AUTO".equalsIgnoreCase(host);
        log.info("Setting up UDP connection: host={}, port={}, srvPort={}, machineId={}, nrec={}, passive={}",
                host, port, srvPort, machineId, nrec, passive);

        // Probe if srvPort is available; handler is not @Sharable so we can't
        // try/retry binding the same instance. Check with a plain DatagramSocket.
        int actualSrvPort = srvPort;
        try (java.net.DatagramSocket probe = new java.net.DatagramSocket(srvPort)) {
            // srvPort available
        } catch (java.net.BindException be) {
            if (passive) {
                throw new RuntimeException("Passiivi-yhteys vaatii kiinteän srvPortin " + srvPort
                        + ", mutta se on käytössä: " + be.getMessage(), be);
            }
            log.warn("srvPort {} not available ({}), will use ephemeral port", srvPort, be.getMessage());
            actualSrvPort = 0;
        }

        udpConnection = passive
                ? TulospalveluConnection.passive(machineId, nrec)
                : new TulospalveluConnection(host, port, machineId, nrec);
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
        channel = bootstrap.bind(actualSrvPort).sync().channel();
        log.info("UDP channel bound to {}, channel.isActive={}, channel.isOpen={}",
                channel.localAddress(), channel.isActive(), channel.isOpen());

        if (passive) {
            log.info("Passive UDP listener on port {} - waiting for peer to initiate", srvPort);
            return;
        }

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

    public boolean isVacantClass(int sarja) {
        return kilpSrjReader != null && kilpSrjReader.isVacantClass(sarja);
    }

    /** sarja (0-based) → class name, in xml order. Empty if KilpSrj.xml wasn't loaded. */
    public java.util.Map<Integer, String> getAllClasses() {
        return kilpSrjReader != null ? kilpSrjReader.getAllClasses() : java.util.Map.of();
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

    private static final int MAX_RETRIES = 10;
    private static final long RETRY_DELAY_MS = 2000;

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
            int npv = Math.max(1, KilpReader.getNpv());
            int kilppvtpsize = KilpReader.getKilppvtpsize();

            // Mirror C++ HkConsole/HkKilp.cpp:970-973 forward-only propagation, with the
            // "current stage" auto-detected: change applies from the first stage that has
            // not yet been finalised (no result yet, no DNF/DSQ/DNS mark). If every stage
            // is already decided we still update the last one so the user's input isn't
            // silently dropped.
            int startStage = npv - 1;
            for (int i = 0; i < npv; i++) {
                KilpReader.StageStatus s = KilpReader.readStageStatus(kilpFile, recordIndex, i);
                if (!s.hasResult()) {
                    startStage = i;
                    break;
                }
            }
            log.info("Card change record={} newBadge={}: npv={}, startStage={}",
                    recordIndex, newBadge, npv, startStage);

            for (int stage = startStage; stage < npv; stage++) {
                if (!sendCardChangeStage(recordIndex, stage, newBadge, kilppvtpsize)) {
                    log.warn("Card change failed at stage {} (stages [{}..{}) updated, [{}..{}) not)",
                            stage, startStage, stage, stage, npv);
                    return false;
                }
            }
            return true;
        } catch (Exception e) {
            log.error("Card change failed for record={}", recordIndex, e);
            return false;
        }
    }

    private boolean sendCardChangeStage(int recordIndex, int pvIndex, int newBadge, int kilppvtpsize)
            throws Exception {
        byte[] pvData = KilpReader.readPvData(kilpFile, recordIndex, pvIndex);

        for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
            CompletableFuture<Boolean> result;
            if (tcpConnection != null) {
                result = tcpConnection.sendKilppvt(recordIndex, pvIndex, pvData, kilppvtpsize, newBadge);
            } else {
                result = udpConnection.sendKilppvt(recordIndex, pvIndex, pvData, kilppvtpsize, newBadge);
            }

            Boolean success = result.get(10, TimeUnit.SECONDS);
            if (Boolean.TRUE.equals(success)) {
                KilpReader.writeBadge(kilpFile, recordIndex, pvIndex, newBadge);
                if (pvIndex == 0) {
                    fi.pirila.tulospalvelu.Competitor comp = getCompetitorByRecordIndex(recordIndex);
                    if (comp != null) comp.badge = newBadge;
                }
                log.info("Card change OK: record={}, pv={}, newBadge={}, attempt={}",
                        recordIndex, pvIndex, newBadge, attempt);
                return true;
            }

            if (attempt < MAX_RETRIES) {
                log.info("Card change NAK'd (record={} pv={} attempt {}/{}), retrying in {}ms...",
                        recordIndex, pvIndex, attempt, MAX_RETRIES, RETRY_DELAY_MS);
                Thread.sleep(RETRY_DELAY_MS);
            } else {
                log.warn("Card change for stage {} rejected after {} attempts: record={}",
                        pvIndex, MAX_RETRIES, recordIndex);
            }
        }
        return false;
    }

    /**
     * Update an existing competitor's record-level fields (sukunimi, etunimi, seura, sarja).
     * Sends a single KILPT message and updates the local KILP.DAT on success.
     * Badge changes are not handled here — call sendCardChange separately.
     */
    public boolean sendCompetitorEdit(int recordIndex, String sukunimi, String etunimi,
                                       String seura, int sarja) {
        log.info("sendCompetitorEdit: record={} sukunimi={} etunimi={} seura={} sarja={}",
                recordIndex, sukunimi, etunimi, seura, sarja);
        if (!isConnected()) {
            log.warn("Cannot send competitor edit - not connected");
            return false;
        }
        if (kilpFile == null) {
            log.warn("Cannot send competitor edit - KILP.DAT not available");
            return false;
        }
        try {
            byte[] record = KilpReader.readFullRecord(kilpFile, recordIndex);
            int kilprecsize0 = record.length;

            // Field offsets (kilp_fields, default sizes from HkDat.cpp:94-119)
            int OFF_SUKUNIMI = 48, LEN_SUKUNIMI = 25;
            int OFF_ETUNIMI = 98, LEN_ETUNIMI = 25;
            int OFF_SEURA = 180, LEN_SEURA = 32;
            int OFF_SARJA = 348;
            int OFF_KILPNO = 2;
            int kilpno = (record[OFF_KILPNO] & 0xFF) | ((record[OFF_KILPNO + 1] & 0xFF) << 8);

            TulospalveluProtocol.writeWideString(record, OFF_SUKUNIMI, LEN_SUKUNIMI, sukunimi);
            TulospalveluProtocol.writeWideString(record, OFF_ETUNIMI, LEN_ETUNIMI, etunimi);
            TulospalveluProtocol.writeWideString(record, OFF_SEURA, LEN_SEURA, seura);
            TulospalveluProtocol.writeInt16LE(record, OFF_SARJA, (short) sarja);

            for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
                CompletableFuture<Boolean> result;
                if (tcpConnection != null) {
                    result = tcpConnection.sendKilpt(recordIndex, kilpno, record, kilprecsize0);
                } else {
                    result = udpConnection.sendKilpt(recordIndex, kilpno, record, kilprecsize0);
                }
                Boolean success = result.get(10, TimeUnit.SECONDS);
                if (Boolean.TRUE.equals(success)) {
                    KilpReader.writeFullRecord(kilpFile, recordIndex, record);
                    fi.pirila.tulospalvelu.Competitor comp = getCompetitorByRecordIndex(recordIndex);
                    if (comp != null) {
                        comp.sukunimi = sukunimi;
                        comp.etunimi = etunimi;
                        comp.seura = seura;
                        comp.sarja = sarja;
                        for (var l : updateListeners) {
                            try { l.accept(comp); } catch (Exception e) { log.warn("Update listener failed", e); }
                        }
                    }
                    log.info("Competitor edit OK: record={}, attempt={}", recordIndex, attempt);
                    return true;
                }
                if (attempt < MAX_RETRIES) {
                    log.info("KILPT NAK'd (attempt {}/{}), retrying in {}ms...",
                            attempt, MAX_RETRIES, RETRY_DELAY_MS);
                    Thread.sleep(RETRY_DELAY_MS);
                } else {
                    log.warn("KILPT rejected after {} attempts: record={}", MAX_RETRIES, recordIndex);
                }
            }
            return false;
        } catch (Exception e) {
            log.error("Competitor edit failed for record={}", recordIndex, e);
            return false;
        }
    }

    public boolean sendStatusChange(int recordIndex, char newStatus) {
        log.info("sendStatusChange called: recordIndex={}, newStatus='{}', connected={}",
                recordIndex, newStatus, isConnected());
        if (!isConnected()) {
            log.warn("Cannot send status change - not connected to server");
            return false;
        }
        if (kilpFile == null) {
            log.warn("Cannot send status change - KILP.DAT not available");
            return false;
        }

        try {
            byte[] pvData = KilpReader.readPvData(kilpFile, recordIndex);
            int kilppvtpsize = KilpReader.getKilppvtpsize();

            for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
                CompletableFuture<Boolean> result;
                if (tcpConnection != null) {
                    result = tcpConnection.sendStatusChange(recordIndex, pvData, kilppvtpsize, newStatus);
                } else {
                    result = udpConnection.sendStatusChange(recordIndex, pvData, kilppvtpsize, newStatus);
                }

                Boolean success = result.get(10, TimeUnit.SECONDS);
                if (Boolean.TRUE.equals(success)) {
                    KilpReader.writeKeskhyl(kilpFile, recordIndex, newStatus);
                    fi.pirila.tulospalvelu.Competitor comp = getCompetitorByRecordIndex(recordIndex);
                    if (comp != null) {
                        comp.keskhyl = newStatus;
                        for (var l : updateListeners) {
                            try { l.accept(comp); } catch (Exception e) { log.warn("Update listener failed", e); }
                        }
                    }
                    log.info("Status change successful: record={}, newStatus='{}', attempt={}",
                            recordIndex, newStatus, attempt);
                    return true;
                }

                if (attempt < MAX_RETRIES) {
                    log.info("Status change NAK'd (attempt {}/{}), retrying in {}ms...", attempt, MAX_RETRIES, RETRY_DELAY_MS);
                    Thread.sleep(RETRY_DELAY_MS);
                } else {
                    log.warn("Status change rejected after {} attempts: record={}", MAX_RETRIES, recordIndex);
                }
            }
            return false;
        } catch (Exception e) {
            log.error("Status change failed for record={}", recordIndex, e);
            return false;
        }
    }

    /**
     * Register a listener for competitor updates from the network.
     * Called on Netty thread — listener must handle UI.access() itself.
     */
    public void addUpdateListener(Consumer<fi.pirila.tulospalvelu.Competitor> listener) {
        updateListeners.add(listener);
    }

    public void removeUpdateListener(Consumer<fi.pirila.tulospalvelu.Competitor> listener) {
        updateListeners.remove(listener);
    }

    // --- Server-initiated updates ---

    @Override
    public void onFullCompetitorRecord(int dk, int entno, byte[] recordData) {
        fi.pirila.tulospalvelu.Competitor comp = getCompetitorByRecordIndex(dk);
        if (comp == null) {
            log.warn("KILPT for unknown record dk={}, entno={}", dk, entno);
            return;
        }
        try {
            KilpReader.ParsedRecord r = KilpReader.parseRecord(recordData);
            comp.kilpno = r.kilpno();
            comp.sukunimi = r.sukunimi();
            comp.etunimi = r.etunimi();
            comp.seura = r.seura();
            comp.sarja = r.sarja();
            if (kilpFile != null) {
                KilpReader.writeFullRecord(kilpFile, dk, recordData);
            }
            log.info("Server updated full record: dk={}, {} {} ({})", dk, comp.sukunimi, comp.etunimi, comp.seura);
            for (var l : updateListeners) {
                try { l.accept(comp); } catch (Exception e) { log.warn("Update listener failed", e); }
            }
        } catch (Exception e) {
            log.warn("Failed to apply KILPT for dk={}: {}", dk, e.getMessage());
        }
    }

    @Override
    public void onTimeResult(int dk, int bib, int stage, int splitIndex, int time) {
        // VAIN_TULOST carries one timestamp at a time. The Windows side fires it
        // both when a result lands and when an existing result is cleared (time=0).
        // splitIndex semantics: -1 = start time (tlahto), 0 = finish (vatp[1].time),
        //                       >0 = intermediate split.
        if (kilpFile != null) {
            try {
                KilpReader.writeTimeResult(kilpFile, dk, stage, splitIndex, time);
            } catch (Exception e) {
                log.warn("Failed to write VAIN_TULOST to local KILP.DAT: {}", e.getMessage());
            }
        }
        // Only the finish (splitIndex=0) on the active stage is reflected in our
        // Competitor model — intermediate splits and start times don't affect the
        // grid's "Tulos" column.
        if (splitIndex == 0) {
            fi.pirila.tulospalvelu.Competitor comp = getCompetitorByRecordIndex(dk);
            if (comp != null) {
                comp.finishTime = time;
                if (time == 0) comp.ysija = 0;  // result cleared → drop placement too
                log.info("Server time result: dk={} bib={} stage={} time={}{}",
                        dk, bib, stage, time, time == 0 ? " (cleared)" : "");
                for (var l : updateListeners) {
                    try { l.accept(comp); } catch (Exception e) { log.warn("Update listener failed", e); }
                }
            }
        } else {
            log.debug("VAIN_TULOST split (not finish): dk={} stage={} split={} time={}",
                    dk, stage, splitIndex, time);
        }
    }

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
            for (var l : updateListeners) {
                try { l.accept(comp); } catch (Exception e) { log.warn("Update listener failed", e); }
            }
        }
    }
}
