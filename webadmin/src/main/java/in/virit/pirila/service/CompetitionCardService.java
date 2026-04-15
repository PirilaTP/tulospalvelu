package in.virit.pirila.service;

import fi.pirila.tulospalvelu.KilpReader;
import org.springframework.stereotype.Service;

import java.util.Optional;

@Service
public class CompetitionCardService {

    private final TulospalveluService tulospalveluService;

    public CompetitionCardService(TulospalveluService tulospalveluService) {
        this.tulospalveluService = tulospalveluService;
    }

    public Optional<String> findCardHolder(String cardNumber) {
        try {
            int badge = Integer.parseInt(cardNumber);
            return tulospalveluService.getCompetitors().stream()
                    .filter(c -> c.badge == badge)
                    .findFirst()
                    .map(c -> c.sukunimi + " " + c.etunimi + " (#" + c.kilpno + ")");
        } catch (NumberFormatException e) {
            return Optional.empty();
        }
    }

    public boolean changeCard(Long competitorId, String newCardNumber) {
        try {
            int badge = Integer.parseInt(newCardNumber);
            return tulospalveluService.sendCardChange(competitorId.intValue(), badge);
        } catch (NumberFormatException e) {
            return false;
        }
    }
}
