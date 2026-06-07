package in.virit.pirila.rest;

import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.web.filter.OncePerRequestFilter;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

/**
 * Authenticates REST API requests with a shared API key.
 *
 * <p>The key is read from the {@code tulospalvelu.api.key} property, which
 * follows Spring Boot's usual resolution order: {@code application.properties},
 * the {@code TULOSPALVELU_API_KEY} environment variable, or a
 * {@code --tulospalvelu.api.key=...} command-line argument.
 *
 * <p>Callers present the key in the {@code X-API-Key} header (or as an
 * {@code Authorization: Bearer <key>} header). If no key is configured the API
 * is treated as disabled and every request is rejected — the API never runs
 * open.
 *
 * <p>{@link ApiConfig} registers this filter scoped to {@code /api/*}, so the
 * Vaadin UI and its session-based authentication are left untouched.
 */
public class ApiKeyAuthFilter extends OncePerRequestFilter {

    private static final Logger log = LoggerFactory.getLogger(ApiKeyAuthFilter.class);

    public static final String HEADER = "X-API-Key";
    private static final String BEARER_PREFIX = "Bearer ";

    private final String configuredKey;

    public ApiKeyAuthFilter(String configuredKey) {
        this.configuredKey = configuredKey == null ? "" : configuredKey.trim();
        if (this.configuredKey.isEmpty()) {
            log.warn("No tulospalvelu.api.key configured — REST API under /api is disabled "
                    + "and will reject all requests. Set the property, the TULOSPALVELU_API_KEY "
                    + "environment variable, or pass --tulospalvelu.api.key=... to enable it.");
        }
    }

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response,
                                    FilterChain chain) throws ServletException, IOException {
        if (configuredKey.isEmpty()) {
            deny(response, HttpStatus.SERVICE_UNAVAILABLE,
                    "API is disabled: no api key configured on the server");
            return;
        }
        String presented = extractKey(request);
        if (presented == null || !constantTimeEquals(presented, configuredKey)) {
            log.info("Rejected API request to {} — missing or invalid api key", request.getRequestURI());
            deny(response, HttpStatus.UNAUTHORIZED, "Missing or invalid api key");
            return;
        }
        chain.doFilter(request, response);
    }

    private static String extractKey(HttpServletRequest request) {
        String header = request.getHeader(HEADER);
        if (header != null && !header.isBlank()) {
            return header.trim();
        }
        String auth = request.getHeader(HttpHeaders.AUTHORIZATION);
        if (auth != null && auth.startsWith(BEARER_PREFIX)) {
            return auth.substring(BEARER_PREFIX.length()).trim();
        }
        return null;
    }

    private static boolean constantTimeEquals(String a, String b) {
        return MessageDigest.isEqual(
                a.getBytes(StandardCharsets.UTF_8), b.getBytes(StandardCharsets.UTF_8));
    }

    private static void deny(HttpServletResponse response, HttpStatus status, String message)
            throws IOException {
        response.setStatus(status.value());
        response.setContentType(MediaType.APPLICATION_JSON_VALUE);
        response.setCharacterEncoding(StandardCharsets.UTF_8.name());
        // Small hand-written JSON keeps the filter free of an ObjectMapper dependency.
        response.getWriter().write("{\"error\":\"" + message.replace("\"", "'") + "\"}");
    }
}
