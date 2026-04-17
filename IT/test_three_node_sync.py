#!/usr/bin/env python3
"""
Integration test: Three-node sync — two C++ HkMaali + one Java webadmin.

Scenario:
1. Start MA (HkMaali, port 41901), WI (HkMaali, port 41902), WB (webadmin, port 41903)
   All connected via UDP on localhost forming a triangle:
     MA <-> WI, MA <-> WB, WI <-> WB
2. MA changes competitor 88's emit card to 111111
3. Verify WI and WB both receive the change
4. Use Playwright on WB to change emit to 222222
5. Verify MA and WI both receive the change
6. WI changes emit to 333333
7. Verify MA and WB both receive the change

Uses high port numbers (41901-41903) and webadmin HTTP on 48080.
"""

import pty, os, select, time, struct, fcntl, termios, re, sys, shutil, hashlib
import subprocess, signal

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.join(SCRIPT_DIR, '..')
HKMAALI = os.path.join(PROJECT_ROOT, 'TPsource', 'V52', 'HkMaali')
SOURCE_DATA = os.path.join(PROJECT_ROOT, 'kisat', 'HkKisaWinData')
WEBADMIN_DIR = os.path.join(PROJECT_ROOT, 'webadmin')

COMPETITOR = '88'
ORIGINAL_EMIT = '15676'
EMIT_A = '111111'  # MA sets
EMIT_B = '222222'  # WB sets (via Playwright)
EMIT_C = '333333'  # WI sets

PORT_MA = 41901
PORT_WI = 41902
PORT_WB = 41903
HTTP_PORT = 48080

KEY_TAB = '\t'
KEY_ENTER = '\r'
KEY_DELETE = '\x1b[3~'
TABS_TO_EME = 9
SYNC_WAIT = 4  # seconds to wait for UDP sync


class HkMaaliSession:
    """Manages a HkMaali C++ process via pty."""
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
                    if chunk: data += chunk
                except: break
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
            try: os.kill(self.pid, 9)
            except: pass
            try: os.waitpid(self.pid, 0)
            except: pass
            self.pid = None

    def accept_and_wait(self):
        self.read(2.0)
        self.send_read(KEY_ENTER, 1.0, 2.0)

    def navigate_to_korjaa(self, competitor):
        self.send_read('K', 0.5, 0.5)
        self.send_read('K', 0.5, 0.5)
        self.send_read(competitor + KEY_ENTER, 0.5, 2.0)

    def change_emit(self, new_value):
        for i in range(TABS_TO_EME):
            self.send(KEY_TAB, 0.05)
            time.sleep(0.3)
            self.read(0.5)
        time.sleep(0.5)
        self.read(0.5)
        for i in range(8):
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
        self.all_output = b''
        self.send_read('K', 0.5, 0.5)
        self.send_read('K', 0.5, 0.5)
        self.send_read(competitor + KEY_ENTER, 0.5, 2.0)
        text = self.output_text()
        self.escape_to_main()
        return text

    def escape_to_main(self):
        for _ in range(3):
            self.send_read('\x1b', 0.3, 0.5)


def setup_instance(name, kone, connections):
    """Create data dir for an instance with given UDP connections.
    connections: list of (own_port, peer_addr, peer_port) tuples."""
    instdir = os.path.join(SCRIPT_DIR, f'test_data_3node_{name}')
    if os.path.exists(instdir):
        shutil.rmtree(instdir)
    os.makedirs(instdir)
    shutil.copy2(os.path.join(SOURCE_DATA, 'KILP.DAT'), instdir)
    shutil.copy2(os.path.join(SOURCE_DATA, 'KilpSrj.xml'), instdir)
    src_radat = os.path.join(SOURCE_DATA, 'radat1.xml')
    if os.path.exists(src_radat):
        shutil.copy2(src_radat, instdir)
    with open(os.path.join(instdir, 'laskenta.cfg'), 'w') as f:
        f.write(f'Kone={kone}\n')
        f.write('Emit\n')
        for i, (own_port, peer_addr, peer_port) in enumerate(connections):
            f.write(f'yhteys{i+1}=udp:{own_port}/{peer_addr}:{peer_port}\n')
            f.write(f'lähemit{i+1}\n')
    return instdir


def start_webadmin(datadir, http_port):
    """Start webadmin via java -jar (production build, fast startup)."""
    jar = os.path.join(WEBADMIN_DIR, 'target', 'webadmin-1.0-SNAPSHOT.jar')
    if not os.path.exists(jar):
        print(f"   Building webadmin jar...")
        result = subprocess.run(
            ['mvn', 'package', '-DskipTests', '-q'],
            cwd=WEBADMIN_DIR, capture_output=True, timeout=120)
        if result.returncode != 0:
            print(f"   FAIL: mvn package failed")
            return None

    proc = subprocess.Popen(
        ['java', '-jar', jar,
         f'--tulospalvelu.data-dir={datadir}',
         '--tulospalvelu.auto-start=true',
         f'--server.port={http_port}'],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        preexec_fn=os.setsid
    )
    import urllib.request
    start = time.time()
    while time.time() - start < 30:
        try:
            resp = urllib.request.urlopen(f'http://localhost:{http_port}/', timeout=2)
            if resp.status == 200:
                print(f"   Webadmin ready on port {http_port} ({int(time.time()-start):.0f}s)")
                return proc
        except:
            pass
        time.sleep(1)
    print("   WARNING: Webadmin may not have started in time")
    return proc


def open_card_change_page(page, http_port):
    """Navigate to the Card Change view."""
    page.goto(f'http://localhost:{http_port}/', timeout=15000)
    page.wait_for_load_state('networkidle', timeout=10000)
    page.get_by_text('Card Change').click()
    time.sleep(2)
    page.wait_for_load_state('networkidle', timeout=10000)


def check_webadmin_emit(http_port, expected_emit):
    """Use Playwright to check the emit value visible in webadmin."""
    from playwright.sync_api import sync_playwright
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page()
        try:
            open_card_change_page(page, http_port)
            search = page.get_by_role('textbox', name='Hae kilpailija')
            search.fill('88')
            time.sleep(2)
            page_text = page.locator('body').inner_text()
            browser.close()
            return expected_emit in page_text
        except Exception as e:
            print(f"   Playwright error: {e}")
            try: browser.close()
            except: pass
            return False


def change_emit_via_webadmin(http_port, new_emit, competitor='88'):
    """Use Playwright to change emit via webadmin UI."""
    from playwright.sync_api import sync_playwright
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page()
        try:
            open_card_change_page(page, http_port)

            # Enter new card number
            card_input = page.get_by_role('textbox', name='Kilpailukortin numero')
            card_input.fill(new_emit)

            # Search for competitor
            search = page.get_by_role('textbox', name='Hae kilpailija')
            search.fill(competitor)
            time.sleep(2)

            # Select competitor from grid (click the row with competitor number)
            page.get_by_text('Liimatainen').first.click()
            time.sleep(1)

            # Click "Vaihda kortti" button
            page.get_by_text('Vaihda kortti').click()
            time.sleep(2)

            browser.close()
            return True
        except Exception as e:
            print(f"   Playwright error: {e}")
            try: browser.close()
            except: pass
            return False


def test_three_node_sync():
    print("=" * 60)
    print("TEST: Three-node sync (2x HkMaali + webadmin)")
    print("=" * 60)

    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali not found. Run 'make' in TPsource/V52/.")
        return False

    # Setup instances — star topology: MA is hub
    # Each connection needs a unique local:remote port pair.
    # MA:41901 <-> WI:41902 (connection pair 1)
    # MA:41911 <-> WB:41903 (connection pair 2)
    dir_ma = setup_instance('MA', 'MA', [
        (PORT_MA, '127.0.0.1', PORT_WI),       # yhteys1: MA:41901 <-> WI:41902
        (PORT_MA + 10, '127.0.0.1', PORT_WB),  # yhteys2: MA:41911 <-> WB:41903
    ])
    dir_wi = setup_instance('WI', 'WI', [
        (PORT_WI, '127.0.0.1', PORT_MA),       # yhteys1: WI:41902 <-> MA:41901
    ])
    dir_wb = setup_instance('WB', 'WB', [
        (PORT_WB, '127.0.0.1', PORT_MA + 10),  # yhteys1: WB:41903 <-> MA:41911
    ])

    print(f"\n1. Instances:")
    print(f"   MA: {dir_ma} (UDP {PORT_MA})")
    print(f"   WI: {dir_wi} (UDP {PORT_WI})")
    print(f"   WB: {dir_wb} (UDP {PORT_WB}, HTTP {HTTP_PORT})")

    sess_ma = HkMaaliSession(dir_ma, 'MA')
    sess_wi = HkMaaliSession(dir_wi, 'WI')
    wb_proc = None
    results = {'ma_to_wi': False, 'ma_to_wb': False,
               'wb_to_ma': False, 'wb_to_wi': False,
               'wi_to_ma': False, 'wi_to_wb': False}

    try:
        # Start C++ instances
        print("\n2. Starting instances...")
        sess_ma.start()
        sess_wi.start()
        sess_ma.accept_and_wait()
        sess_wi.accept_and_wait()

        # Start webadmin
        print("   Starting webadmin (may take a moment)...")
        wb_proc = start_webadmin(dir_wb, HTTP_PORT)
        time.sleep(SYNC_WAIT)
        sess_ma.read(1.0)
        sess_wi.read(1.0)
        print("   All instances running.")

        # === Phase A: MA changes emit to 111111 ===
        print(f"\n3. MA: Changing emit -> {EMIT_A}...")
        sess_ma.navigate_to_korjaa(COMPETITOR)
        sess_ma.change_emit(EMIT_A)
        sess_ma.escape_to_main()
        print(f"   MA: Done.")

        time.sleep(SYNC_WAIT)
        sess_wi.read(2.0)

        # Check WI
        print(f"\n4. Checking sync from MA...")
        text_wi = sess_wi.read_competitor_emit(COMPETITOR)
        if EMIT_A in text_wi:
            print(f"   WI: {EMIT_A} ✓")
            results['ma_to_wi'] = True
        else:
            print(f"   WI: sync failed")

        # Check WB via Playwright
        wb_ok = check_webadmin_emit(HTTP_PORT, EMIT_A)
        if wb_ok:
            print(f"   WB: {EMIT_A} ✓")
            results['ma_to_wb'] = True
        else:
            print(f"   WB: sync failed (or emit not visible in UI)")

        # === Phase B: WB changes emit to 222222 via Playwright ===
        print(f"\n5. WB: Changing emit -> {EMIT_B} via Playwright...")
        wb_changed = change_emit_via_webadmin(HTTP_PORT, EMIT_B)
        if wb_changed:
            print(f"   WB: Edit submitted.")
        else:
            print(f"   WB: Edit failed!")

        time.sleep(SYNC_WAIT)
        sess_ma.read(2.0)
        sess_wi.read(2.0)

        # Check MA
        print(f"\n6. Checking sync from WB...")
        sess_ma.all_output = b''
        text_ma = sess_ma.read_competitor_emit(COMPETITOR)
        if EMIT_B in text_ma:
            print(f"   MA: {EMIT_B} ✓")
            results['wb_to_ma'] = True
        else:
            print(f"   MA: sync failed")

        # Check WI
        text_wi = sess_wi.read_competitor_emit(COMPETITOR)
        if EMIT_B in text_wi:
            print(f"   WI: {EMIT_B} ✓")
            results['wb_to_wi'] = True
        else:
            print(f"   WI: sync failed")

        # === Phase C: WI changes emit to 333333 ===
        print(f"\n7. WI: Changing emit -> {EMIT_C}...")
        sess_wi.navigate_to_korjaa(COMPETITOR)
        sess_wi.change_emit(EMIT_C)
        sess_wi.escape_to_main()
        print(f"   WI: Done.")

        time.sleep(SYNC_WAIT)
        sess_ma.read(2.0)

        # Check MA
        print(f"\n8. Checking sync from WI...")
        sess_ma.all_output = b''
        text_ma = sess_ma.read_competitor_emit(COMPETITOR)
        if EMIT_C in text_ma:
            print(f"   MA: {EMIT_C} ✓")
            results['wi_to_ma'] = True
        else:
            print(f"   MA: sync failed")

        # Check WB
        wb_ok = check_webadmin_emit(HTTP_PORT, EMIT_C)
        if wb_ok:
            print(f"   WB: {EMIT_C} ✓")
            results['wi_to_wb'] = True
        else:
            print(f"   WB: sync failed")

    finally:
        sess_ma.stop()
        sess_wi.stop()
        if wb_proc:
            os.killpg(os.getpgid(wb_proc.pid), signal.SIGTERM)
            try: wb_proc.wait(5)
            except: os.killpg(os.getpgid(wb_proc.pid), signal.SIGKILL)

    # Results
    all_pass = all(results.values())
    cpp_pass = results['ma_to_wi'] and results['wi_to_ma']
    wb_pass = results['ma_to_wb'] and results['wb_to_ma'] and results['wb_to_wi'] and results['wi_to_wb']

    if all_pass:
        shutil.rmtree(dir_ma)
        shutil.rmtree(dir_wi)
        shutil.rmtree(dir_wb)

    print(f"\n{'=' * 60}")
    print("RESULTS:")
    for k, v in results.items():
        src, dst = k.split('_to_')
        print(f"  {src.upper()} -> {dst.upper()}: {'✓' if v else 'FAIL'}")

    if all_pass:
        print("\nRESULT: PASS ✓ (all 6 sync directions work)")
    elif cpp_pass:
        print(f"\nRESULT: PARTIAL - C++ sync OK, webadmin sync {'partial' if any([results['ma_to_wb'], results['wb_to_ma']]) else 'failed'}")
    else:
        print("\nRESULT: FAIL")
    return all_pass


if __name__ == '__main__':
    success = test_three_node_sync()
    sys.exit(0 if success else 1)
