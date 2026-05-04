package fi.pirila.tulospalvelu;

/**
 * A parsed connection entry from laskenta.cfg.
 *
 * @param index connection number (1-based, as in yhteys1, yhteys2...)
 * @param protocol "udp" or "tcp"
 * @param destAddr destination host address
 * @param destPort destination port
 * @param srvPort local server port for incoming data
 * @param sendEmit true if this connection sends emit data (lähemit)
 */
public record Connection(int index, String protocol, String destAddr, int destPort, int srvPort, boolean sendEmit) {

    public boolean isTcp() {
        return "tcp".equalsIgnoreCase(protocol);
    }

    /**
     * Passive (listen-only) UDP entry from laskenta.cfg, e.g. "yhteys1=UDP" with no
     * destination. Peer address is learned from the first incoming packet.
     */
    public boolean isPassive() {
        return destAddr == null || destAddr.isBlank() || "AUTO".equalsIgnoreCase(destAddr);
    }

    @Override
    public String toString() {
        if (isPassive()) {
            return String.format("yhteys%d: %s passive (srv:%d, emit:%s)",
                    index, protocol, srvPort, sendEmit);
        }
        return String.format("yhteys%d: %s %s:%d (srv:%d, emit:%s)",
                index, protocol, destAddr, destPort, srvPort, sendEmit);
    }
}
