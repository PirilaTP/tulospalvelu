package in.virit.pirila.views;

import com.vaadin.browserless.SpringBrowserlessTest;
import com.vaadin.flow.component.grid.Grid;
import com.vaadin.flow.component.textfield.TextField;
import in.virit.pirila.data.Competitor;
import in.virit.pirila.service.CompetitorService;
import in.virit.pirila.service.CompetitionCardService;
import in.virit.pirila.service.TulospalveluService;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import java.util.List;
import java.util.Random;

import static org.junit.jupiter.api.Assertions.*;

@SpringBootTest
public class CardChangeViewBrowserlessTest extends SpringBrowserlessTest {

    private CardChangeView view;

    @Autowired
    private CompetitorService competitorService;

    @Autowired
    private CompetitionCardService cardService;

    @Autowired
    private TulospalveluService tulospalveluService;

    @BeforeEach
    public void setUp() {
        view = navigate(CardChangeView.class);
    }

    @Test
    public void testSaveButtonDisabledInitially() {
        assertFalse(view.getSaveButton().isEnabled(),
                "Save button should be disabled when form is empty");
    }

    @Test
    public void testCardInUseShowsErrorOnField() {
        List<Competitor> allCompetitors = competitorService.getAllCompetitors();
        Competitor withCard = allCompetitors.stream()
                .filter(c -> c.getCardNumber() != null && !c.getCardNumber().isEmpty())
                .findFirst()
                .orElse(null);
        if (withCard == null) return;

        test(view.getCardNumberField()).setValue(withCard.getCardNumber());
        test(view.getCompetitorGrid()).select(0);
        // TODO API should support selecting by actual row item
        //view.getCompetitorGrid().asSingleSelect().setValue(allCompetitors.getFirst());

        assertTrue(view.getCardNumberField().isInvalid(),
                "Card number field should be invalid when card is in use");
        assertTrue(view.getCardNumberField().getErrorMessage().contains("käytössä"),
                "Error message should mention the card holder");
        assertFalse(view.getSaveButton().isEnabled(),
                "Save button should stay disabled when card is in use");
    }

    @Test
    public void testFreeCardAndCompetitorEnablesSaveButton() {
        List<Competitor> allCompetitors = competitorService.getAllCompetitors();

        if (allCompetitors.isEmpty()) return;

        String randomUsedCard = "0" + new Random().nextInt(1000, 2000);

        TextField cardNumberField = view.getCardNumberField();
        test(cardNumberField).setValue(randomUsedCard);

        // TODO can't find by label !?!
        // TODO can't just focus field !?!
        // Indexes start from 1 !?!
        // Trying to type something for search field to get shit done..
        test($(TextField.class).atIndex(2)).setValue("L");

        view.getCompetitorGrid().asSingleSelect().setValue(allCompetitors.getFirst());

        assertFalse(cardNumberField.isInvalid(),
                "Card number field should not be invalid for a free card");
        assertTrue(view.getSaveButton().isEnabled(),
                "Save button should be enabled when card is free and competitor selected");
    }

    @Test
    public void testValidFormEnablesSaveAndClickDoesNotThrow() {
        List<Competitor> allCompetitors = competitorService.getAllCompetitors();
        if (allCompetitors.isEmpty()) return;

        int randomeCard = new Random().nextInt(10000, 50000);
        test(view.getCardNumberField()).setValue("" + randomeCard);
        test($(Grid.class).single()).select(0);
        assertTrue(view.getSaveButton().isEnabled(),
                "Save button should be enabled with valid form data");

        assertDoesNotThrow(() -> view.getSaveButton().click());
    }
}
