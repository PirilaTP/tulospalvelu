#!/usr/bin/env python3
"""
Integration test: Change emit card number for competitor 88.

Creates a temporary data directory with minimal config (no network),
runs HkMaali, changes the emit card, restarts and verifies the change persisted.
"""

import pty, os, select, time, struct, fcntl, termios, re, sys, shutil, tempfile, hashlib

HKMAALI = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'HkMaali')
SOURCE_DATA = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'kisat', 'HkKisaWinData')

COMPETITOR = '88'
OLD_EMIT = '15676'
NEW_EMIT = '123456'

class HkMaaliSession:
    """Manages a HkMaali process via pseudo-terminal."""

    def __init__(self, workdir, rows=50, cols=80):
        self.workdir = workdir
        self.rows = rows
        self.cols = cols
        self.pid = None
        self.fd = None
        self.all_output = b''

    def start(self):
        self.pid, self.fd = pty.fork()
        if self.pid == 0:
            os.chdir(self.workdir)
            os.execv(HKMAALI, ['HkMaali'])
        winsize = struct.pack('HHHH', self.rows, self.cols, 0, 0)
        fcntl.ioctl(self.fd, termios.TIOCSWINSZ, winsize)

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

    def send(self, key, delay=0.3):
        os.write(self.fd, key.encode() if isinstance(key, str) else key)
        time.sleep(delay)

    def send_read(self, key, delay=0.3, read_timeout=0.5):
        self.send(key, delay)
        return self.read(read_timeout)

    def output_contains(self, text):
        return text.encode() in self.all_output if isinstance(text, str) else text in self.all_output

    def stop(self):
        if self.pid:
            try:
                self.send('\x1b', 0.2)  # ESC
                self.read(0.3)
                self.send('\x1b', 0.2)
                self.read(0.3)
                self.send('\x1b', 0.2)
                self.read(0.3)
                self.send('P', 0.2)     # Poistu
                self.read(0.5)
                self.send('K', 0.2)     # Kyllä (confirm exit)
                self.read(0.5)
            except:
                pass
            try:
                os.kill(self.pid, 9)
            except:
                pass
            try:
                os.waitpid(self.pid, 0)
            except:
                pass
            self.pid = None


def setup_test_dir():
    """Create temp directory with minimal data files (no network)."""
    tmpdir = tempfile.mkdtemp(prefix='hkmaali_test_')

    # Copy KILP.DAT - the competitor database
    shutil.copy2(os.path.join(SOURCE_DATA, 'KILP.DAT'), tmpdir)

    # Copy KilpSrj.xml - competition structure
    shutil.copy2(os.path.join(SOURCE_DATA, 'KilpSrj.xml'), tmpdir)

    # Copy radat1.xml if exists - course data
    src_radat = os.path.join(SOURCE_DATA, 'radat1.xml')
    if os.path.exists(src_radat):
        shutil.copy2(src_radat, tmpdir)

    # Create minimal laskenta.cfg WITHOUT network connections
    with open(os.path.join(tmpdir, 'laskenta.cfg'), 'w') as f:
        f.write('Kone=MA\nEmit\n')

    return tmpdir


def read_emit_from_kilpdat(kilpdat_path, competitor_no=88):
    """Read emit card value for a competitor directly from KILP.DAT binary."""
    with open(kilpdat_path, 'rb') as f:
        data = f.read()

    # Search for the emit card number in UTF-16LE near competitor data
    # The emit card (badge) is stored as INT32 in the competitor record
    # For now, just search for the known values as UTF-16LE text
    for value in [NEW_EMIT, OLD_EMIT]:
        # Check as UTF-16LE text
        utf16 = value.encode('utf-16-le')
        if utf16 in data:
            return value

    return None


def test_emit_change():
    """Main test: change emit card and verify persistence."""
    print("=" * 60)
    print("TEST: Change emit card for competitor 88")
    print("=" * 60)

    # Check binary exists
    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali binary not found at {HKMAALI}")
        print(f"  Run 'make' first in {os.path.dirname(HKMAALI)}")
        return False

    # Setup
    tmpdir = setup_test_dir()
    kilpdat = os.path.join(tmpdir, 'KILP.DAT')
    original_md5 = hashlib.md5(open(kilpdat, 'rb').read()).hexdigest()
    print(f"\n1. Test directory: {tmpdir}")
    print(f"   KILP.DAT md5: {original_md5}")

    # Phase 1: Open program, change emit card, exit
    print(f"\n2. Starting HkMaali, changing emit {OLD_EMIT} -> {NEW_EMIT}...")
    sess = HkMaaliSession(tmpdir)
    sess.start()

    try:
        sess.read(2.0)                          # Wait for startup
        sess.send_read('\r', 1.0, 1.0)          # Accept settings
        sess.send_read('K', 0.5, 0.5)           # Korjaukset
        sess.send_read('K', 0.5, 0.5)           # Korjaa
        sess.send_read(f'{COMPETITOR}\r', 0.5, 2.0)  # Find competitor

        # Tab to EME field (emit card). Field sequence starts at TRKE(15).
        # 15->16->...->27->1->2->3->4->5->6(EME) = 18 tabs
        for i in range(18):
            sess.send('\t', 0.15)
            sess.read(0.2)

        # Clear old value and type new one
        for i in range(10):
            sess.send('\x08', 0.05)  # backspace
            sess.read(0.05)

        sess.send_read(NEW_EMIT, 0.3, 0.3)     # Type new value
        sess.send_read('+', 0.5, 1.0)           # Accept with +

        print("   Edit complete, exiting...")
    finally:
        sess.stop()

    # Check file was modified
    after_md5 = hashlib.md5(open(kilpdat, 'rb').read()).hexdigest()
    if after_md5 == original_md5:
        print(f"   WARNING: KILP.DAT unchanged (md5 still {original_md5})")
        print("   The save may not have worked.")
    else:
        print(f"   KILP.DAT modified (md5: {after_md5})")

    # Phase 2: Restart and verify
    print(f"\n3. Restarting HkMaali to verify...")
    sess2 = HkMaaliSession(tmpdir)
    sess2.start()

    startup_ok = True
    emit_found = False

    try:
        out = sess2.read(2.0)
        sess2.send_read('\r', 1.0, 1.0)          # Accept settings

        # Check for errors
        text = sess2.all_output.decode('utf-8', errors='replace')
        if 'yhteensopivia' in text or 'DATA_ERR' in text or 'eroa)' in text:
            print("   FAIL: Startup error detected!")
            # Show the error
            for line in text.split('\n'):
                if 'yhteensopivia' in line or 'DATA_ERR' in line or 'eroa' in line:
                    clean = re.sub(r'\033\[[0-9;]*[A-Za-z]', '', line).strip()
                    if clean:
                        print(f"   Error: {clean}")
            startup_ok = False
        else:
            print("   Startup OK (no errors)")

        # Navigate to competitor 88 and check emit value
        sess2.send_read('K', 0.5, 0.5)           # Korjaukset
        sess2.send_read('K', 0.5, 0.5)           # Korjaa
        sess2.send_read(f'{COMPETITOR}\r', 0.5, 2.0)  # Find competitor

        text2 = sess2.all_output.decode('utf-8', errors='replace')
        if NEW_EMIT in text2:
            emit_found = True
            print(f"   Emit card shows {NEW_EMIT} ✓")
        elif OLD_EMIT in text2:
            print(f"   Emit card still shows {OLD_EMIT} (change not saved)")
        else:
            print(f"   Could not find emit value in output")

    finally:
        sess2.stop()

    # Cleanup
    shutil.rmtree(tmpdir)

    # Results
    print(f"\n{'=' * 60}")
    if startup_ok and emit_found:
        print("RESULT: PASS ✓")
        print(f"  - Emit card changed {OLD_EMIT} -> {NEW_EMIT}")
        print(f"  - Change persisted after restart")
        print(f"  - No data corruption")
        return True
    elif startup_ok and not emit_found:
        print("RESULT: FAIL - Save did not persist")
        print(f"  - Program starts OK but emit card not changed")
        return False
    elif not startup_ok:
        print("RESULT: FAIL - Data corruption!")
        print(f"  - KILP.DAT corrupted after edit")
        return False
    else:
        print("RESULT: FAIL")
        return False


if __name__ == '__main__':
    success = test_emit_change()
    sys.exit(0 if success else 1)
