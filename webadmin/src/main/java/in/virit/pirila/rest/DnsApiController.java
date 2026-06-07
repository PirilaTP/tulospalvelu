package in.virit.pirila.rest;

import fi.pirila.tulospalvelu.Competitor;
import fi.pirila.tulospalvelu.TulospalveluProtocol;
import in.virit.pirila.service.TulospalveluService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.server.ResponseStatusException;

/**
 * REST API for marking competitors as not started (DNS) and reopening them.
 *
 * <p>Intended first consumer is the DNS start-line app
 * (https://github.com/mstahv/dns): when an official confirms a competitor did
 * not start, the app calls {@code dns}; if a late starter who was already
 * marked DNS turns up and is registered at the start, the app calls
 * {@code open} to revert the status.
 *
 * <p>The endpoints validate the current state so a "dumb" client cannot
 * clobber real data:
 * <ul>
 *   <li>{@code dns} only acts on an <em>open</em> competitor. If a result
 *       already exists (or any other status is set) the change is refused with
 *       {@code 409}. If the competitor is already DNS it is a no-op success.</li>
 *   <li>{@code open} only acts on a competitor currently marked DNS. Anything
 *       else is refused with {@code 409}; already-open is a no-op success.</li>
 * </ul>
 *
 * <p>Competitors are addressed by competition number (kilpno). All requests
 * must carry a valid api key — see {@link ApiKeyAuthFilter}.
 */
@RestController
@RequestMapping("/api/v1/competitors")
public class DnsApiController {

    private static final Logger log = LoggerFactory.getLogger(DnsApiController.class);

    private final TulospalveluService service;

    public DnsApiController(TulospalveluService service) {
        this.service = service;
    }

    /** Mark the competitor as not started (ei lähtenyt / DNS). Only allowed if currently open. */
    @PostMapping("/{kilpno}/dns")
    public ResponseEntity<CompetitorStatusResponse> markNotStarted(@PathVariable int kilpno) {
        Competitor c = requireCompetitor(kilpno);
        if (isDns(c)) {
            return noChange(c, TulospalveluProtocol.STATUS_DNS,
                    "Oli jo merkitty ei lähteneeksi (DNS)");
        }
        if (!isOpen(c)) {
            return refused(c, "Ei voitu merkitä ei lähteneeksi");
        }
        send(c, TulospalveluProtocol.STATUS_DNS);
        return changed(c, TulospalveluProtocol.STATUS_DNS, "Merkitty ei lähteneeksi (DNS)");
    }

    /** Revert the competitor to open (avoin). Only allowed if currently marked DNS. */
    @PostMapping("/{kilpno}/open")
    public ResponseEntity<CompetitorStatusResponse> markOpen(@PathVariable int kilpno) {
        Competitor c = requireCompetitor(kilpno);
        if (isOpen(c)) {
            return noChange(c, TulospalveluProtocol.STATUS_OPEN, "Oli jo avoin");
        }
        if (!isDns(c)) {
            return refused(c, "Ei voitu palauttaa avoimeksi");
        }
        send(c, TulospalveluProtocol.STATUS_OPEN);
        return changed(c, TulospalveluProtocol.STATUS_OPEN, "Palautettu avoimeksi");
    }

    // --- state predicates -------------------------------------------------

    /** Open = no result and no decided status (keskhyl unset). */
    private static boolean isOpen(Competitor c) {
        boolean noStatus = c.keskhyl == 0 || c.keskhyl == TulospalveluProtocol.STATUS_OPEN;
        boolean noResult = c.finishTime <= 0 && c.ysija <= 0;
        return noStatus && noResult;
    }

    private static boolean isDns(Competitor c) {
        return c.keskhyl == 'E' || c.keskhyl == 'e';
    }

    // --- helpers ----------------------------------------------------------

    private Competitor requireCompetitor(int kilpno) {
        if (!service.isConnected()) {
            throw new ResponseStatusException(HttpStatus.SERVICE_UNAVAILABLE,
                    "Ei yhteyttä Tulospalvelu-palvelimeen");
        }
        Competitor c = service.getCompetitorByKilpno(kilpno);
        if (c == null) {
            throw new ResponseStatusException(HttpStatus.NOT_FOUND,
                    "Kilpailijaa numerolla " + kilpno + " ei löytynyt");
        }
        return c;
    }

    private void send(Competitor c, char newStatus) {
        boolean ok = service.sendStatusChange(c.recordIndex, newStatus);
        if (!ok) {
            log.warn("Status change rejected by server: kilpno={} newStatus='{}'", c.kilpno, newStatus);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY,
                    "Tulospalvelu-palvelin ei hyväksynyt muutosta");
        }
        log.info("API status change ok: kilpno={} ('{} {}') newStatus='{}'",
                c.kilpno, c.etunimi, c.sukunimi, newStatus);
    }

    private static ResponseEntity<CompetitorStatusResponse> changed(
            Competitor c, char status, String message) {
        return ResponseEntity.ok(CompetitorStatusResponse.of(c, status, true, message));
    }

    private static ResponseEntity<CompetitorStatusResponse> noChange(
            Competitor c, char status, String message) {
        return ResponseEntity.ok(CompetitorStatusResponse.of(c, status, false, message));
    }

    private static ResponseEntity<CompetitorStatusResponse> refused(Competitor c, String why) {
        String message = why + ": kilpailijan nykyinen tila on \"" + c.formatResult() + "\"";
        char current = c.keskhyl == 0 ? TulospalveluProtocol.STATUS_OPEN : c.keskhyl;
        return ResponseEntity.status(HttpStatus.CONFLICT)
                .body(CompetitorStatusResponse.of(c, current, false, message));
    }
}
