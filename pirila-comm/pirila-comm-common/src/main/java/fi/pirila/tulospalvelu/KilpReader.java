package fi.pirila.tulospalvelu;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

/**
 * Reads competitor data from tulospalvelu KILP.DAT file.
 *
 * KILP.DAT is a "dbbox" random-access binary file with:
 * - Record 0: header (firstfree, numberfree, int1, int2 compressed as 4 x UINT16)
 * - Records 1..N: competitor data
 * - numrec = filesize / reclen (includes header)
 *
 * Record layout (all fields little-endian, #pragma pack(1)):
 *   Base part (kilprecsize0 bytes):
 *     kilpstatus(INT16,0) kilpno(INT16,2) lisno(INT32x2,4) wrkoodi(WTEXT,12)
 *     ilmlista(INT16,32) piiri(INT16,34) piste(INT32x3,36)
 *     sukunimi(WTEXT,48) etunimi(WTEXT,98) arvo(WTEXT,148)
 *     seura(WTEXT,180) seuralyh(WTEXT,244) yhdistys(WTEXT,276)
 *     joukkue(WTEXT,308) maa(WTEXT,340) sarja(INT16,348)
 *     sukup(INT16,350) ikasarja(INT16,352) alisarja(INT16,354)
 *     synt(INT16,356) arvryhma(INT16,358)
 *
 *   Stage data (n_pv x kilppvtpsize bytes each):
 *     txt(WTEXT,0) uusi(CHAR,20) vac(CHAR,21) flags(INT32x2,22)
 *     tav(INT32,30) enn(INT32,34) rata(WTEXT,38) sarja(INT16,62)
 *     era(INT16,64) bib(INT16,66) badge(INT32x2,68) laina(WCHARx2,76)
 *     selitys(WTEXT,80) pvpisteet(INT32x2,116) tlahto(INT32,124)
 *     keskhyl(WCHAR,128) ampsakot(WTEXT,130) tasoitus(INT32,142)
 *     sakko(INT32,146) ysija(INT16,150) va(vatp[],154)
 */
public class KilpReader {

    // Default field offsets in base record (kilp_fields)
    private static final int OFF_KILPSTATUS = 0;
    private static final int OFF_KILPNO = 2;
    private static final int OFF_SUKUNIMI = 48;
    private static final int OFF_ETUNIMI = 98;
    private static final int OFF_SEURA = 180;
    private static final int OFF_SARJA = 348;

    // Field offsets within pv (stage) block (pv_fields)
    private static final int PV_OFF_BADGE = 68;    // INT32[2]
    private static final int PV_OFF_BIB = 66;      // INT16
    private static final int PV_OFF_TLAHTO = 124;    // INT32 start time (clock, 1/100s)
    private static final int PV_OFF_KESKHYL = 128;   // WCHAR (2 bytes)
    private static final int PV_OFF_VA = 152;         // vatp[] array, 8 bytes each
    // vatp[0] = start time (= tlahto), vatp[1] = finish time (clock, 1/100s)
    // vatp[n].val2 = position/order number

    /**
     * Read numrec (total record count including header) from KILP.DAT header.
     * This value must be sent in the ALKUT handshake message.
     */
    public static int readNumrec(Path kilpFile) throws IOException {
        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "r")) {
            byte[] header = new byte[8];
            raf.readFully(header);
            ByteBuffer hdr = ByteBuffer.wrap(header).order(ByteOrder.LITTLE_ENDIAN);
            return hdr.getShort(6) & 0xFFFF;
        }
    }

    /** kilppvtpsize for the current file (set by detectRecordSize). */
    private static int lastKilppvtpsize = 248;

    /** Get kilppvtpsize (full stage record size incl. intermediate times). */
    public static int getKilppvtpsize() {
        return lastKilppvtpsize;
    }

    /**
     * Read the raw pv[0] data for a competitor record.
     * Returns kilppvtpsize bytes starting at offset kilprecsize0 within the record.
     */
    public static byte[] readPvData(Path kilpFile, int recordIndex) throws IOException {
        int reclen = detectRecordSize(kilpFile);
        int kilprecsize0 = findKilprecsize0(reclen);
        int kilppvtpsize = reclen > kilprecsize0 ? (reclen - kilprecsize0) / Math.max(1, (reclen - kilprecsize0) / 248) : 248;
        // Simple: pv[0] starts at kilprecsize0, take the rest up to reclen
        // But we need exactly kilppvtpsize bytes for pv[0]
        kilppvtpsize = lastKilppvtpsize;
        byte[] pvData = new byte[kilppvtpsize];
        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "r")) {
            long offset = (long) recordIndex * reclen + kilprecsize0;
            raf.seek(offset);
            int toRead = Math.min(kilppvtpsize, reclen - kilprecsize0);
            raf.readFully(pvData, 0, toRead);
        }
        return pvData;
    }

    /**
     * Write badge value to KILP.DAT for a specific competitor record.
     * Updates pv[0].badge (INT32 LE at offset kilprecsize0 + 68).
     */
    public static void writeBadge(Path kilpFile, int recordIndex, int badge) throws IOException {
        int reclen = detectRecordSize(kilpFile);
        int kilprecsize0 = findKilprecsize0(reclen);
        long offset = (long) recordIndex * reclen + kilprecsize0 + PV_OFF_BADGE;

        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "rw")) {
            raf.seek(offset);
            writeInt32LEToFile(raf, badge);
        }
    }

    /**
     * Write keskhyl (status, wchar_t UTF-16LE) to KILP.DAT for a specific competitor record.
     * Updates pv[0].keskhyl at offset kilprecsize0 + 128.
     */
    public static void writeKeskhyl(Path kilpFile, int recordIndex, char keskhyl) throws IOException {
        int reclen = detectRecordSize(kilpFile);
        int kilprecsize0 = findKilprecsize0(reclen);
        long offset = (long) recordIndex * reclen + kilprecsize0 + PV_OFF_KESKHYL;

        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "rw")) {
            raf.seek(offset);
            raf.writeByte(keskhyl & 0xFF);
            raf.writeByte((keskhyl >> 8) & 0xFF);
        }
    }

    /**
     * Write full stage (pv) data to KILP.DAT for a competitor.
     * Used when receiving KILPPVT messages from the network.
     * Corresponds to C++ tark_kilp(cn, 2) → pv[n].unpack0(cpv) → tallenna().
     *
     * @param kilpFile path to KILP.DAT
     * @param recordIndex 1-based record index (dk)
     * @param pvIndex stage number (0-based)
     * @param pvData raw packed stage data (kilppvtpsize bytes)
     */
    public static void writePvData(Path kilpFile, int recordIndex, int pvIndex, byte[] pvData) throws IOException {
        int reclen = detectRecordSize(kilpFile);
        int kilprecsize0 = findKilprecsize0(reclen);
        int kilppvtpsize = lastKilppvtpsize;
        long offset = (long) recordIndex * reclen + kilprecsize0 + (long) pvIndex * kilppvtpsize;
        int toWrite = Math.min(pvData.length, kilppvtpsize);

        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "rw")) {
            raf.seek(offset);
            raf.write(pvData, 0, toWrite);
        }
    }

    /**
     * Write full base record to KILP.DAT for a competitor.
     * Used when receiving KILPT messages from the network.
     * Corresponds to C++ tark_kilp(cn, 1) → unpack0(ckilp) → tallenna().
     *
     * @param kilpFile path to KILP.DAT
     * @param recordIndex 1-based record index (dk)
     * @param recordData raw packed base record data (kilprecsize0 bytes)
     */
    public static void writeFullRecord(Path kilpFile, int recordIndex, byte[] recordData) throws IOException {
        int reclen = detectRecordSize(kilpFile);
        int kilprecsize0 = findKilprecsize0(reclen);
        long offset = (long) recordIndex * reclen;
        int toWrite = Math.min(recordData.length, kilprecsize0);

        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "rw")) {
            raf.seek(offset);
            raf.write(recordData, 0, toWrite);
        }
    }

    /**
     * Write a single time result to KILP.DAT.
     * Used when receiving VAIN_TULOST messages from the network.
     * Corresponds to C++ tark_kilp(cn, 0) → tall_tulos(vali, aika) → tallenna().
     *
     * The time is stored in the vatp (split time) array within pv[currentStage].
     * Split index mapping: -1=start time (tlahto at offset 124),
     * 0=finish time (first vatp slot), >0=split times.
     *
     * @param kilpFile path to KILP.DAT
     * @param recordIndex 1-based record index (dk)
     * @param pvIndex stage number (0-based, typically k_pv)
     * @param splitIndex split point (-1=start, 0=finish, >0=split)
     * @param time time value in 1/100s
     */
    public static void writeTimeResult(Path kilpFile, int recordIndex, int pvIndex,
                                        int splitIndex, int time) throws IOException {
        int reclen = detectRecordSize(kilpFile);
        int kilprecsize0 = findKilprecsize0(reclen);
        int kilppvtpsize = lastKilppvtpsize;
        long pvBase = (long) recordIndex * reclen + kilprecsize0 + (long) pvIndex * kilppvtpsize;

        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "rw")) {
            if (splitIndex == -1) {
                // Start time: tlahto at offset 124 within pv
                raf.seek(pvBase + 124);
            } else {
                // vatp array starts at offset 152 within pv, each entry is 8 bytes
                // splitIndex 0 = finish (first slot), >0 = intermediate times
                raf.seek(pvBase + 152 + (long) splitIndex * 8);
            }
            writeInt32LEToFile(raf, time);
        }
    }

    private static void writeInt32LEToFile(RandomAccessFile raf, int value) throws IOException {
        raf.writeByte(value & 0xFF);
        raf.writeByte((value >> 8) & 0xFF);
        raf.writeByte((value >> 16) & 0xFF);
        raf.writeByte((value >> 24) & 0xFF);
    }

    /**
     * Auto-detect record size from KILP.DAT file.
     * Tries common configurations based on KilpSrj.xml FileFormat parameters.
     * Falls back to scanning for valid record boundaries.
     */
    public static int detectRecordSize(Path kilpFile) throws IOException {
        long fileSize = java.nio.file.Files.size(kilpFile);

        // Try to read int2 from header (= numrec)
        // Header: 4 x UINT16 (compressed from UINT32)
        // short[3] = int2 = numrec
        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "r")) {
            byte[] header = new byte[8];
            raf.readFully(header);
            ByteBuffer hdr = ByteBuffer.wrap(header).order(ByteOrder.LITTLE_ENDIAN);
            int numrec = hdr.getShort(6) & 0xFFFF; // short[3] = int2

            if (numrec > 0 && fileSize % numrec == 0) {
                int reclen = (int) (fileSize / numrec);
                if (reclen >= 360 && reclen <= 4000) {
                    int k0 = findKilprecsize0(reclen);
                    int pvTotal = reclen - k0;
                    // Detect n_pv and kilppvtpsize.
                    // kilppvtpsize = kilppvtpsize0 + (valuku+2)*vatpsize
                    // kilppvtpsize0 = 152, vatpsize = 8, so (size-152) must be divisible by 8
                    for (int npv = 3; npv >= 1; npv--) {
                        if (pvTotal % npv == 0) {
                            int candidate = pvTotal / npv;
                            if (candidate >= 152 && (candidate - 152) % 8 == 0) {
                                lastKilppvtpsize = candidate;
                                break;
                            }
                        }
                    }
                    return reclen;
                }
            }
        }

        // Fallback: try common sizes
        int[] commonSizes = {856, 642, 610, 860, 514};
        for (int size : commonSizes) {
            if (fileSize % size == 0 && fileSize / size >= 2) {
                return size;
            }
        }

        throw new IOException("Cannot detect KILP.DAT record size for file of " + fileSize + " bytes");
    }

    private static long Files_size(Path p) throws IOException {
        return java.nio.file.Files.size(p);
    }

    public static List<Competitor> read(Path kilpFile) throws IOException {
        long fileSize = Files_size(kilpFile);
        int reclen = detectRecordSize(kilpFile);
        int numrec = (int) (fileSize / reclen);

        // kilprecsize0: base record without pv data
        // We need to find where pv[0] starts. Use default 360 but verify.
        // Actual kilprecsize0 = reclen - n_pv * kilppvtpsize
        // For the demo file: 856 - 2*250 = 356
        int kilprecsize0 = findKilprecsize0(reclen);

        List<Competitor> competitors = new ArrayList<>();

        try (RandomAccessFile raf = new RandomAccessFile(kilpFile.toFile(), "r")) {
            byte[] record = new byte[reclen];

            // Skip record 0 (header), read records 1..numrec-1
            for (int i = 1; i < numrec; i++) {
                raf.seek((long) i * reclen);
                raf.readFully(record);
                ByteBuffer buf = ByteBuffer.wrap(record).order(ByteOrder.LITTLE_ENDIAN);

                int kilpno = buf.getShort(OFF_KILPNO) & 0xFFFF;
                if (kilpno == 0 || kilpno == 0xFFFF) continue; // empty/deleted

                String sukunimi = readWideString(record, OFF_SUKUNIMI, 25);
                String etunimi = readWideString(record, OFF_ETUNIMI, 25);
                String seura = readWideString(record, OFF_SEURA, 32);
                int sarja = buf.getShort(OFF_SARJA) & 0xFFFF;

                // Fields from pv[0] (first stage)
                int badge = 0;
                int badge2 = 0;
                char keskhyl = 0;
                int ysija = 0;
                int resultTime = 0;
                if (kilprecsize0 + PV_OFF_BADGE + 8 <= reclen) {
                    badge = buf.getInt(kilprecsize0 + PV_OFF_BADGE);
                    badge2 = buf.getInt(kilprecsize0 + PV_OFF_BADGE + 4);
                }
                if (kilprecsize0 + PV_OFF_KESKHYL + 2 <= reclen) {
                    keskhyl = (char) (buf.getShort(kilprecsize0 + PV_OFF_KESKHYL) & 0xFFFF);
                }
                // vatp[1] = result: {time_ms, position}
                // time is result time in milliseconds, val2 = position (sija)
                if (kilprecsize0 + PV_OFF_VA + 16 <= reclen) {
                    int resultMs = buf.getInt(kilprecsize0 + PV_OFF_VA + 8);  // vatp[1].time
                    ysija = buf.getInt(kilprecsize0 + PV_OFF_VA + 12);        // vatp[1].val2
                    if (resultMs > 0) {
                        resultTime = resultMs;
                    }
                }

                competitors.add(new Competitor(i, kilpno, sukunimi, etunimi, seura, sarja, badge, badge2, keskhyl, ysija, resultTime));
            }
        }

        return competitors;
    }

    /**
     * Return the base record size (without pv/stage data).
     * This is 360 bytes with default field lengths (LSNIMI=24, LENIMI=24, etc.)
     * which matches the standard kilp_fields layout.
     */
    private static int findKilprecsize0(int reclen) {
        return 360;
    }

    /**
     * Parsed core fields from a KILPT record (kilprecsize0 base block).
     * Use to merge an incoming server record into an existing in-memory Competitor.
     */
    public record ParsedRecord(int kilpno, String sukunimi, String etunimi,
                                String seura, int sarja) {}

    /**
     * Parse name/club/class/kilpno fields from a raw KILPT record byte buffer.
     * Offsets follow kilp_fields in HkDat.cpp.
     */
    public static ParsedRecord parseRecord(byte[] recordData) {
        java.nio.ByteBuffer buf = java.nio.ByteBuffer.wrap(recordData).order(java.nio.ByteOrder.LITTLE_ENDIAN);
        int kilpno = buf.getShort(OFF_KILPNO) & 0xFFFF;
        String sukunimi = readWideString(recordData, OFF_SUKUNIMI, 25);
        String etunimi = readWideString(recordData, OFF_ETUNIMI, 25);
        String seura = readWideString(recordData, OFF_SEURA, 32);
        int sarja = buf.getShort(OFF_SARJA) & 0xFFFF;
        return new ParsedRecord(kilpno, sukunimi, etunimi, seura, sarja);
    }

    /** Read a null-terminated UTF-16LE (wchar_t) string from a byte array. */
    private static String readWideString(byte[] data, int offset, int maxChars) {
        int end = Math.min(offset + maxChars * 2, data.length);
        StringBuilder sb = new StringBuilder();
        for (int i = offset; i + 1 < end; i += 2) {
            int ch = (data[i] & 0xFF) | ((data[i + 1] & 0xFF) << 8);
            if (ch == 0) break;
            sb.append((char) ch);
        }
        return sb.toString().trim();
    }

    /** Dump all competitors to stdout for debugging. */
    public static void main(String[] args) throws IOException {
        Path kilpFile;
        if (args.length > 0) {
            kilpFile = Path.of(args[0]);
        } else {
            kilpFile = Path.of("kisat/HkMaaliData/KILP.DAT");
        }

        int reclen = detectRecordSize(kilpFile);
        int kilprecsize0 = findKilprecsize0(reclen);
        System.out.println("KILP.DAT: reclen=" + reclen + ", kilprecsize0=" + kilprecsize0);

        List<Competitor> competitors = read(kilpFile);
        System.out.println("Competitors: " + competitors.size());
        System.out.println();
        System.out.printf("%4s  %-20s %-15s %-20s %s%n", "No", "Sukunimi", "Etunimi", "Seura", "Emit");
        System.out.println("-".repeat(75));
        for (Competitor c : competitors) {
            System.out.println(c);
        }
    }
}
