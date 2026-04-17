#!/usr/bin/env python3
"""
Integration test: Two HkMaali instances sync competitor data via UDP.

Scenario:
1. Start two HkMaali instances (MA and WI) connected via UDP on localhost
2. On machine MA: change competitor 88's emit card to 111111
3. Verify machine WI receives the change
4. On machine WI: change competitor 88's emit card to 222222
5. Verify machine MA receives the change back

Uses high port numbers (41901/41902) to avoid conflicts.
"""

import pty, os, select, time, struct, fcntl, termios, re, sys, shutil, hashlib

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.join(SCRIPT_DIR, '..')
HKMAALI = os.path.join(PROJECT_ROOT, 'TPsource', 'V52', 'HkMaali')
SOURCE_DATA = os.path.join(PROJECT_ROOT, 'kisat', 'HkKisaWinData')

COMPETITOR = '88'
ORIGINAL_EMIT = '15676'
EMIT_A = '111111'   # MA sets this
EMIT_B = '222222'   # WI sets this

PORT_MA = 41901
PORT_WI = 41902

KEY_TAB = '\t'
KEY_ENTER = '\r'
KEY_DELETE = '\x1b[3~'
TABS_TO_EME = 9   # empirically determined for this test data


class HkMaaliSession:
    def __init__(self, workdir, name='', rows=50, cols=80):
        self.workdir = workdir
        self.name = name
        self.pid = None
        self.fd = None
        self.all_output = b''
        self.rows = rows
        self.cols = cols

    def start(self):
        self.pid, self.fd = pty.fork()
        if self.pid == 0:
            os.chdir(self.workdir)
            os.execv(HKMAALI, ['HkMaali'])
        fcntl.ioctl(self.fd, termios.TIOCSWINSZ,
                     struct.pack('HHHH', self.rows, self.cols, 0, 0))

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
        """Accept settings and wait for startup."""
        self.read(2.0)
        self.send_read(KEY_ENTER, 1.0, 2.0)

    def navigate_to_korjaa(self, competitor):
        """Navigate to Korjaukset -> Korjaa -> Find competitor."""
        self.send_read('K', 0.5, 0.5)
        self.send_read('K', 0.5, 0.5)
        self.send_read(competitor + KEY_ENTER, 0.5, 2.0)

    def change_emit(self, new_value):
        """Tab to EME field, clear, type new value, accept."""
        for i in range(TABS_TO_EME):
            self.send(KEY_TAB, 0.05)
            time.sleep(0.3)
            self.read(0.5)
        time.sleep(0.5)
        self.read(0.5)

        # Delete old value
        for i in range(8):
            self.send(KEY_DELETE, 0.1)
            time.sleep(0.1)
            self.read(0.1)
        time.sleep(0.3)

        # Type new value
        for ch in new_value:
            self.send(ch, 0.1)
            self.read(0.1)
        time.sleep(0.3)

        # Accept with +
        self.send_read('+', 0.5, 2.0)

    def read_competitor_emit(self, competitor):
        """Navigate to competitor and return which emit value is visible."""
        self.send_read('K', 0.5, 0.5)
        self.send_read('K', 0.5, 0.5)
        self.send_read(competitor + KEY_ENTER, 0.5, 2.0)
        text = self.output_text()
        # Go back with ESC
        self.send_read('\x1b', 0.3, 0.5)
        self.send_read('\x1b', 0.3, 0.5)
        return text

    def escape_to_main(self):
        """ESC back to main menu."""
        for _ in range(3):
            self.send_read('\x1b', 0.3, 0.5)


def setup_instance(name, kone, own_port, peer_port):
    """Create a data directory for one HkMaali instance."""
    instdir = os.path.join(SCRIPT_DIR, f'test_data_udp_{name}')
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
        f.write(f'yhteys1=udp:{own_port}/127.0.0.1:{peer_port}\n')
        f.write('lähemit1\n')

    return instdir


def test_udp_sync():
    print("=" * 60)
    print("TEST: Two-instance UDP sync of emit card changes")
    print("=" * 60)

    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali not found. Run 'make' first.")
        return False

    # Setup two instances
    dir_ma = setup_instance('MA', 'MA', PORT_MA, PORT_WI)
    dir_wi = setup_instance('WI', 'WI', PORT_WI, PORT_MA)
    print(f"\n1. Instance MA: {dir_ma} (port {PORT_MA})")
    print(f"   Instance WI: {dir_wi} (port {PORT_WI})")

    sess_ma = HkMaaliSession(dir_ma, 'MA')
    sess_wi = HkMaaliSession(dir_wi, 'WI')

    try:
        # Start both instances
        print("\n2. Starting both instances...")
        sess_ma.start()
        sess_wi.start()
        sess_ma.accept_and_wait()
        sess_wi.accept_and_wait()

        # Wait for UDP handshake
        print("   Waiting for UDP handshake...")
        time.sleep(3)
        sess_ma.read(1.0)
        sess_wi.read(1.0)
        print("   Both instances running.")

        # Step 1: MA changes emit to 111111
        print(f"\n3. MA: Changing emit {ORIGINAL_EMIT} -> {EMIT_A}...")
        sess_ma.navigate_to_korjaa(COMPETITOR)
        sess_ma.change_emit(EMIT_A)
        sess_ma.escape_to_main()
        print(f"   MA: Edit complete.")

        # Wait for sync
        print("   Waiting for UDP sync...")
        time.sleep(3)
        sess_wi.read(2.0)

        # Step 2: Verify WI received the change
        print(f"\n4. WI: Checking emit value...")
        # Reset output tracking for cleaner detection
        sess_wi.all_output = b''
        text_wi = sess_wi.read_competitor_emit(COMPETITOR)

        sync_a_ok = False
        if EMIT_A in text_wi:
            print(f"   WI: Emit shows {EMIT_A} ✓ (sync from MA worked)")
            sync_a_ok = True
        elif ORIGINAL_EMIT in text_wi:
            print(f"   WI: Emit still shows {ORIGINAL_EMIT} (sync failed)")
        else:
            print(f"   WI: Could not determine emit value")

        # Step 3: WI changes emit to 222222
        print(f"\n5. WI: Changing emit -> {EMIT_B}...")
        sess_wi.navigate_to_korjaa(COMPETITOR)
        sess_wi.change_emit(EMIT_B)
        sess_wi.escape_to_main()
        print(f"   WI: Edit complete.")

        # Wait for sync back
        print("   Waiting for UDP sync back...")
        time.sleep(3)
        sess_ma.read(2.0)

        # Step 4: Verify MA received the change back
        print(f"\n6. MA: Checking emit value...")
        sess_ma.all_output = b''
        text_ma = sess_ma.read_competitor_emit(COMPETITOR)

        sync_b_ok = False
        if EMIT_B in text_ma:
            print(f"   MA: Emit shows {EMIT_B} ✓ (sync from WI worked)")
            sync_b_ok = True
        elif EMIT_A in text_ma:
            print(f"   MA: Emit still shows {EMIT_A} (reverse sync failed)")
        else:
            print(f"   MA: Could not determine emit value")

    finally:
        sess_ma.stop()
        sess_wi.stop()

    # Cleanup on success
    if sync_a_ok and sync_b_ok:
        shutil.rmtree(dir_ma)
        shutil.rmtree(dir_wi)

    # Results
    print(f"\n{'=' * 60}")
    if sync_a_ok and sync_b_ok:
        print("RESULT: PASS ✓")
        print(f"  - MA -> WI sync: {ORIGINAL_EMIT} -> {EMIT_A} ✓")
        print(f"  - WI -> MA sync: {EMIT_A} -> {EMIT_B} ✓")
        return True
    elif sync_a_ok:
        print("RESULT: PARTIAL - Forward sync works, reverse failed")
        return False
    else:
        print("RESULT: FAIL - No sync detected")
        return False


if __name__ == '__main__':
    success = test_udp_sync()
    sys.exit(0 if success else 1)
