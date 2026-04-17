package fi.pirila.tulospalvelu;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Reads class/series names from KilpSrj.xml.
 * Maps ClassNo (1-based) to ClassId (e.g. "H18", "D21A").
 * KILP.DAT sarja field is 0-based, so sarja + 1 = ClassNo.
 */
public class KilpSrjReader {

    private static final Pattern CLASS_NO = Pattern.compile("<Class\\s+ClassNo=\"(\\d+)\"");
    private static final Pattern CLASS_ID = Pattern.compile("<ClassId>([^<]+)</ClassId>");

    private final Map<Integer, String> classNames = new HashMap<>();

    public void read(Path xmlFile) throws IOException {
        String content = Files.readString(xmlFile);
        Matcher classMatcher = CLASS_NO.matcher(content);
        while (classMatcher.find()) {
            int classNo = Integer.parseInt(classMatcher.group(1));
            int searchFrom = classMatcher.end();
            Matcher idMatcher = CLASS_ID.matcher(content);
            if (idMatcher.find(searchFrom)) {
                classNames.put(classNo, idMatcher.group(1).trim());
            }
        }
    }

    /**
     * Get class name for a 0-based sarja index from KILP.DAT.
     */
    public String getClassName(int sarja) {
        return classNames.getOrDefault(sarja + 1, String.valueOf(sarja));
    }

    public Map<Integer, String> getClassNames() {
        return classNames;
    }
}
