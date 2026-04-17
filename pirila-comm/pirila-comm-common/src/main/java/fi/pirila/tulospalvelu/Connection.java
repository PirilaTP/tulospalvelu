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

    @Override
    public String toString() {
        return String.format("yhteys%d: %s %s:%d (srv:%d, emit:%s)",
                index, protocol, destAddr, destPort, srvPort, sendEmit);
    }
}
