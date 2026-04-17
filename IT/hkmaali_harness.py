"""
Shared test harness for HkMaali / webadmin integration tests.

Provides:
  - HkMaaliSession: PTY-based C++ HkMaali process control
  - WebadminSession: Java webadmin process + Playwright helpers
  - setup_data_dir(): create a test data directory with KILP.DAT etc.
  - Paths and constants
"""

import pty, os, select, time, struct, fcntl, termios, shutil, subprocess, signal

# --- Paths ---

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.join(SCRIPT_DIR, '..')
HKMAALI = os.path.join(PROJECT_ROOT, 'TPsource', 'V52', 'HkMaali')
SOURCE_DATA = os.path.join(PROJECT_ROOT, 'kisat', 'HkKisaWinData')
WEBADMIN_DIR = os.path.join(PROJECT_ROOT, 'webadmin')
WEBADMIN_JAR = os.path.join(WEBADMIN_DIR, 'target', 'webadmin-1.0-SNAPSHOT.jar')

# --- Keys ---

KEY_TAB = '\t'
KEY_ENTER = '\r'
KEY_ESC = '\x1b'
KEY_DELETE = '\x1b[3~'
KEY_HOME = '\x1b[H'
KEY_INSERT = '\x1b[2~'

# --- Constants ---

TABS_TO_EME = 9   # empirically determined for HkKisaWinData demo data
SYNC_WAIT = 4     # seconds to wait for UDP sync


# --- Data directory setup ---

def setup_data_dir(name, kone, connections=None, base_dir=None):
    """Create a test data directory with KILP.DAT, KilpSrj.xml, laskenta.cfg.

    Args:
        name: directory suffix (e.g. 'MA', 'WI')
        kone: machine ID for laskenta.cfg (e.g. 'MA', 'WI', 'WB')
        connections: list of (local_port, peer_addr, peer_port) tuples, or None for no network
        base_dir: parent directory (default: SCRIPT_DIR)
    Returns:
        path to created directory
    """
    base = base_dir or SCRIPT_DIR
    instdir = os.path.join(base, f'test_data_{name}')
    if os.path.exists(instdir):
        shutil.rmtree(instdir)
    os.makedirs(instdir)

    shutil.copy2(os.path.join(SOURCE_DATA, 'KILP.DAT'), instdir)
    shutil.copy2(os.path.join(SOURCE_DATA, 'KilpSrj.xml'), instdir)
    radat = os.path.join(SOURCE_DATA, 'radat1.xml')
    if os.path.exists(radat):
        shutil.copy2(radat, instdir)

    with open(os.path.join(instdir, 'laskenta.cfg'), 'w') as f:
        f.write(f'Kone={kone}\n')
        f.write('Emit\n')
        if connections:
            for i, (local_port, peer_addr, peer_port) in enumerate(connections):
                f.write(f'yhteys{i+1}=udp:{local_port}/{peer_addr}:{peer_port}\n')
                f.write(f'lähemit{i+1}\n')

    return instdir


# --- HkMaali C++ session ---

class HkMaaliSession:
    """Manages a HkMaali C++ process via pseudo-terminal."""

    def __init__(self, workdir, name=''):
        self.workdir = workdir
        self.name = name
        self.pid = None
        self.fd = None
        self.all_output = b''

    def start(self):
        self.pid, self.fd = pty.fork()
        if self.pid == 0:
            os.chdir(self.workdir)
            os.execv(HKMAALI, ['HkMaali'])
        fcntl.ioctl(self.fd, termios.TIOCSWINSZ,
                     struct.pack('HHHH', 50, 80, 0, 0))

    def read(self, timeout=1.0):
        data = b''
        end = time.time() + timeout
        while time.time() < end:
            r, _, _ = select.select([self.fd], [], [], 0.1)
            if r:
                try:
                    chunk = os.read(self.fd, 65536)
                    if chunk:
                        data += chunk
                except:
                    break
        self.all_output += data
        return data

    def send(self, key, delay=0.2):
        os.write(self.fd, key.encode() if isinstance(key, str) else key)
        time.sleep(delay)

    def send_read(self, key, delay=0.3, read_timeout=0.5):
        self.send(key, delay)
        return self.read(read_timeout)

    def output_text(self):
        return self.all_output.decode('utf-8', errors='replace')

    def stop(self):
        if self.pid:
            try:
                os.kill(self.pid, 9)
            except:
                pass
            try:
                os.waitpid(self.pid, 0)
            except:
                pass
            self.pid = None

    def accept_and_wait(self):
        """Accept initial settings screen and wait for startup."""
        self.read(2.0)
        self.send_read(KEY_ENTER, 1.0, 2.0)

    def navigate_to_korjaa(self, competitor):
        """Navigate: Korjaukset -> Korjaa -> Find competitor by number."""
        self.send_read('K', 0.5, 0.5)
        self.send_read('K', 0.5, 0.5)
        self.send_read(competitor + KEY_ENTER, 0.5, 2.0)

    def change_emit(self, new_value):
        """Tab to EME field, delete old value, type new, accept with +."""
        for _ in range(TABS_TO_EME):
            self.send(KEY_TAB, 0.05)
            time.sleep(0.3)
            self.read(0.5)
        time.sleep(0.5)
        self.read(0.5)

        for _ in range(8):
            self.send(KEY_DELETE, 0.1)
            time.sleep(0.1)
            self.read(0.1)
        time.sleep(0.3)

        for ch in new_value:
            self.send(ch, 0.1)
            self.read(0.1)
        time.sleep(0.3)

        self.send_read('+', 0.5, 2.0)

    def read_competitor_emit(self, competitor):
        """Navigate to competitor, return all output text, then escape back."""
        self.all_output = b''
        self.navigate_to_korjaa(competitor)
        text = self.output_text()
        self.escape_to_main()
        return text

    def escape_to_main(self):
        """ESC back to main menu."""
        for _ in range(3):
            self.send_read(KEY_ESC, 0.3, 0.5)

    def has_startup_errors(self):
        """Check output for KILP.DAT corruption errors."""
        text = self.output_text()
        return 'yhteensopivia' in text or 'DATA_ERR' in text


# --- Webadmin Java session ---

class WebadminSession:
    """Manages a webadmin Spring Boot process and Playwright interactions."""

    def __init__(self, datadir, http_port):
        self.datadir = datadir
        self.http_port = http_port
        self.proc = None

    def start(self):
        """Start webadmin jar. Builds if jar is missing."""
        if not os.path.exists(WEBADMIN_JAR):
            print("   Building webadmin jar...")
            result = subprocess.run(
                ['mvn', 'package', '-DskipTests', '-q'],
                cwd=WEBADMIN_DIR, capture_output=True, timeout=120)
            if result.returncode != 0:
                raise RuntimeError("mvn package failed")

        self.proc = subprocess.Popen(
            ['java', '-jar', WEBADMIN_JAR,
             f'--tulospalvelu.data-dir={self.datadir}',
             '--tulospalvelu.auto-start=true',
             f'--server.port={self.http_port}'],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            preexec_fn=os.setsid)

        import urllib.request
        start = time.time()
        while time.time() - start < 30:
            try:
                resp = urllib.request.urlopen(
                    f'http://localhost:{self.http_port}/', timeout=2)
                if resp.status == 200:
                    print(f"   Webadmin ready on port {self.http_port} "
                          f"({int(time.time()-start):.0f}s)")
                    return
            except:
                pass
            time.sleep(1)
        print("   WARNING: Webadmin may not have started in time")

    def stop(self):
        if self.proc:
            try:
                os.killpg(os.getpgid(self.proc.pid), signal.SIGTERM)
                self.proc.wait(5)
            except:
                try:
                    os.killpg(os.getpgid(self.proc.pid), signal.SIGKILL)
                except:
                    pass
            self.proc = None

    def _open_card_change(self, page):
        """Navigate Playwright page to Card Change view."""
        page.goto(f'http://localhost:{self.http_port}/', timeout=15000)
        page.wait_for_load_state('networkidle', timeout=10000)
        page.get_by_text('Card Change').click()
        time.sleep(2)
        page.wait_for_load_state('networkidle', timeout=10000)

    def check_emit(self, expected_emit):
        """Use Playwright to check if expected emit value is visible in the grid."""
        from playwright.sync_api import sync_playwright
        with sync_playwright() as p:
            browser = p.chromium.launch(headless=True)
            page = browser.new_page()
            try:
                self._open_card_change(page)
                search = page.get_by_role('textbox', name='Hae kilpailija')
                search.fill('88')
                time.sleep(2)
                page_text = page.locator('body').inner_text()
                return expected_emit in page_text
            except Exception as e:
                print(f"   Playwright error: {e}")
                return False
            finally:
                browser.close()

    def change_emit(self, new_emit, competitor='88'):
        """Use Playwright to change emit card via webadmin UI."""
        from playwright.sync_api import sync_playwright
        with sync_playwright() as p:
            browser = p.chromium.launch(headless=True)
            page = browser.new_page()
            try:
                self._open_card_change(page)
                page.get_by_role('textbox', name='Kilpailukortin numero').fill(new_emit)
                search = page.get_by_role('textbox', name='Hae kilpailija')
                search.fill(competitor)
                time.sleep(2)
                # Click the first grid body row via JS (Vaadin grid shadow DOM)
                page.evaluate("""() => {
                    const grid = document.querySelector('vaadin-grid');
                    if (grid) {
                        const row = grid.shadowRoot?.querySelector('#items tr');
                        if (row) row.click();
                        else {
                            // Fallback: click first visible cell content
                            const cells = document.querySelectorAll('vaadin-grid-cell-content');
                            for (const c of cells) {
                                if (c.textContent.trim().length > 0 && !c.closest('thead')) {
                                    c.click(); break;
                                }
                            }
                        }
                    }
                }""")
                time.sleep(1)
                page.get_by_text('Vaihda kortti').click()
                time.sleep(2)
                return True
            except Exception as e:
                print(f"   Playwright error: {e}")
                return False
            finally:
                browser.close()
