#!/usr/bin/env python3
"""
Integration test: Change emit card number for competitor 88.

Creates a temporary data directory with minimal config (no network),
runs HkMaali, changes the emit card, restarts and verifies the change persisted.
"""

import pty, os, select, time, struct, fcntl, termios, re, sys, shutil, hashlib

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.join(SCRIPT_DIR, '..')
HKMAALI = os.path.join(PROJECT_ROOT, 'TPsource', 'V52', 'HkMaali')
SOURCE_DATA = os.path.join(PROJECT_ROOT, 'kisat', 'HkKisaWinData')

COMPETITOR = '88'
OLD_EMIT = '15676'
NEW_EMIT = '123456'

KEY_TAB = '\t'
KEY_ENTER = '\r'
KEY_ESC = '\x1b'
KEY_DELETE = '\x1b[3~'


class HkMaaliSession:
    def __init__(self, workdir, rows=50, cols=80):
        self.workdir = workdir
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

    def recent_text(self, data):
        return data.decode('utf-8', errors='replace')

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


def setup_test_dir():
    tmpdir = os.path.join(SCRIPT_DIR, 'test_data_tmp')
    if os.path.exists(tmpdir):
        shutil.rmtree(tmpdir)
    os.makedirs(tmpdir)
    shutil.copy2(os.path.join(SOURCE_DATA, 'KILP.DAT'), tmpdir)
    shutil.copy2(os.path.join(SOURCE_DATA, 'KilpSrj.xml'), tmpdir)
    src_radat = os.path.join(SOURCE_DATA, 'radat1.xml')
    if os.path.exists(src_radat):
        shutil.copy2(src_radat, tmpdir)
    with open(os.path.join(tmpdir, 'laskenta.cfg'), 'w') as f:
        f.write('Kone=MA\nEmit\n')
    return tmpdir


def navigate_to_korjaa(sess, competitor):
    """Navigate: Accept -> Korjaukset -> Korjaa -> Find competitor"""
    sess.read(2.0)
    sess.send_read(KEY_ENTER, 1.0, 1.0)
    sess.send_read('K', 0.5, 0.5)
    sess.send_read('K', 0.5, 0.5)
    sess.send_read(competitor + KEY_ENTER, 0.5, 2.0)


def tab_to_emit_field(sess):
    """Tab to the EME (emit card) field.
    The exact count depends on which fields are active for this competition data.
    Empirically determined: 9 Tabs from TRKE start field to EME."""
    TABS_TO_EME = 9
    for i in range(TABS_TO_EME):
        sess.send(KEY_TAB, 0.05)
        time.sleep(0.3)
        sess.read(0.5)
    # Extra wait to ensure EME field's inputstr is blocking and ready for input
    time.sleep(0.5)
    sess.read(0.5)
    return True


def edit_emit_field(sess, new_value):
    """Edit EME field: Delete old value, type new, accept with +"""
    # Delete existing characters
    for i in range(8):
        sess.send(KEY_DELETE, 0.1)
        time.sleep(0.1)
        sess.read(0.1)
    time.sleep(0.3)
    # Type new value
    for ch in new_value:
        sess.send(ch, 0.1)
        sess.read(0.1)
    time.sleep(0.3)
    # Accept with +
    sess.send_read('+', 0.5, 2.0)


def test_emit_change():
    print("=" * 60)
    print("TEST: Change emit card for competitor", COMPETITOR)
    print("=" * 60)

    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali not found. Run 'make' first.")
        return False

    tmpdir = setup_test_dir()
    kilpdat = os.path.join(tmpdir, 'KILP.DAT')
    original_md5 = hashlib.md5(open(kilpdat, 'rb').read()).hexdigest()
    print(f"\n1. Test dir: {tmpdir}")
    print(f"   KILP.DAT: {original_md5}")

    # Phase 1: Change emit
    print(f"\n2. Changing emit {OLD_EMIT} -> {NEW_EMIT}...")
    sess = HkMaaliSession(tmpdir)
    sess.start()
    try:
        navigate_to_korjaa(sess, COMPETITOR)
        found = tab_to_emit_field(sess)
        if not found:
            print("   FAIL: Could not find EME field")
            sess.stop()
            return False
        print("   Found EME field, editing...")
        edit_emit_field(sess, NEW_EMIT)
        print("   Edit complete.")
    finally:
        sess.stop()

    after_md5 = hashlib.md5(open(kilpdat, 'rb').read()).hexdigest()
    file_changed = after_md5 != original_md5
    print(f"   KILP.DAT {'modified' if file_changed else 'UNCHANGED'}")

    # Phase 2: Verify
    print(f"\n3. Restarting to verify...")
    sess2 = HkMaaliSession(tmpdir)
    sess2.start()
    startup_ok = True
    emit_found = False
    try:
        sess2.read(2.0)
        sess2.send_read(KEY_ENTER, 1.0, 1.0)
        text = sess2.output_text()
        if 'yhteensopivia' in text or 'DATA_ERR' in text:
            print("   FAIL: Startup error!")
            startup_ok = False
        else:
            print("   Startup OK")
            sess2.send_read('K', 0.5, 0.5)
            sess2.send_read('K', 0.5, 0.5)
            sess2.send_read(COMPETITOR + KEY_ENTER, 0.5, 2.0)
            text2 = sess2.output_text()
            if NEW_EMIT in text2:
                emit_found = True
                print(f"   Emit card: {NEW_EMIT} ✓")
            elif OLD_EMIT in text2:
                print(f"   Emit card: {OLD_EMIT} (unchanged)")
            else:
                print("   Could not verify emit value")
    finally:
        sess2.stop()

    if startup_ok and emit_found:
        shutil.rmtree(tmpdir)

    print(f"\n{'=' * 60}")
    if startup_ok and emit_found:
        print("RESULT: PASS ✓")
        return True
    else:
        print("RESULT: FAIL")
        return False


if __name__ == '__main__':
    success = test_emit_change()
    sys.exit(0 if success else 1)
