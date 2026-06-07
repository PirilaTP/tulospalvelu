package in.virit.pirila.rest;

import in.virit.pirila.service.TulospalveluService;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * Connectivity check for API clients. Reaching this endpoint at all means the
 * api key was accepted (otherwise {@link ApiKeyAuthFilter} would have answered
 * 401/503), so a {@code 200} confirms the credentials are correct.
 *
 * <p>The body additionally reports whether the webadmin currently has a live
 * connection to the Tulospalvelu C++ server, which lets a client tell "key is
 * good but the competition isn't reachable yet" apart from "key is wrong".
 */
@RestController
@RequestMapping("/api/v1")
public class PingController {

    private final TulospalveluService service;

    public PingController(TulospalveluService service) {
        this.service = service;
    }

    /**
     * @param ok        always true — a non-200 response means the key was rejected
     * @param connected whether webadmin is connected to the Tulospalvelu server
     */
    public record PingResponse(boolean ok, boolean connected) {}

    @GetMapping("/ping")
    public PingResponse ping() {
        return new PingResponse(true, service.isConnected());
    }
}
