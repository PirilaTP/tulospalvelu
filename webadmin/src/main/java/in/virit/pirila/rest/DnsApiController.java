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

    /** Mark the competitor as not started (ei lähtenyt / DNS). */
    @PostMapping("/{kilpno}/dns")
    public CompetitorStatusResponse markNotStarted(@PathVariable int kilpno) {
        return changeStatus(kilpno, TulospalveluProtocol.STATUS_DNS,
                "Merkitty ei lähteneeksi (DNS)");
    }

    /** Revert the competitor to open (avoin) — e.g. a late starter did start after all. */
    @PostMapping("/{kilpno}/open")
    public CompetitorStatusResponse markOpen(@PathVariable int kilpno) {
        return changeStatus(kilpno, TulospalveluProtocol.STATUS_OPEN,
                "Palautettu avoimeksi");
    }

    private CompetitorStatusResponse changeStatus(int kilpno, char newStatus, String okMessage) {
        if (!service.isConnected()) {
            throw new ResponseStatusException(HttpStatus.SERVICE_UNAVAILABLE,
                    "Ei yhteyttä Tulospalvelu-palvelimeen");
        }
        Competitor c = service.getCompetitorByKilpno(kilpno);
        if (c == null) {
            throw new ResponseStatusException(HttpStatus.NOT_FOUND,
                    "Kilpailijaa numerolla " + kilpno + " ei löytynyt");
        }
        boolean ok = service.sendStatusChange(c.recordIndex, newStatus);
        if (!ok) {
            log.warn("Status change rejected by server: kilpno={} newStatus='{}'", kilpno, newStatus);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY,
                    "Tulospalvelu-palvelin ei hyväksynyt muutosta");
        }
        log.info("API status change ok: kilpno={} ('{} {}') newStatus='{}'",
                kilpno, c.etunimi, c.sukunimi, newStatus);
        return CompetitorStatusResponse.of(c, newStatus, okMessage);
    }
}

