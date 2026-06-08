package in.virit.pirila;

import fi.pirila.tulospalvelu.TulospalveluProtocol;
import fi.pirila.tulospalvelu.TulospalveluProtocol.AlkutInfo;
import in.virit.pirila.service.TulospalveluService;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Covers ALKUT handshake parsing and the day/record mismatch detection that
 * drives the connection-warning notification.
 */
class PeerHandshakeTest {

    @Test
    void parseAlkutRoundTrip() {
        byte[] data = TulospalveluProtocol.buildAlkutData("J1", 21);
        AlkutInfo info = TulospalveluProtocol.parseAlkutData(data);
        assertNotNull(info);
        assertEquals("J1", info.machineId());
        assertEquals(1, info.vaihe());
        assertEquals(21, info.nrec());
    }

    @Test
    void parsesObservedServerBytes() {
        // The exact ALKUT payload seen from the C++ server (Kone=SE, 21 records).
        byte[] data = {0x01, 0x53, 0x45, 0x01, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00};
        AlkutInfo info = TulospalveluProtocol.parseAlkutData(data);
        assertNotNull(info);
        assertEquals("SE", info.machineId());
        assertEquals(1, info.vaihe());
        assertEquals(21, info.nrec());
    }

    @Test
    void parseAlkutRejectsShortPayload() {
        assertNull(TulospalveluProtocol.parseAlkutData(new byte[]{1, 2, 3}));
        assertNull(TulospalveluProtocol.parseAlkutData(null));
    }

    @Test
    void differentDayRaisesWarning() {
        TulospalveluService service = new TulospalveluService();
        service.onPeerHandshake("MA", 2, 21); // peer on day 2, webadmin on day 1
        assertNotNull(service.getConnectionWarning());
        assertTrue(service.getConnectionWarning().contains("eri kilpailupäivässä"));
    }

    @Test
    void matchingHandshakeLeavesNoWarning() {
        TulospalveluService service = new TulospalveluService();
        service.onPeerHandshake("MA", 1, 21); // same day; localNrec unknown (-1) so nrec ignored
        assertNull(service.getConnectionWarning());
    }
}
