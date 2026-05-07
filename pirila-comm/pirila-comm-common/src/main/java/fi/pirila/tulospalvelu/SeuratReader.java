package fi.pirila.tulospalvelu;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

/**
 * Parser for seurat.csv: one club per line, ISO-8859-1, semicolon-separated
 * <code>piiri;lyhenne;nimi</code>. Lines that don't have at least three
 * columns are skipped silently; an empty file just returns an empty list.
 *
 * Example row: <code>14;AOK;Akilles OK</code>
 */
public final class SeuratReader {

    private SeuratReader() {}

    public static List<Seura> read(Path csv) throws IOException {
        if (!Files.exists(csv)) return List.of();
        List<Seura> out = new ArrayList<>();
        for (String line : Files.readAllLines(csv, Charset.forName("ISO-8859-1"))) {
            if (line.isBlank()) continue;
            String[] parts = line.split(";", -1);
            if (parts.length < 3) continue;
            int piiri;
            try {
                piiri = Integer.parseInt(parts[0].trim());
            } catch (NumberFormatException e) {
                continue;
            }
            String lyhenne = parts[1].trim();
            String nimi = parts[2].trim();
            if (nimi.isEmpty()) continue;
            out.add(new Seura(piiri, lyhenne, nimi));
        }
        return out;
    }
}
