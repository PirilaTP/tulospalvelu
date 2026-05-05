"""
Shared test harness for HkMaali / webadmin integration tests.

Provides:
  - HkMaaliSession: PTY-based C++ HkMaali process control
  - WebadminSession: Java webadmin process + Playwright helpers
  - setup_data_dir(): create a test data directory with KILP.DAT etc.
  - Paths and constants
"""

import pty, os, select, time, struct, fcntl, termios, shutil, subprocess, signal, sys

# Ensure user-site-packages is on sys.path (setsid/subprocess-launched contexts
# sometimes inherit a stripped PYTHONPATH where ~/.local site-packages is not
# picked up automatically).
_user_site = os.path.expanduser('~/.local/lib/python3.11/site-packages')
if os.path.isdir(_user_site) and _user_site not in sys.path:
    sys.path.insert(0, _user_site)

# --- Paths ---

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.join(SCRIPT_DIR, '..')
HKMAALI = os.path.join(PROJECT_ROOT, 'TPsource', 'V52', 'HkMaali')
SOURCE_DATA = os.path.join(PROJECT_ROOT, 'kisat', 'HkKisaWinData')
WEBADMIN_DIR = os.path.join(PROJECT_ROOT, 'webadmin')


def _find_webadmin_jar():
    """Glob for webadmin-*.jar so the harness keeps working when the version bumps."""
    import glob
    candidates = sorted(glob.glob(os.path.join(WEBADMIN_DIR, 'target', 'webadmin-*.jar')),
                        reverse=True)
    return candidates[0] if candidates else os.path.join(
        WEBADMIN_DIR, 'target', 'webadmin.jar')


WEBADMIN_JAR = _find_webadmin_jar()

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

# KILP.DAT layout (default field sizes — see KilpReader.java)
KILPRECSIZE0 = 360
PV_OFF_BADGE = 68
PV_OFF_KESKHYL = 128
PV_OFF_VA = 152
OFF_KILPNO = 2


# --- KILP.DAT inspection / mutation helpers ---

def kilp_layout(kilp_path):
    """Detect (numrec, reclen, kilppvtpsize, n_pv) from a KILP.DAT file."""
    import os, struct
    size = os.path.getsize(kilp_path)
    with open(kilp_path, 'rb') as f:
        header = f.read(8)
    numrec = struct.unpack('<H', header[6:8])[0]
    reclen = size // numrec
    pv_total = reclen - KILPRECSIZE0
    n_pv = 1
    kilppvtpsize = pv_total
    for npv in range(3, 0, -1):
        if pv_total % npv == 0:
            cand = pv_total // npv
            if cand >= 152 and (cand - 152) % 8 == 0:
                kilppvtpsize = cand
                n_pv = npv
                break
    return numrec, reclen, kilppvtpsize, n_pv


def find_record_by_kilpno(kilp_path, kilpno):
    """Return record_index for a given kilpno, or None if not found."""
    import struct
    numrec, reclen, _, _ = kilp_layout(kilp_path)
    with open(kilp_path, 'rb') as f:
        for i in range(1, numrec):
            f.seek(i * reclen + OFF_KILPNO)
            v = struct.unpack('<H', f.read(2))[0]
            if v == kilpno:
                return i
    return None


def read_pv_badge(kilp_path, record_index, pv_index=0):
    """Read pv[pv_index].badge[0] for a given record. Returns int."""
    import struct
    _, reclen, kilppvtpsize, _ = kilp_layout(kilp_path)
    offset = record_index * reclen + KILPRECSIZE0 + pv_index * kilppvtpsize + PV_OFF_BADGE
    with open(kilp_path, 'rb') as f:
        f.seek(offset)
        return struct.unpack('<i', f.read(4))[0]


def read_pv_status(kilp_path, record_index, pv_index=0):
    """Return (keskhyl_char, finish_time_ms, ysija) for a given record/stage."""
    import struct
    _, reclen, kilppvtpsize, _ = kilp_layout(kilp_path)
    pv_base = record_index * reclen + KILPRECSIZE0 + pv_index * kilppvtpsize
    with open(kilp_path, 'rb') as f:
        f.seek(pv_base + PV_OFF_KESKHYL)
        keskhyl = struct.unpack('<H', f.read(2))[0]
        f.seek(pv_base + PV_OFF_VA + 8)  # vatp[1].time
        finish = struct.unpack('<i', f.read(4))[0]
        ysija = struct.unpack('<i', f.read(4))[0]
    return chr(keskhyl) if keskhyl > 0 else '\0', finish, ysija


def clear_pv_status(kilp_path, record_index, pv_index=0):
    """Zero out keskhyl + vatp[1].time/ysija for a stage so it counts as 'open'."""
    import struct
    _, reclen, kilppvtpsize, _ = kilp_layout(kilp_path)
    pv_base = record_index * reclen + KILPRECSIZE0 + pv_index * kilppvtpsize
    with open(kilp_path, 'r+b') as f:
        f.seek(pv_base + PV_OFF_KESKHYL)
        f.write(b'\x00\x00')
        f.seek(pv_base + PV_OFF_VA + 8)
        f.write(b'\x00' * 8)  # finishTime + ysija both zeroed


# --- Data directory setup ---

def setup_data_dir(name, kone, connections=None, base_dir=None, paiva=None,
                   source_data=None):
    """Create a test data directory with KILP.DAT, KilpSrj.xml, laskenta.cfg.

    Args:
        name: directory suffix (e.g. 'MA', 'WI')
        kone: machine ID for laskenta.cfg (e.g. 'MA', 'WI', 'WB')
        connections: list of (local_port, peer_addr, peer_port) tuples, or None for no network
        base_dir: parent directory (default: SCRIPT_DIR)
        paiva: if set, writes PÄIVÄ=<paiva> line (active stage, 1-based)
        source_data: override source data dir (defaults to SOURCE_DATA)
    Returns:
        path to created directory
    """
    base = base_dir or SCRIPT_DIR
    src = source_data or SOURCE_DATA
    instdir = os.path.join(base, f'test_data_{name}')
    if os.path.exists(instdir):
        shutil.rmtree(instdir)
    os.makedirs(instdir)

    shutil.copy2(os.path.join(src, 'KILP.DAT'), instdir)
    shutil.copy2(os.path.join(src, 'KilpSrj.xml'), instdir)
    radat = os.path.join(src, 'radat1.xml')
    if os.path.exists(radat):
        shutil.copy2(radat, instdir)

    with open(os.path.join(instdir, 'laskenta.cfg'), 'w') as f:
        f.write(f'Kone={kone}\n')
        f.write('Emit\n')
        if paiva is not None:
            f.write(f'PÄIVÄ={paiva}\n')
        if connections:
            for i, conn in enumerate(connections):
                # conn = (local_port, peer_addr, peer_port) or
                #        (local_port, peer_addr, peer_port, lahemit_suffix)
                # lahemit_suffix e.g. 'O' → 'lähemit<n>=O' (yksisuuntainen)
                if len(conn) == 4:
                    local_port, peer_addr, peer_port, suffix = conn
                    lahemit_line = f'lähemit{i+1}={suffix}'
                else:
                    local_port, peer_addr, peer_port = conn
                    lahemit_line = f'lähemit{i+1}'
                f.write(f'yhteys{i+1}=udp:{local_port}/{peer_addr}:{peer_port}\n')
                f.write(f'{lahemit_line}\n')

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
        """Accept initial settings screen and wait until main menu is ready.

        Keeps dismissing prompts (production data produces several error
        prompts like 'LEIMAT.LST ei onnistu', 'Ratatietojen lukeminen...')
        until the main menu string appears in the output, up to a timeout.
        """
        import time as _t
        deadline = _t.time() + 25.0
        self.read(3.0)
        self.send_read(KEY_ENTER, 0.5, 2.0)
        last_sent_len = 0
        while _t.time() < deadline:
            _t.sleep(1.0)
            self.read(1.0)
            text = self.output_text()
            if 'PÄÄVALIKKO' in text[-4000:] or 'M)aali' in text[-2000:]:
                return
            # Strip ANSI escape sequences + control chars so prompt matching
            # looks at the actual rendered text, not cursor-move noise.
            import re as _re
            clean = _re.sub(r'\x1b\[[?0-9;]*[a-zA-Z]', '', text)
            clean = _re.sub(r'[\x00-\x08\x0b-\x1f\x7f]', '', clean)
            tail = clean[-500:]
            if 'J)atka ilman' in tail:
                self.send_read('J', 0.5, 1.5)
            elif 'L)opeta n' in tail or 'Lopeta näm' in tail or 'n�m�' in tail:
                # 'J)atka, L)opeta nämä virheilmoitukset' — choose L to stop flood
                self.send_read('L', 0.5, 1.5)
            elif 'Paina Enter' in tail:
                self.send_read(KEY_ENTER, 0.5, 1.5)
            elif 'Vahvista kilpailup' in tail:
                self.send_read(KEY_ENTER, 0.5, 1.5)
            elif len(text) == last_sent_len:
                # No new output — just poke with ENTER
                self.send_read(KEY_ENTER, 0.5, 1.5)
            last_sent_len = len(text)

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
        jar = _find_webadmin_jar()
        if not os.path.exists(jar):
            print("   Building webadmin jar...")
            result = subprocess.run(
                ['mvn', 'package', '-DskipTests', '-q'],
                cwd=WEBADMIN_DIR, capture_output=True, timeout=300)
            if result.returncode != 0:
                raise RuntimeError("mvn package failed")
            jar = _find_webadmin_jar()

        self.log_path = f'/tmp/webadmin-{self.http_port}.log'
        self.log_file = open(self.log_path, 'w')
        self.proc = subprocess.Popen(
            ['java', '-jar', jar,
             f'--tulospalvelu.data-dir={self.datadir}',
             '--tulospalvelu.auto-start=true',
             f'--server.port={self.http_port}'],
            stdout=self.log_file,
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

    def check_emit(self, expected_emit, competitor='88'):
        """Use Playwright to check if expected emit value is visible in the grid."""
        from playwright.sync_api import sync_playwright
        with sync_playwright() as p:
            browser = p.chromium.launch(headless=True)
            page = browser.new_page()
            try:
                self._open_card_change(page)
                # Fill the first (competitor selector) vaadin-text-field
                page.locator('vaadin-text-field').nth(0).locator('input').fill(competitor)
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
                # First vaadin-text-field = competitor selector
                page.locator('vaadin-text-field').nth(0).locator('input').fill(competitor)
                time.sleep(1.5)
                # Second vaadin-text-field = new card number
                page.locator('vaadin-text-field').nth(1).locator('input').fill(new_emit)
                time.sleep(0.5)
                page.get_by_role('button', name='Vaihda kortti').click()
                time.sleep(3)
                return True
            except Exception as e:
                print(f"   Playwright error: {e}")
                return False
            finally:
                browser.close()
