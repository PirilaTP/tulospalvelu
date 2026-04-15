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
    private static final int PV_OFF_BADGE = 68;   // INT32[2]
    private static final int PV_OFF_BIB = 66;     // INT16

    public static class Competitor {
        public final int recordIndex;
        public final int kilpno;
        public final String sukunimi;
        public final String etunimi;
        public final String seura;
        public final int sarja;
        public int badge;   // emit card from pv[0]
        public int badge2;  // second emit card from pv[0]

        Competitor(int recordIndex, int kilpno, String sukunimi, String etunimi,
                   String seura, int sarja, int badge, int badge2) {
            this.recordIndex = recordIndex;
            this.kilpno = kilpno;
            this.sukunimi = sukunimi;
            this.etunimi = etunimi;
            this.seura = seura;
            this.sarja = sarja;
            this.badge = badge;
            this.badge2 = badge2;
        }

        @Override
        public String toString() {
            String badgeStr = badge > 0 ? String.valueOf(badge) : "-";
            return String.format("%4d  %-20s %-15s %-20s emit:%s",
                    kilpno, sukunimi, etunimi, seura, badgeStr);
        }
    }

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
            // Write INT32 little-endian
            raf.writeByte(badge & 0xFF);
            raf.writeByte((badge >> 8) & 0xFF);
            raf.writeByte((badge >> 16) & 0xFF);
            raf.writeByte((badge >> 24) & 0xFF);
        }
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

                // Badge from pv[0] (first stage)
                int badge = 0;
                int badge2 = 0;
                if (kilprecsize0 + PV_OFF_BADGE + 8 <= reclen) {
                    badge = buf.getInt(kilprecsize0 + PV_OFF_BADGE);
                    badge2 = buf.getInt(kilprecsize0 + PV_OFF_BADGE + 4);
                }

                competitors.add(new Competitor(i, kilpno, sukunimi, etunimi, seura, sarja, badge, badge2));
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
