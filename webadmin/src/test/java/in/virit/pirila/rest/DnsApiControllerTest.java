package in.virit.pirila.rest;

import fi.pirila.tulospalvelu.Competitor;
import fi.pirila.tulospalvelu.TulospalveluProtocol;
import in.virit.pirila.service.TulospalveluService;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.server.ResponseStatusException;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.*;

/**
 * Unit tests for the state validation in {@link DnsApiController} — the
 * guardrails that stop a "dumb" client from clobbering real results.
 */
class DnsApiControllerTest {

    private TulospalveluService service;
    private DnsApiController controller;

    private static final int KILPNO = 101;
    private static final int RECORD = 5;

    @BeforeEach
    void setUp() {
        service = mock(TulospalveluService.class);
        controller = new DnsApiController(service);
        when(service.isConnected()).thenReturn(true);
        when(service.sendStatusChange(anyInt(), anyChar())).thenReturn(true);
    }

    private void hasCompetitor(char keskhyl, int ysija, int finishTime) {
        Competitor c = new Competitor(RECORD, KILPNO, "Lähtijä", "Liisa",
                "Seura", "SEU", 0, 1, 0, 0, keskhyl, ysija, finishTime, 0);
        when(service.getCompetitorByKilpno(KILPNO)).thenReturn(c);
    }

    // --- dns ---------------------------------------------------------------

    @Test
    void dnsOnOpenCompetitorSendsChange() {
        hasCompetitor((char) 0, 0, 0); // open: no status, no result
        ResponseEntity<CompetitorStatusResponse> r = controller.markNotStarted(KILPNO);
        assertEquals(HttpStatus.OK, r.getStatusCode());
        assertTrue(r.getBody().changed());
        assertEquals("E", r.getBody().status());
        verify(service).sendStatusChange(RECORD, TulospalveluProtocol.STATUS_DNS);
    }

    @Test
    void dnsOnAlreadyDnsIsNoOp() {
        hasCompetitor('E', 0, 0);
        ResponseEntity<CompetitorStatusResponse> r = controller.markNotStarted(KILPNO);
        assertEquals(HttpStatus.OK, r.getStatusCode());
        assertFalse(r.getBody().changed());
        verify(service, never()).sendStatusChange(anyInt(), anyChar());
    }

    @Test
    void dnsRefusedWhenResultExists() {
        hasCompetitor((char) 0, 3, 4899000); // placed finisher
        ResponseEntity<CompetitorStatusResponse> r = controller.markNotStarted(KILPNO);
        assertEquals(HttpStatus.CONFLICT, r.getStatusCode());
        assertFalse(r.getBody().changed());
        verify(service, never()).sendStatusChange(anyInt(), anyChar());
    }

    @Test
    void dnsRefusedWhenOtherStatusSet() {
        hasCompetitor('H', 0, 0); // disqualified
        ResponseEntity<CompetitorStatusResponse> r = controller.markNotStarted(KILPNO);
        assertEquals(HttpStatus.CONFLICT, r.getStatusCode());
        verify(service, never()).sendStatusChange(anyInt(), anyChar());
    }

    // --- open --------------------------------------------------------------

    @Test
    void openOnDnsSendsChange() {
        hasCompetitor('E', 0, 0);
        ResponseEntity<CompetitorStatusResponse> r = controller.markOpen(KILPNO);
        assertEquals(HttpStatus.OK, r.getStatusCode());
        assertTrue(r.getBody().changed());
        assertEquals("-", r.getBody().status());
        verify(service).sendStatusChange(RECORD, TulospalveluProtocol.STATUS_OPEN);
    }

    @Test
    void openOnAlreadyOpenIsNoOp() {
        hasCompetitor((char) 0, 0, 0);
        ResponseEntity<CompetitorStatusResponse> r = controller.markOpen(KILPNO);
        assertEquals(HttpStatus.OK, r.getStatusCode());
        assertFalse(r.getBody().changed());
        verify(service, never()).sendStatusChange(anyInt(), anyChar());
    }

    @Test
    void openRefusedWhenFinished() {
        hasCompetitor((char) 0, 1, 4899000);
        ResponseEntity<CompetitorStatusResponse> r = controller.markOpen(KILPNO);
        assertEquals(HttpStatus.CONFLICT, r.getStatusCode());
        verify(service, never()).sendStatusChange(anyInt(), anyChar());
    }

    // --- preconditions -----------------------------------------------------

    @Test
    void notConnectedYields503() {
        when(service.isConnected()).thenReturn(false);
        ResponseStatusException ex = assertThrows(ResponseStatusException.class,
                () -> controller.markNotStarted(KILPNO));
        assertEquals(HttpStatus.SERVICE_UNAVAILABLE, ex.getStatusCode());
    }

    @Test
    void unknownCompetitorYields404() {
        when(service.getCompetitorByKilpno(anyInt())).thenReturn(null);
        ResponseStatusException ex = assertThrows(ResponseStatusException.class,
                () -> controller.markNotStarted(KILPNO));
        assertEquals(HttpStatus.NOT_FOUND, ex.getStatusCode());
    }

    @Test
    void serverRejectionYields502() {
        hasCompetitor((char) 0, 0, 0);
        when(service.sendStatusChange(eq(RECORD), anyChar())).thenReturn(false);
        ResponseStatusException ex = assertThrows(ResponseStatusException.class,
                () -> controller.markNotStarted(KILPNO));
        assertEquals(HttpStatus.BAD_GATEWAY, ex.getStatusCode());
    }
}
