///usr/bin/env jbang "$0" "$@" ; exit $?
//JAVA 21
//SOURCES Harness.java
//DEPS fi.pirila:pirila-comm-common:1.0.0-SNAPSHOT
//DEPS org.jetbrains.pty4j:pty4j:0.13.12
//DEPS com.microsoft.playwright:playwright:1.58.0

/**
 * One-shot utility: rewrite the PII-bearing UTF-16LE wide-char fields in a
 * KILP.DAT in place so the committed test fixture cannot be tied to real
 * orienteers. Idempotent — running twice produces the same output.
 *
 * Names are derived deterministically from kilpno so that:
 *  - tests that search by kilpno (e.g. webadmin's CardChangeView lookup)
 *    still find exactly one competitor;
 *  - the kilpno's decimal string is still NOT a substring of any name/club
 *    (otherwise the search would yield multiple matches).
 *
 * Fields cleared to empty: arvo, seuralyh, yhdistys, joukkue, maa, wrkoodi.
 * Field set to 0:           synt (birth year — PII).
 * sukunimi/etunimi:         "Sukunimi <kilpno>" / "Etunimi <kilpno>".
 * seura:                    "Seura A".."Seura Z" bucketed from the original
 *                           seura's first letter — preserves grouping size
 *                           without revealing the original club.
 *
 * Usage:  jbang AnonymizeKilpDat.java <path/to/KILP.DAT>
 */

import fi.pirila.tulospalvelu.KilpReader;

import java.io.RandomAccessFile;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

import static java.lang.System.out;

public class AnonymizeKilpDat {

    static final int OFF_KILPNO    = 2;
    static final int OFF_WRKOODI   = 12;   // 10 wchars  (20 B)
    static final int OFF_SUKUNIMI  = 48;   // 25 wchars  (50 B)
    static final int OFF_ETUNIMI   = 98;   // 25 wchars  (50 B)
    static final int OFF_ARVO      = 148;  // 16 wchars  (32 B)
    static final int OFF_SEURA     = 180;  // 32 wchars  (64 B)
    static final int OFF_SEURALYH  = 244;  // 16 wchars  (32 B)
    static final int OFF_YHDISTYS  = 276;  // 16 wchars  (32 B)
    static final int OFF_JOUKKUE   = 308;  // 16 wchars  (32 B)
    static final int OFF_MAA       = 340;  //  4 wchars  ( 8 B)
    static final int OFF_SYNT      = 356;  // INT16

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            out.println("usage: jbang AnonymizeKilpDat.java <KILP.DAT>");
            System.exit(2);
        }
        Path file = Path.of(args[0]);
        if (!Files.exists(file)) {
            out.println("file not found: " + file);
            System.exit(2);
        }

        int reclen = KilpReader.detectRecordSize(file);
        long size = Files.size(file);
        int numrec = (int) (size / reclen);
        out.printf("File: %s  (%d bytes, reclen=%d, %d records)%n",
                file, size, reclen, numrec);

        int touched = 0;
        try (RandomAccessFile raf = new RandomAccessFile(file.toFile(), "rw")) {
            for (int i = 1; i < numrec; i++) {
                long base = (long) i * reclen;
                raf.seek(base + OFF_KILPNO);
                int kilpno = readU16LE(raf);
                if (kilpno == 0 || kilpno == 0xFFFF) continue;

                // Read original seura's first character to bucket-group clubs.
                raf.seek(base + OFF_SEURA);
                byte[] seuraBuf = new byte[64];
                raf.readFully(seuraBuf);
                char firstSeura = readFirstChar(seuraBuf);
                String seuraBucket = firstSeura == 0 ? "Z" : String.valueOf(
                        Character.toUpperCase(firstSeura));

                writeWide(raf, base + OFF_SUKUNIMI, 25, "Sukunimi " + kilpno);
                writeWide(raf, base + OFF_ETUNIMI, 25, "Etunimi " + kilpno);
                writeWide(raf, base + OFF_SEURA, 32, "Seura " + seuraBucket);
                writeWide(raf, base + OFF_SEURALYH, 16, "S-" + seuraBucket);
                writeWide(raf, base + OFF_ARVO, 16, "");
                writeWide(raf, base + OFF_YHDISTYS, 16, "");
                writeWide(raf, base + OFF_JOUKKUE, 16, "");
                writeWide(raf, base + OFF_WRKOODI, 10, "");
                writeWide(raf, base + OFF_MAA, 4, "");

                raf.seek(base + OFF_SYNT);
                writeI16LE(raf, 0);

                touched++;
            }
        }
        out.printf("Anonymised %d competitor records.%n", touched);
    }

    private static int readU16LE(RandomAccessFile raf) throws java.io.IOException {
        return raf.readUnsignedByte() | (raf.readUnsignedByte() << 8);
    }

    private static void writeI16LE(RandomAccessFile raf, int v) throws java.io.IOException {
        raf.writeByte(v & 0xFF);
        raf.writeByte((v >> 8) & 0xFF);
    }

    private static char readFirstChar(byte[] wideBuf) {
        int c = (wideBuf[0] & 0xFF) | ((wideBuf[1] & 0xFF) << 8);
        return (char) c;
    }

    /** Write a UTF-16LE wide string into a fixed-size slot, NUL-padded to len. */
    private static void writeWide(RandomAccessFile raf, long offset, int wcharCount, String s)
            throws java.io.IOException {
        // Truncate to fit
        if (s.length() > wcharCount) s = s.substring(0, wcharCount);
        byte[] out = new byte[wcharCount * 2];
        for (int i = 0; i < s.length(); i++) {
            int c = s.charAt(i);
            out[i * 2]     = (byte) (c & 0xFF);
            out[i * 2 + 1] = (byte) ((c >> 8) & 0xFF);
        }
        raf.seek(offset);
        raf.write(out);
    }
}
