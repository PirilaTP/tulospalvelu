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
    public static final Path WEBADMIN_DIR = PROJECT_ROOT.resolve("webadmin");
    public static final int SYNC_WAIT_SEC = 4;

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
            // Two-stage detection: any sighting of the main-menu marker counts
            // as "we made it", because the daemon pumper keeps appending status-
            // line redraws long after the menu was first reached. Looking only
            // at the tail (Python harness style) loses the marker once enough
            // post-menu refreshes pile up.
            long deadline = System.currentTimeMillis() + 25_000;
            read(3.0);
            sendRead("\r", 0.5, 2.0);
            int lastLen = 0;
            while (System.currentTimeMillis() < deadline) {
                Thread.sleep(1000);
                read(1.0);
                String text = outputText();
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
            throw new RuntimeException("HkMaali did not reach main menu within 25s");
        }

        public String outputText() {
            synchronized (allOutput) { return allOutput.toString(); }
        }

        public void writeLog(Path target) {
            try { Files.writeString(target, outputText()); }
            catch (IOException ignored) {}
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

        /** Drive the Card Change view via Playwright. Returns true on submit. */
        public boolean changeEmit(String competitor, String newBadge) {
            try (Playwright pw = Playwright.create()) {
                try (Browser browser = pw.chromium().launch(
                        new BrowserType.LaunchOptions().setHeadless(true))) {
                    Page page = browser.newPage();
                    page.navigate("http://localhost:" + httpPort + "/",
                            new Page.NavigateOptions().setTimeout(15000));
                    page.waitForLoadState(LoadState.NETWORKIDLE,
                            new Page.WaitForLoadStateOptions().setTimeout(10000));
                    page.getByText("Card Change").click();
                    Thread.sleep(2000);
                    page.waitForLoadState(LoadState.NETWORKIDLE,
                            new Page.WaitForLoadStateOptions().setTimeout(10000));
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

    public static Path setupDataDir(String name, String kone, int localPort,
                                    String peerHost, int peerPort) throws IOException {
        Path base = SCRIPT_DIR.resolve("test_data_" + name);
        if (Files.exists(base)) deleteRecursive(base);
        Files.createDirectories(base);

        Files.copy(SOURCE_DATA.resolve("KILP.DAT"), base.resolve("KILP.DAT"));
        Files.copy(SOURCE_DATA.resolve("KilpSrj.xml"), base.resolve("KilpSrj.xml"));
        Path radat = SOURCE_DATA.resolve("radat1.xml");
        if (Files.exists(radat)) Files.copy(radat, base.resolve("radat1.xml"));

        String cfg = "Kone=" + kone + "\n"
                   + "Emit\n"
                   + "yhteys1=udp:" + localPort + "/" + peerHost + ":" + peerPort + "\n"
                   + "lähemit1\n";
        Files.writeString(base.resolve("laskenta.cfg"), cfg);
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
