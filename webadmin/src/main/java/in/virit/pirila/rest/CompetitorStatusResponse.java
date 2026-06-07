package in.virit.pirila.rest;

import fi.pirila.tulospalvelu.Competitor;

/**
 * JSON body returned by the status endpoints.
 *
 * @param kilpno  competition number (the key the caller addressed)
 * @param name    competitor's full name, for convenience / logging on the caller side
 * @param status  the new status code now in effect ("E" = ei lähtenyt / DNS, "-" = avoin / open)
 * @param message human-readable summary in Finnish
 */
public record CompetitorStatusResponse(int kilpno, String name, String status, String message) {

    static CompetitorStatusResponse of(Competitor c, char status, String message) {
        String name = ((c.etunimi == null ? "" : c.etunimi) + " "
                + (c.sukunimi == null ? "" : c.sukunimi)).trim();
        return new CompetitorStatusResponse(c.kilpno, name, String.valueOf(status), message);
    }
}
