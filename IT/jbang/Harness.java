/**
 * Shared helpers for JBang IT tests against HkMaali (C++ TUI) + webadmin (Java).
 * Included in test scripts via "//SOURCES Harness.java".
 *
 * The KILP.DAT helpers reuse pirila-comm-common's KilpReader so test offsets
 * stay in sync with the production code automatically.
 *
 * HkMaali requires a real PTY; we use pty4j. Webadmin is a normal child process
 * with HTTP polling, and Playwright drives the UI.
 */

import com.microsoft.playwright.*;
import com.microsoft.playwright.options.AriaRole;
import com.microsoft.playwright.options.LoadState;
import com.pty4j.PtyProcess;
import com.pty4j.PtyProcessBuilder;

import fi.pirila.tulospalvelu.Competitor;
import fi.pirila.tulospalvelu.KilpReader;
import fi.pirila.tulospalvelu.TulospalveluProtocol;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.URI;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.*;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

public class Harness {

    public static final Path SCRIPT_DIR = Path.of(System.getProperty("user.dir"))
            .toAbsolutePath();
    public static final Path PROJECT_ROOT = findProjectRoot();
    public static final Path HKMAALI = PROJECT_ROOT.resolve("TPsource/V52/HkMaali");
    public static final Path SOURCE_DATA = PROJECT_ROOT.resolve("kisat/HkKisaWinData");
    /** Lazily-created derivative of SOURCE_DATA with every stage open (no results). */
    public static final Path PRE_RACE_DATA = Path.of("/tmp", "HkKisaWinDataPreRace");
    public static final Path WEBADMIN_DIR = PROJECT_ROOT.resolve("webadmin");
    public static final int SYNC_WAIT_SEC = 4;

    /** Empirically determined for HkKisaWinData/nikondataa demo data. */
    public static final int TABS_TO_EME = 9;

    public static final String KEY_ENTER = "\r";
    public static final String KEY_TAB = "\t";
    public static final String KEY_ESC = "\u001b";
    public static final String KEY_DELETE = "\u001b[3~";

    /**
     * One UDP connection entry for laskenta.cfg.
     * lahemitSuffix=null → "lähemit{n}" (bidirectional).
     * lahemitSuffix="O"  → "lähemit{n}=O" (one-way, leimantarkastus).
     */
    public record Connection(int localPort, String peerHost, int peerPort,
                             String lahemitSuffix) {
        public Connection(int localPort, String peerHost, int peerPort) {
            this(localPort, peerHost, peerPort, null);
        }
    }

    private static Path findProjectRoot() {
        // Walk up looking for the TPsource directory
        Path p = Path.of(System.getProperty("user.dir")).toAbsolutePath();
        while (p != null && p.getParent() != null) {
            if (Files.isDirectory(p.resolve("TPsource"))) return p;
            p = p.getParent();
        }
        // Fallback: assume cwd is IT/jbang and project root is two levels up
        return Path.of(System.getProperty("user.dir")).toAbsolutePath()
                .getParent().getParent();
    }

    // --- HkMaali (PTY-driven C++ TUI) ---

    public static class HkMaali implements AutoCloseable {
        private final Path workdir;
        private PtyProcess proc;
        private OutputStream in;
        private final StringBuilder allOutput = new StringBuilder();
        private Thread pumper;
        private static final Pattern ANSI = Pattern.compile("\\x1b\\[[?0-9;]*[a-zA-Z]");
        private static final Pattern CTRL =
                Pattern.compile("[\\x00-\\x08\\x0b-\\x1f\\x7f]");

        public HkMaali(Path workdir) { this.workdir = workdir; }

        public void start() throws IOException {
            proc = new PtyProcessBuilder()
                    .setCommand(new String[]{HKMAALI.toString()})
                    .setDirectory(workdir.toString())
                    .setRedirectErrorStream(true)
                    .setInitialColumns(80)
                    .setInitialRows(50)
                    .start();
            InputStream out = proc.getInputStream();
            in = proc.getOutputStream();
            // pty4j's InputStream blocks on read; use a daemon pumper so we
            // accumulate output without polling available() (which is unreliable).
            pumper = new Thread(() -> {
                byte[] buf = new byte[4096];
                try {
                    int n;
                    while ((n = out.read(buf)) >= 0) {
                        synchronized (allOutput) {
                            allOutput.append(new String(buf, 0, n,
                                    java.nio.charset.StandardCharsets.UTF_8));
                        }
                    }
                } catch (IOException ignored) {}
            }, "hkmaali-pumper");
            pumper.setDaemon(true);
            pumper.start();
        }

        /** Sleep for timeoutSec; useful as a delay before checking outputText(). */
        public String read(double timeoutSec) {
            try { Thread.sleep((long) (timeoutSec * 1000)); } catch (InterruptedException ignored) {}
            return outputText();
        }

        public void send(String key) throws IOException {
            in.write(key.getBytes(java.nio.charset.StandardCharsets.UTF_8));
            in.flush();
        }

        public String sendRead(String key, double sleepBefore, double readSec)
                throws IOException, InterruptedException {
            send(key);
            Thread.sleep((long) (sleepBefore * 1000));
            return read(readSec);
        }

        /**
         * Click through HkMaali's startup prompts until the main menu appears.
         * Mirrors the Python harness logic (hkmaali_harness.py:accept_and_wait).
         */
        public void acceptAndWait() throws IOException, InterruptedException {
            acceptAndWait(60);
        }

        public void acceptAndWait(int timeoutSec) throws IOException, InterruptedException {
            // Two-stage detection: any sighting of the main-menu marker counts
            // as "we made it", because the daemon pumper keeps appending status-
            // line redraws long after the menu was first reached. Looking only
            // at the tail (Python harness style) loses the marker once enough
            // post-menu refreshes pile up.
            long deadline = System.currentTimeMillis() + timeoutSec * 1000L;
            read(3.0);
            sendRead("\r", 0.5, 2.0);
            int lastLen = 0;
            while (System.currentTimeMillis() < deadline) {
                Thread.sleep(1000);
                read(1.0);
                String text = outputText();
                // Main-menu markers: PÄÄVALIKKO header (some configs) or "M)aali"
                // which only appears when the menu's input prompt is live and
                // ready to accept keys. TIED.SIIRTO and Ilmoitt. show up earlier
                // during status-panel redraws and are unsafe.
                if (text.contains("PÄÄVALIKKO") || text.contains("M)aali")) return;
                String clean = ANSI.matcher(text).replaceAll("");
                clean = CTRL.matcher(clean).replaceAll("");
                String tail = clean.substring(Math.max(0, clean.length() - 500));
                if (tail.contains("J)atka ilman")) {
                    sendRead("J", 0.5, 1.5);
                } else if (tail.contains("L)opeta n") || tail.contains("Lopeta näm")) {
                    sendRead("L", 0.5, 1.5);
                } else if (tail.contains("Paina Enter")) {
                    sendRead("\r", 0.5, 1.5);
                } else if (tail.contains("Vahvista kilpailup")) {
                    sendRead("\r", 0.5, 1.5);
                } else if (tail.contains("H)yväksy valinnat")) {
                    sendRead("H", 0.5, 1.5);
                } else if (text.length() == lastLen) {
                    sendRead("\r", 0.5, 1.5);
                }
                lastLen = text.length();
            }
            throw new RuntimeException("HkMaali did not reach main menu within " + timeoutSec + "s");
        }

        public String outputText() {
            synchronized (allOutput) { return allOutput.toString(); }
        }

        public void clearOutput() {
            synchronized (allOutput) { allOutput.setLength(0); }
        }

        public boolean hasStartupErrors() {
            String t = outputText();
            return t.contains("yhteensopivia") || t.contains("DATA_ERR");
        }

        public void writeLog(Path target) {
            try { Files.writeString(target, outputText()); }
            catch (IOException ignored) {}
        }

        // --- Korjaukset / Korjaa flow (mirrors Python harness) ---

        public void navigateToKorjaa(String competitor) throws IOException, InterruptedException {
            sendRead("K", 0.5, 0.5);
            sendRead("K", 0.5, 0.5);
            sendRead(competitor + KEY_ENTER, 0.5, 2.0);
        }

        public void escapeToMain() throws IOException, InterruptedException {
            for (int i = 0; i < 3; i++) sendRead(KEY_ESC, 0.3, 0.5);
        }

        /**
         * On a competitor's edit page: TAB to the EME (emit) field, delete the
         * old value, type the new one, accept with '+'. Tab count and delete
         * count match the Python harness.
         */
        public void changeEmit(String newValue) throws IOException, InterruptedException {
            for (int i = 0; i < TABS_TO_EME; i++) {
                send(KEY_TAB);
                Thread.sleep(50);
                Thread.sleep(300);
                read(0.5);
            }
            Thread.sleep(500);
            read(0.5);

            for (int i = 0; i < 8; i++) {
                send(KEY_DELETE);
                Thread.sleep(100);
                read(0.1);
            }
            Thread.sleep(300);

            for (char ch : newValue.toCharArray()) {
                send(String.valueOf(ch));
                Thread.sleep(100);
                read(0.1);
            }
            Thread.sleep(300);

            sendRead("+", 0.5, 2.0);
        }

        /** Navigate to the competitor, return all output captured during the navigation, then escape back. */
        public String readCompetitorEmit(String competitor) throws IOException, InterruptedException {
            clearOutput();
            navigateToKorjaa(competitor);
            String text = outputText();
            escapeToMain();
            return text;
        }

        @Override
        public void close() {
            if (proc != null) {
                try { proc.destroyForcibly(); } catch (Exception ignored) {}
                try { proc.waitFor(2, TimeUnit.SECONDS); } catch (Exception ignored) {}
                proc = null;
            }
        }
    }

    // --- Webadmin (Spring Boot jar + Playwright UI) ---

    public static class Webadmin implements AutoCloseable {
        private final Path dataDir;
        private final int httpPort;
        private Process proc;

        public Webadmin(Path dataDir, int httpPort) {
            this.dataDir = dataDir;
            this.httpPort = httpPort;
        }

        public void start() throws Exception {
            Path jar = findWebadminJar();
            if (jar == null) {
                throw new IOException("webadmin jar not found in " + WEBADMIN_DIR.resolve("target")
                        + " — run 'mvn package -DskipTests' there first");
            }
            ProcessBuilder pb = new ProcessBuilder(
                    "java", "-jar", jar.toString(),
                    "--tulospalvelu.data-dir=" + dataDir,
                    "--tulospalvelu.auto-start=true",
                    "--server.port=" + httpPort);
            pb.redirectErrorStream(true);
            pb.redirectOutput(new File("/tmp/webadmin-" + httpPort + ".log"));
            proc = pb.start();

            long deadline = System.currentTimeMillis() + 30_000;
            while (System.currentTimeMillis() < deadline) {
                try {
                    HttpURLConnection c = (HttpURLConnection) URI
                            .create("http://localhost:" + httpPort + "/").toURL().openConnection();
                    c.setConnectTimeout(2000);
                    c.setReadTimeout(2000);
                    if (c.getResponseCode() == 200) return;
                } catch (Exception ignored) {}
                Thread.sleep(1000);
            }
            throw new IOException("webadmin did not respond on port " + httpPort);
        }

        private static Path findWebadminJar() throws IOException {
            Path target = WEBADMIN_DIR.resolve("target");
            if (!Files.isDirectory(target)) return null;
            try (var stream = Files.list(target)) {
                return stream
                        .filter(p -> {
                            String n = p.getFileName().toString();
                            return n.startsWith("webadmin-") && n.endsWith(".jar");
                        })
                        .sorted(Comparator.reverseOrder())
                        .findFirst()
                        .orElse(null);
            }
        }

        private void openCardChange(Page page) throws InterruptedException {
            page.navigate("http://localhost:" + httpPort + "/",
                    new Page.NavigateOptions().setTimeout(15000));
            page.waitForLoadState(LoadState.NETWORKIDLE,
                    new Page.WaitForLoadStateOptions().setTimeout(10000));
            page.getByText("Card Change").click();
            Thread.sleep(2000);
            page.waitForLoadState(LoadState.NETWORKIDLE,
                    new Page.WaitForLoadStateOptions().setTimeout(10000));
        }

        /** Drive the Card Change view via Playwright. Returns true on submit. */
        public boolean changeEmit(String competitor, String newBadge) {
            try (Playwright pw = Playwright.create()) {
                try (Browser browser = pw.chromium().launch(
                        new BrowserType.LaunchOptions().setHeadless(true))) {
                    Page page = browser.newPage();
                    openCardChange(page);
                    page.locator("vaadin-text-field").nth(0).locator("input").fill(competitor);
                    Thread.sleep(1500);
                    page.locator("vaadin-text-field").nth(1).locator("input").fill(newBadge);
                    Thread.sleep(500);
                    page.getByRole(AriaRole.BUTTON,
                            new Page.GetByRoleOptions().setName("Vaihda kortti")).click();
                    Thread.sleep(3000);
                    return true;
                }
            } catch (Exception e) {
                System.err.println("Playwright error: " + e);
                return false;
            }
        }

        /**
         * Edit a competitor through the Competitor List view's master-detail form.
         * Toggles "Näytä vakantit" so vakant entries are reachable, searches by
         * kilpno, picks the row, fills the form, hits Tallenna.
         *
         * sarjaLabel: the class label (e.g. "H21A") to pick from the sarja dropdown.
         * cardNumber: pass null/blank to leave card unchanged.
         */
        public boolean editCompetitor(String kilpno, String etunimi, String sukunimi,
                                       String seura, String sarjaLabel, String cardNumber) {
            try (Playwright pw = Playwright.create()) {
                try (Browser browser = pw.chromium().launch(
                        new BrowserType.LaunchOptions().setHeadless(true))) {
                    Page page = browser.newPage();
                    page.navigate("http://localhost:" + httpPort + "/",
                            new Page.NavigateOptions().setTimeout(15000));
                    page.waitForLoadState(LoadState.NETWORKIDLE,
                            new Page.WaitForLoadStateOptions().setTimeout(10000));
                    page.getByText("Competitor List").click();
                    Thread.sleep(2000);
                    page.waitForLoadState(LoadState.NETWORKIDLE,
                            new Page.WaitForLoadStateOptions().setTimeout(10000));

                    // Reveal vakantit
                    page.getByLabel("Näytä vakantit").check();
                    Thread.sleep(500);

                    // Filter by kilpno
                    page.locator("vaadin-text-field input").first().fill(kilpno);
                    Thread.sleep(1500);

                    // Click the matching row in the grid
                    page.locator("vaadin-grid-cell-content").getByText(kilpno).first()
                            .click(new com.microsoft.playwright.Locator.ClickOptions().setForce(true));
                    Thread.sleep(1500);

                    // Fill the form fields. The detail panel renders text-fields
                    // for etunimi, sukunimi, seura, cardNumber AND a combobox for sarja.
                    // Order in the form: etunimi(0), sukunimi(1), seura(2), cardNumber(3)
                    // — but the search field at the top is also a vaadin-text-field, so
                    // we pick the form's fields by label.
                    // Field labels collide with grid column headers ("Seura" appears
                    // both as a vaadin-grid-sorter and a form field) — use ARIA TEXTBOX
                    // role to pick only inputs.
                    page.getByRole(AriaRole.TEXTBOX,
                            new Page.GetByRoleOptions().setName("Etunimi")).fill(etunimi);
                    Thread.sleep(200);
                    page.getByRole(AriaRole.TEXTBOX,
                            new Page.GetByRoleOptions().setName("Sukunimi")).fill(sukunimi);
                    Thread.sleep(200);
                    page.getByRole(AriaRole.TEXTBOX,
                            new Page.GetByRoleOptions().setName("Seura")).fill(seura);
                    Thread.sleep(200);

                    // Sarja combobox — type the label then pick the matching option.
                    var sarjaInput = page.getByRole(AriaRole.COMBOBOX,
                            new Page.GetByRoleOptions().setName("Sarja"));
                    sarjaInput.click();
                    Thread.sleep(300);
                    sarjaInput.fill(sarjaLabel);
                    Thread.sleep(800);
                    page.getByRole(AriaRole.OPTION,
                            new Page.GetByRoleOptions().setName(sarjaLabel)).first().click();
                    Thread.sleep(300);

                    if (cardNumber != null && !cardNumber.isBlank()) {
                        page.getByRole(AriaRole.TEXTBOX,
                                new Page.GetByRoleOptions().setName("Kilpailukortti")).fill(cardNumber);
                        Thread.sleep(200);
                    }

                    page.getByRole(AriaRole.BUTTON,
                            new Page.GetByRoleOptions().setName("Tallenna")).click();
                    Thread.sleep(3000);
                    return true;
                }
            } catch (Exception e) {
                System.err.println("Playwright error: " + e);
                return false;
            }
        }

        /** Open Card Change with the competitor selected, check whether expectedBadge is visible. */
        public boolean checkEmit(String competitor, String expectedBadge) {
            try (Playwright pw = Playwright.create()) {
                try (Browser browser = pw.chromium().launch(
                        new BrowserType.LaunchOptions().setHeadless(true))) {
                    Page page = browser.newPage();
                    openCardChange(page);
                    page.locator("vaadin-text-field").nth(0).locator("input").fill(competitor);
                    Thread.sleep(2000);
                    String body = page.locator("body").innerText();
                    return body.contains(expectedBadge);
                }
            } catch (Exception e) {
                System.err.println("Playwright error: " + e);
                return false;
            }
        }

        @Override
        public void close() {
            if (proc != null) {
                proc.destroy();
                try { proc.waitFor(5, TimeUnit.SECONDS); } catch (Exception ignored) {}
                if (proc.isAlive()) proc.destroyForcibly();
                proc = null;
            }
        }
    }

    // --- Test data directory setup (mirrors setup_data_dir in Python harness) ---

    /** Single-connection convenience wrapper (same default source data). */
    public static Path setupDataDir(String name, String kone, int localPort,
                                    String peerHost, int peerPort) throws IOException {
        return setupDataDir(name, kone, null, SOURCE_DATA,
                new Connection(localPort, peerHost, peerPort));
    }

    /** Multi-connection setup using the default HkKisaWinData source. */
    public static Path setupDataDir(String name, String kone, Connection... conns)
            throws IOException {
        return setupDataDir(name, kone, null, SOURCE_DATA, conns);
    }

    /** Full setup. paiva=null skips the PÄIVÄ line; sourceData=null falls back to HkKisaWinData. */
    public static Path setupDataDir(String name, String kone, Integer paiva,
                                    Path sourceData, Connection... conns) throws IOException {
        Path base = SCRIPT_DIR.resolve("test_data_" + name);
        Path src = sourceData != null ? sourceData : SOURCE_DATA;
        if (Files.exists(base)) deleteRecursive(base);
        Files.createDirectories(base);

        Files.copy(src.resolve("KILP.DAT"), base.resolve("KILP.DAT"));
        Files.copy(src.resolve("KilpSrj.xml"), base.resolve("KilpSrj.xml"));
        Path radat = src.resolve("radat1.xml");
        if (Files.exists(radat)) Files.copy(radat, base.resolve("radat1.xml"));

        StringBuilder cfg = new StringBuilder();
        cfg.append("Kone=").append(kone).append('\n');
        cfg.append("Emit\n");
        if (paiva != null) cfg.append("PÄIVÄ=").append(paiva).append('\n');
        for (int i = 0; i < conns.length; i++) {
            Connection c = conns[i];
            cfg.append("yhteys").append(i + 1).append("=udp:")
                    .append(c.localPort()).append('/')
                    .append(c.peerHost()).append(':').append(c.peerPort()).append('\n');
            cfg.append("lähemit").append(i + 1);
            if (c.lahemitSuffix() != null) cfg.append('=').append(c.lahemitSuffix());
            cfg.append('\n');
        }
        Files.writeString(base.resolve("laskenta.cfg"), cfg.toString());
        return base;
    }

    /**
     * Lazily build (and cache) a derivative of SOURCE_DATA where every stage of every
     * competitor is open: keskhyl='-', vatp[1].time=0, vatp[1].val2=0. Use this as the
     * source for tests that simulate the realistic mid-race emit-change scenario where
     * the competitor has not yet finished. Default SOURCE_DATA has all-decided demo
     * results which would (correctly) make webadmin's auto-detect skip pv[0].
     */
    public static Path preRaceSourceData() throws IOException {
        if (!Files.exists(PRE_RACE_DATA.resolve("KILP.DAT"))) {
            Files.createDirectories(PRE_RACE_DATA);
            for (String f : new String[]{"KILP.DAT", "KilpSrj.xml", "radat1.xml", "seurat.csv"}) {
                Path src = SOURCE_DATA.resolve(f);
                if (Files.exists(src)) {
                    Files.copy(src, PRE_RACE_DATA.resolve(f),
                            StandardCopyOption.REPLACE_EXISTING);
                }
            }
            // Clear every stage of every competitor.
            Path kilp = PRE_RACE_DATA.resolve("KILP.DAT");
            int reclen = KilpReader.detectRecordSize(kilp);
            int npv = KilpReader.getNpv();
            int numrec = (int) (Files.size(kilp) / reclen);
            for (int i = 1; i < numrec; i++) {
                for (int pv = 0; pv < npv; pv++) clearPvStatus(kilp, i, pv);
            }
        }
        return PRE_RACE_DATA;
    }

    /** Escape hatch for tests that need a fully custom laskenta.cfg (e.g. yhteys3 without yhteys2). */
    public static Path setupDataDirRaw(String name, String laskentaCfg, Path sourceData)
            throws IOException {
        Path base = SCRIPT_DIR.resolve("test_data_" + name);
        Path src = sourceData != null ? sourceData : SOURCE_DATA;
        if (Files.exists(base)) deleteRecursive(base);
        Files.createDirectories(base);
        Files.copy(src.resolve("KILP.DAT"), base.resolve("KILP.DAT"));
        Files.copy(src.resolve("KilpSrj.xml"), base.resolve("KilpSrj.xml"));
        Path radat = src.resolve("radat1.xml");
        if (Files.exists(radat)) Files.copy(radat, base.resolve("radat1.xml"));
        Path seurat = src.resolve("seurat.csv");
        if (Files.exists(seurat)) Files.copy(seurat, base.resolve("seurat.csv"));
        Files.writeString(base.resolve("laskenta.cfg"), laskentaCfg);
        return base;
    }

    public static void deleteRecursive(Path p) throws IOException {
        if (!Files.exists(p)) return;
        Files.walkFileTree(p, new SimpleFileVisitorImpl());
    }

    private static final class SimpleFileVisitorImpl
            extends java.nio.file.SimpleFileVisitor<Path> {
        @Override public java.nio.file.FileVisitResult visitFile(
                Path file, BasicFileAttributes attrs) throws IOException {
            Files.delete(file); return java.nio.file.FileVisitResult.CONTINUE;
        }
        @Override public java.nio.file.FileVisitResult postVisitDirectory(
                Path dir, IOException exc) throws IOException {
            Files.delete(dir); return java.nio.file.FileVisitResult.CONTINUE;
        }
    }

    // --- KILP.DAT helpers (reuse pirila-comm-common where possible) ---

    public static int findRecordByKilpno(Path kilpFile, int kilpno) throws IOException {
        for (Competitor c : KilpReader.read(kilpFile)) {
            if (c.kilpno == kilpno) return c.recordIndex;
        }
        return -1;
    }

    public static int readPvBadge(Path kilpFile, int recordIndex, int pvIndex)
            throws IOException {
        byte[] pv = KilpReader.readPvData(kilpFile, recordIndex, pvIndex);
        return TulospalveluProtocol.readInt32LE(pv, TulospalveluProtocol.PV_OFF_BADGE);
    }

    public static KilpReader.StageStatus readPvStatus(Path kilpFile, int recordIndex, int pvIndex)
            throws IOException {
        return KilpReader.readStageStatus(kilpFile, recordIndex, pvIndex);
    }

    /** Zero out keskhyl + vatp[1].time + vatp[1].val2 so a stage counts as 'open'. */
    public static void clearPvStatus(Path kilpFile, int recordIndex, int pvIndex)
            throws IOException {
        // Layout values come from KilpReader's static state, which is set by
        // detectRecordSize. Force a fresh detect by reading the layout once.
        KilpReader.read(kilpFile);
        int reclen = KilpReader.detectRecordSize(kilpFile);
        int kilppvtpsize = KilpReader.getKilppvtpsize();
        long pvBase = (long) recordIndex * reclen + 360 + (long) pvIndex * kilppvtpsize;
        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "rw")) {
            raf.seek(pvBase + TulospalveluProtocol.PV_OFF_KESKHYL);
            raf.write(new byte[]{0, 0});
            // vatp[1] (finish slot) at +152+8: zero time(4) + ysija(4) = 8 bytes
            raf.seek(pvBase + 152 + 8);
            raf.write(new byte[8]);
        }
    }

    /** Mark a stage as decided (StageStatus.hasResult() == true). */
    public static void setPvResult(Path kilpFile, int recordIndex, int pvIndex,
                                    char keskhyl, int finishTimeMs, int ysija) throws IOException {
        KilpReader.read(kilpFile);
        int reclen = KilpReader.detectRecordSize(kilpFile);
        int kilppvtpsize = KilpReader.getKilppvtpsize();
        long pvBase = (long) recordIndex * reclen + 360 + (long) pvIndex * kilppvtpsize;
        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "rw")) {
            raf.seek(pvBase + TulospalveluProtocol.PV_OFF_KESKHYL);
            raf.write(new byte[]{(byte) (keskhyl & 0xFF), (byte) ((keskhyl >> 8) & 0xFF)});
            raf.seek(pvBase + 152 + 8);
            byte[] eight = new byte[8];
            TulospalveluProtocol.writeInt32LE(eight, 0, finishTimeMs);
            TulospalveluProtocol.writeInt32LE(eight, 4, ysija);
            raf.write(eight);
        }
    }

    // --- Misc ---

    public static void sleep(long ms) {
        try { Thread.sleep(ms); } catch (InterruptedException ignored) {}
    }

    public static String tag(boolean ok) { return ok ? "✓" : "FAIL"; }
}
