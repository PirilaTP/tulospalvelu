package in.virit.pirila.views;

import com.vaadin.browserless.SpringBrowserlessTest;
import in.virit.pirila.data.CardChangeRequest;
import in.virit.pirila.data.Competitor;
import in.virit.pirila.service.CompetitorService;
import in.virit.pirila.service.CompetitionCardService;
import jakarta.validation.Validator;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

@SpringBootTest
public class CardChangeViewTest extends SpringBrowserlessTest {

    @Autowired
    private CompetitorService competitorService;

    @Autowired
    private CompetitionCardService cardService;

    @Autowired
    private Validator validator;

    @Test
    public void testBlankCardNumberRejected() {
        var request = new CardChangeRequest();
        request.setCompetitor(new Competitor(1L, "1", "Test", "Club", ""));
        var violations = validator.validate(request);
        assertTrue(violations.stream().anyMatch(v -> v.getPropertyPath().toString().equals("cardNumber")),
                "Blank card number should be rejected");
    }

    @Test
    public void testNullCompetitorRejected() {
        var request = new CardChangeRequest();
        request.setCardNumber("88888888");
        var violations = validator.validate(request);
        assertTrue(violations.stream().anyMatch(v -> v.getPropertyPath().toString().equals("competitor")),
                "Null competitor should be rejected");
    }

    @Test
    public void testCardInUseRejected() {
        List<Competitor> competitors = competitorService.getAllCompetitors();
        Competitor withCard = competitors.stream()
                .filter(c -> c.getCardNumber() != null && !c.getCardNumber().isEmpty())
                .findFirst()
                .orElse(null);
        if (withCard == null) return;

        var request = new CardChangeRequest();
        request.setCardNumber(withCard.getCardNumber());
        request.setCompetitor(competitors.getFirst());
        var violations = validator.validate(request);
        assertTrue(violations.stream().anyMatch(v ->
                        v.getPropertyPath().toString().equals("cardNumber")
                                && v.getMessage().contains("käytössä")),
                "Card in use should be rejected with holder info");
    }

    @Test
    public void testFreeCardAccepted() {
        List<Competitor> competitors = competitorService.getAllCompetitors();
        if (competitors.isEmpty()) return;

        var request = new CardChangeRequest();
        request.setCardNumber("88888888");
        request.setCompetitor(competitors.getFirst());
        var violations = validator.validate(request);
        assertTrue(violations.isEmpty(), "Free card with valid competitor should pass validation");
    }
}
