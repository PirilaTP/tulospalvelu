package fi.pirila.tulospalvelu;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Reads class/series info from KilpSrj.xml.
 *
 * Maps ClassNo (1-based) to ClassId (e.g. "H18", "D21A", "VAKANTIT").
 * KILP.DAT sarja field is 0-based, so sarja + 1 = ClassNo.
 *
 * Also tracks the &lt;VacantClass&gt;Yes&lt;/VacantClass&gt; flag so callers
 * can hide placeholder entries from default views.
 */
public class KilpSrjReader {

    private static final Pattern CLASS_BLOCK = Pattern.compile(
            "<Class\\s+ClassNo=\"(\\d+)\"[\\s\\S]*?</Class>");
    private static final Pattern CLASS_ID = Pattern.compile(
            "<ClassId>([^<]+)</ClassId>");
    private static final Pattern VACANT_CLASS = Pattern.compile(
            "<VacantClass>\\s*Yes\\s*</VacantClass>");

    private final Map<Integer, String> classNames = new LinkedHashMap<>();
    private final Map<Integer, Boolean> vacantFlags = new HashMap<>();

    public void read(Path xmlFile) throws IOException {
        String content = Files.readString(xmlFile);
        Matcher block = CLASS_BLOCK.matcher(content);
        while (block.find()) {
            int classNo = Integer.parseInt(block.group(1));
            String body = block.group();
            Matcher id = CLASS_ID.matcher(body);
            if (id.find()) {
                classNames.put(classNo, id.group(1).trim());
            }
            vacantFlags.put(classNo, VACANT_CLASS.matcher(body).find());
        }
    }

    /** Class name for a 0-based KILP.DAT sarja index. */
    public String getClassName(int sarja) {
        return classNames.getOrDefault(sarja + 1, String.valueOf(sarja));
    }

    /** True if the class at the given 0-based sarja is marked &lt;VacantClass&gt;Yes&lt;/VacantClass&gt;. */
    public boolean isVacantClass(int sarja) {
        return vacantFlags.getOrDefault(sarja + 1, false);
    }

    public Map<Integer, String> getClassNames() {
        return classNames;
    }

    /** sarja (0-based) → class name, in the order classes appear in the xml. */
    public Map<Integer, String> getAllClasses() {
        Map<Integer, String> r = new LinkedHashMap<>();
        for (var e : classNames.entrySet()) {
            r.put(e.getKey() - 1, e.getValue());
        }
        return r;
    }
}
