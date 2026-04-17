package fi.pirila.tulospalvelu;

/**
 * Callback interface for incoming Tulospalvelu protocol messages.
 * All methods have default empty implementations so listeners only
 * need to override the message types they care about.
 *
 * Message types that affect KILP.DAT (C++ ref: HkCom32.cpp tark_kilp):
 * - KILPPVT: stage data update (most common during competition)
 * - KILPT: full competitor record (initial sync, base data changes)
 * - VAIN_TULOST: single time result (split time, finish time)
 * - EXTRA checkpoint: electronic control point reading
 */
public interface MessageListener {

    /**
     * KILPPVT (type 2): Competitor stage data update.
     * @param dk record index in KILP.DAT
     * @param pv stage number (0-based)
     * @param cpvData raw packed stage data (kilppvtpsize bytes)
     */
    default void onCompetitorUpdate(int dk, int pv, byte[] cpvData) {}

    /**
     * KILPT (type 1): Full competitor base record.
     * @param dk record index in KILP.DAT
     * @param entno entry/competitor number (0 = new entry)
     * @param recordData raw packed base record (kilprecsize0 bytes)
     */
    default void onFullCompetitorRecord(int dk, int entno, byte[] recordData) {}

    /**
     * VAIN_TULOST (type 3): Single time result.
     * @param dk record index in KILP.DAT
     * @param bib bib number
     * @param stage competition stage (k_pv)
     * @param splitIndex split point (-1=start, 0=finish, >0=split number)
     * @param time time value in 1/100s from t0
     */
    default void onTimeResult(int dk, int bib, int stage, int splitIndex, int time) {}

    /**
     * EXTRA type 1: Electronic checkpoint reading.
     * @param badge EMIT card number
     * @param time punch time
     * @param point control point number
     * @param source source connection (d2 field)
     */
    default void onCheckpoint(int badge, int time, int point, int source) {}

    /**
     * EXTRA type 9: Remote shutdown command received.
     * @param targetMachine 2-char machine ID (empty = all machines)
     */
    default void onShutdown(String targetMachine) {}
}
