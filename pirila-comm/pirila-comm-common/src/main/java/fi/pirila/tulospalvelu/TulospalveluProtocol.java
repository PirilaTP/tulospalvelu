package fi.pirila.tulospalvelu;

/**
 * Tulospalvelu network protocol constants, binary utilities, and payload builders/parsers.
 * Shared between UDP and TCP transports.
 *
 * Protocol frame: SOH(1) + id(1) + iid(1) + pkgclass(1) + len(2 LE) + checksum(2 LE) + data(len)
 * See NETWORK_COMMUNICATION.md for full protocol documentation.
 */
public final class TulospalveluProtocol {

    private TulospalveluProtocol() {}

    // --- Control characters ---
    public static final byte SOH = 0x01;
    public static final byte STX = 0x02;
    public static final byte ETX = 0x03;
    public static final byte ACK = 0x06;
    public static final byte NAK = 0x15;

    // --- Message type constants (pkgclass) ---
    // From HkCom32.cpp:47-57
    public static final byte PKGCLASS_ALKUT = 0;       // Handshake
    public static final byte PKGCLASS_KILPT = 1;       // Full competitor record
    public static final byte PKGCLASS_KILPPVT = 2;     // Competitor stage data
    public static final byte PKGCLASS_VAIN_TULOST = 3; // Time result only
    public static final byte PKGCLASS_AIKAT = 4;       // Timer data
    public static final byte PKGCLASS_EMITT = 6;       // EMIT punch data
    public static final byte PKGCLASS_SEURAT = 7;      // Club data
    public static final byte PKGCLASS_FILESEND = 10;   // File transfer
    public static final byte PKGCLASS_EMITVA = 13;     // EMIT extended data
    public static final byte PKGCLASS_EXTRA = 14;      // Control commands

    // --- Sizes and offsets ---
    public static final int ALKUT_DATA_SIZE = 10;
    public static final int PV_OFF_BADGE = 68;     // Offset to badge (INT32) within pv/cpv data
    public static final int PV_OFF_KESKHYL = 128;  // Offset to keskhyl (WCHAR, UTF-16LE) within pv/cpv data
    public static final int KILPPVT_HEADER = 8;    // tarf(1)+pakota(1)+dk(2)+pv(2)+valuku(2)

    // --- Status (keskhyl) values ---
    // C++ set_tark accepts "-TIHKOEVPXMB"; the four below are the user-facing ones.
    public static final char STATUS_OPEN = '-';   // No status / pending
    public static final char STATUS_DNS  = 'E';   // Ei lähtenyt (Did Not Start)
    public static final char STATUS_DNF  = 'K';   // Keskeyttänyt (Did Not Finish)
    public static final char STATUS_DSQ  = 'H';   // Hylätty (Disqualified)

    // --- EXTRA sub-types (d1 & 0x0F) ---
    public static final int EXTRA_CHECKPOINT = 1;
    public static final int EXTRA_ACTIVATION = 2;
    public static final int EXTRA_ROUND_CHANGE = 3;
    public static final int EXTRA_ERA_START = 5;
    public static final int EXTRA_CURVINEN = 6;
    public static final int EXTRA_TEST = 7;
    public static final int EXTRA_SHUTDOWN = 9;

    // --- Binary utilities ---

    /** 16-bit checksum: sum of little-endian 16-bit words, matching C++ chksum() in TpComY32.cpp:604 */
    public static int checksum(byte[] data) {
        int sum = 0;
        int i;
        for (i = 0; i + 1 < data.length; i += 2) {
            sum += (data[i] & 0xFF) | ((data[i + 1] & 0xFF) << 8);
        }
        if (i < data.length) {
            sum += data[i] & 0xFF;
        }
        return sum & 0xFFFF;
    }

    public static void writeInt16LE(byte[] buf, int offset, short value) {
        buf[offset] = (byte) (value & 0xFF);
        buf[offset + 1] = (byte) ((value >> 8) & 0xFF);
    }

    public static void writeInt32LE(byte[] buf, int offset, int value) {
        buf[offset] = (byte) (value & 0xFF);
        buf[offset + 1] = (byte) ((value >> 8) & 0xFF);
        buf[offset + 2] = (byte) ((value >> 16) & 0xFF);
        buf[offset + 3] = (byte) ((value >> 24) & 0xFF);
    }

    public static short readInt16LE(byte[] buf, int offset) {
        return (short) ((buf[offset] & 0xFF) | ((buf[offset + 1] & 0xFF) << 8));
    }

    public static int readInt32LE(byte[] buf, int offset) {
        return (buf[offset] & 0xFF)
                | ((buf[offset + 1] & 0xFF) << 8)
                | ((buf[offset + 2] & 0xFF) << 16)
                | ((buf[offset + 3] & 0xFF) << 24);
    }

    // --- SOH frame parsing ---

    /** Parsed SOH protocol frame. */
    public record SohFrame(byte id, byte pkgclass, byte[] data) {}

    /**
     * Parse and validate an SOH frame from raw bytes.
     * Validates id+iid==255 and checksum. Returns null if invalid.
     */
    public static SohFrame parseSohFrame(byte[] raw, int offset) {
        if (raw.length < offset + 8) return null;
        if (raw[offset] != SOH) return null;

        byte id = raw[offset + 1];
        byte iid = raw[offset + 2];
        byte pkgclass = raw[offset + 3];
        int len = (raw[offset + 4] & 0xFF) | ((raw[offset + 5] & 0xFF) << 8);

        if (((id & 0xFF) + (iid & 0xFF)) != 255) return null;

        int dataStart = offset + 8;
        if (raw.length < dataStart + len) return null;

        byte[] data = new byte[len];
        System.arraycopy(raw, dataStart, data, 0, len);

        int expectedChk = (raw[offset + 6] & 0xFF) | ((raw[offset + 7] & 0xFF) << 8);
        if (checksum(data) != expectedChk) return null;

        return new SohFrame(id, pkgclass, data);
    }

    // --- ACK/NAK validation ---

    /** Parsed ACK response. */
    public record AckResponse(byte id) {}

    /** Parse ACK bytes: ACK(1)+id(1)+(255-id)(1)+id(1). Returns null if invalid. */
    public static AckResponse parseAck(byte[] raw, int offset) {
        if (raw.length < offset + 4) return null;
        if (raw[offset] != ACK) return null;
        byte id = raw[offset + 1];
        byte iid = raw[offset + 2];
        byte id2 = raw[offset + 3];
        if (((id & 0xFF) + (iid & 0xFF)) != 255 || id != id2) return null;
        return new AckResponse(id);
    }

    // --- Payload builders ---

    /** Build ALKUT handshake payload (10 bytes). */
    public static byte[] buildAlkutData(String machineId, int nrec) {
        byte[] data = new byte[ALKUT_DATA_SIZE];
        data[0] = 1; // tunn
        data[1] = (byte) machineId.charAt(0);
        data[2] = (byte) machineId.charAt(1);
        data[3] = 1; // vaihe
        writeInt16LE(data, 4, (short) nrec);
        // bytes 6-9: flags = 0
        return data;
    }

    /** Peer identity and competition state parsed from an ALKUT handshake payload. */
    public record AlkutInfo(String machineId, int vaihe, int nrec) {}

    /** Parse an ALKUT handshake payload. Returns null if it is too short. */
    public static AlkutInfo parseAlkutData(byte[] data) {
        if (data == null || data.length < ALKUT_DATA_SIZE) return null;
        String machineId = new String(new char[]{
                (char) (data[1] & 0xFF), (char) (data[2] & 0xFF)});
        int vaihe = data[3] & 0xFF;
        int nrec = readInt16LE(data, 4) & 0xFFFF;
        return new AlkutInfo(machineId, vaihe, nrec);
    }

    /** Build KILPPVT outbound payload for pv[0] with new badge. */
    public static byte[] buildKilppvtData(int recordIndex, byte[] pvData,
                                           int kilppvtpsize, int newBadge) {
        return buildKilppvtData(recordIndex, 0, pvData, kilppvtpsize, (Integer) newBadge, null);
    }

    /** Build KILPPVT outbound payload with badge and/or status (keskhyl) override for pv[0]. */
    public static byte[] buildKilppvtData(int recordIndex, byte[] pvData,
                                           int kilppvtpsize, Integer newBadge, Character newKeskhyl) {
        return buildKilppvtData(recordIndex, 0, pvData, kilppvtpsize, newBadge, newKeskhyl);
    }

    /**
     * Build KILPPVT outbound payload for a specific stage with optional badge/status override.
     * Either modifier may be null to leave that field untouched. The header's pv field controls
     * which stage on the receiver gets the update — multi-stage races require one message per stage.
     */
    public static byte[] buildKilppvtData(int recordIndex, int pvIndex, byte[] pvData,
                                           int kilppvtpsize, Integer newBadge, Character newKeskhyl) {
        byte[] cpv = new byte[kilppvtpsize];
        System.arraycopy(pvData, 0, cpv, 0, Math.min(pvData.length, kilppvtpsize));
        if (newBadge != null) {
            writeInt32LE(cpv, PV_OFF_BADGE, newBadge);
        }
        if (newKeskhyl != null) {
            cpv[PV_OFF_KESKHYL] = (byte) (newKeskhyl & 0xFF);
            cpv[PV_OFF_KESKHYL + 1] = (byte) ((newKeskhyl >> 8) & 0xFF);
        }

        byte[] data = new byte[8 + kilppvtpsize];
        data[0] = 0;                                   // tarf
        data[1] = 1;                                   // pakota = force
        writeInt16LE(data, 2, (short) recordIndex);    // dk
        writeInt16LE(data, 4, (short) pvIndex);        // pv = stage index
        writeInt16LE(data, 6, (short) 0);              // valuku = 0
        System.arraycopy(cpv, 0, data, 8, kilppvtpsize);
        return data;
    }

    /**
     * Build KILPT outbound payload (full base record).
     * Header: tarf(1) + pakota(1) + dk(2) + entno(2) = 6 bytes; followed by ckilp.
     * entno = the OLD kilpno (passed unchanged for in-place edits; the C++ tark_kilp
     * branches on entno==kilp.kilpno to mean "same competitor, just save").
     */
    public static byte[] buildKilptData(int recordIndex, int entno, byte[] recordData,
                                         int kilprecsize0) {
        byte[] ckilp = new byte[kilprecsize0];
        System.arraycopy(recordData, 0, ckilp, 0, Math.min(recordData.length, kilprecsize0));

        byte[] data = new byte[6 + kilprecsize0];
        data[0] = 0;                                   // tarf
        data[1] = 1;                                   // pakota
        writeInt16LE(data, 2, (short) recordIndex);    // dk
        writeInt16LE(data, 4, (short) entno);          // entno (old kilpno)
        System.arraycopy(ckilp, 0, data, 6, kilprecsize0);
        return data;
    }

    /**
     * Build VAIN_TULOST outbound payload (24 bytes). Used to push a single
     * time result (or clear one with time=0) to peers without sending the
     * full pv block. Header layout matches the C++ d.v struct:
     *   tarf(1) + pakota(1) + dk(2) + bib(2) + k_pv(2) + vali(2) + aika(4)
     * plus 10 bytes padding to reach the C++ class_len[VAIN_TULOST] = 24
     * (sizeof(INT32) + 20 — the trailing space carries optional daika/iaika
     * which the receiver doesn't read for splitIndex≤valuku+1).
     */
    public static byte[] buildVainTulostData(int dk, int bib, int stage, int splitIndex, int time) {
        byte[] data = new byte[24];
        data[0] = 0;                                 // tarf
        data[1] = 1;                                 // pakota
        writeInt16LE(data, 2, (short) dk);
        writeInt16LE(data, 4, (short) bib);
        writeInt16LE(data, 6, (short) stage);
        writeInt16LE(data, 8, (short) splitIndex);
        writeInt32LE(data, 10, time);
        // bytes 14..23 left as zero (daika/iaika placeholders)
        return data;
    }

    /**
     * Write a UTF-16LE wide string into a fixed-size slot in a byte buffer,
     * NUL-padded to maxChars wchars (= maxChars * 2 bytes). Truncates if longer.
     * Used to update WTEXT fields (sukunimi, etunimi, seura, ...) inside a
     * record buffer before sending KILPT.
     */
    public static void writeWideString(byte[] buf, int offset, int maxChars, String s) {
        if (s == null) s = "";
        // Zero out the slot first so old residue gets cleared on shrink.
        for (int i = 0; i < maxChars * 2; i++) buf[offset + i] = 0;
        int n = Math.min(s.length(), maxChars);
        for (int i = 0; i < n; i++) {
            int c = s.charAt(i);
            buf[offset + i * 2]     = (byte) (c & 0xFF);
            buf[offset + i * 2 + 1] = (byte) ((c >> 8) & 0xFF);
        }
    }

    // --- Payload parsers ---

    /** Parsed KILPPVT (competitor stage) payload. */
    public record KilppvtPayload(int dk, int pv, int valuku, byte[] cpvData) {}

    /** Parse incoming KILPPVT data. Returns null if too short. */
    public static KilppvtPayload parseKilppvtData(byte[] data) {
        if (data.length < KILPPVT_HEADER) return null;
        int dk = readInt16LE(data, 2) & 0xFFFF;
        int pv = readInt16LE(data, 4) & 0xFFFF;
        int valuku = readInt16LE(data, 6) & 0xFFFF;
        byte[] cpv = new byte[data.length - KILPPVT_HEADER];
        System.arraycopy(data, KILPPVT_HEADER, cpv, 0, cpv.length);
        return new KilppvtPayload(dk, pv, valuku, cpv);
    }

    /** Parsed KILPT (full competitor record) payload. */
    public record KilptPayload(int dk, int entno, byte[] recordData) {}

    /** Parse incoming KILPT data: tarf(1)+pakota(1)+dk(2)+entno(2)+ckilp(N). */
    public static KilptPayload parseKilptData(byte[] data) {
        if (data.length < 6) return null;
        int dk = readInt16LE(data, 2) & 0xFFFF;
        int entno = readInt16LE(data, 4) & 0xFFFF;
        byte[] recordData = new byte[data.length - 6];
        System.arraycopy(data, 6, recordData, 0, recordData.length);
        return new KilptPayload(dk, entno, recordData);
    }

    /** Parsed VAIN_TULOST (time result) payload. */
    public record VainTulostPayload(int dk, int bib, int stage, int splitIndex, int time) {}

    /**
     * Parse incoming VAIN_TULOST data:
     * tarf(1)+pakota(1)+dk(2)+bib(2)+k_pv(2)+vali(2)+aika(4)
     */
    public static VainTulostPayload parseVainTulostData(byte[] data) {
        if (data.length < 12) return null;
        int dk = readInt16LE(data, 2) & 0xFFFF;
        int bib = readInt16LE(data, 4) & 0xFFFF;
        int stage = readInt16LE(data, 6) & 0xFFFF;
        int splitIndex = readInt16LE(data, 8);  // signed: -1=start, 0=finish, >0=split
        int time = readInt32LE(data, 10);
        return new VainTulostPayload(dk, bib, stage, splitIndex, time);
    }

    /** Parsed EXTRA command payload. */
    public record ExtraPayload(int d1, int d2, int d3, int d4) {
        /** Sub-type is low nibble of d1 */
        public int subType() { return d1 & 0x0F; }
    }

    /** Parse incoming EXTRA data: d1(4)+d2(4)+d3(4)+d4(4). */
    public static ExtraPayload parseExtraData(byte[] data) {
        if (data.length < 16) return null;
        return new ExtraPayload(
                readInt32LE(data, 0),
                readInt32LE(data, 4),
                readInt32LE(data, 8),
                readInt32LE(data, 12));
    }
}
