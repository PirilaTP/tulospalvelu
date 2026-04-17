package in.virit.pirila.views;

import com.vaadin.flow.component.DetachEvent;
import com.vaadin.flow.component.grid.Grid;
import com.vaadin.flow.component.icon.Icon;
import com.vaadin.flow.component.icon.VaadinIcon;
import com.vaadin.flow.component.notification.Notification;
import com.vaadin.flow.component.orderedlayout.HorizontalLayout;
import com.vaadin.flow.component.textfield.TextField;
import com.vaadin.flow.data.value.ValueChangeMode;
import com.vaadin.flow.router.Route;
import in.virit.emit.Emit250ReaderButton;
import in.virit.pirila.data.CardChangeRequest;
import in.virit.pirila.data.Competitor;
import in.virit.pirila.emit.EmitCardReader;
import in.virit.pirila.service.CompetitionCardService;
import in.virit.pirila.service.CompetitorService;
import in.virit.pirila.service.TulospalveluService;
import in.virit.pirila.service.UserSession;
import jakarta.validation.ConstraintViolation;
import jakarta.validation.Validator;
import org.vaadin.firitin.appframework.MenuItem;
import org.vaadin.firitin.components.button.DefaultButton;
import org.vaadin.firitin.components.orderedlayout.VVerticalLayout;
import org.vaadin.firitin.components.textfield.VTextField;

import java.util.List;
import java.util.Set;

@Route(layout = TopLayout.class)
@MenuItem(icon = VaadinIcon.PENCIL)
public class CardChangeView extends VVerticalLayout {

    private final CompetitionCardService cardService;
    private final CompetitorService competitorService;
    private final TulospalveluService tulospalveluService;
    private final UserSession userSession;
    private final Validator validator;
    private final EmitCardReader emitCardReader;

    private final TextField cardNumber = new VTextField("Kilpailukortin numero") {{
        setPlaceholder("Skannaa tai syötä kortin numero");
        setClearButtonVisible(true);
        setAutofocus(true);
    }};

    private final Emit250ReaderButton emitReaderButton;

    private final TextField searchField = new TextField("Hae kilpailija (numero/nimi)") {{
        setPlaceholder("Hae kilpailijoita numerolla tai nimellä...");
        setClearButtonVisible(true);
        setWidthFull();
        setValueChangeMode(ValueChangeMode.LAZY);
        setValueChangeTimeout(500);
        setPrefixComponent(new Icon("vaadin", "search"));
    }};

    private final Grid<Competitor> competitorGrid = new Grid<>() {{
        addColumn(Competitor::getCompetitionNumber).setHeader("Kilpailunro").setSortable(true);
        addColumn(Competitor::getName).setHeader("Nimi").setSortable(true);
        addColumn(Competitor::getClub).setHeader("Seura").setSortable(true);
    }};

    private boolean clearing;

    private final DefaultButton saveButton = new DefaultButton("Vaihda kortti") {{
        setWidthFull();
    }};

    public CardChangeView(CompetitionCardService cardService,
                          CompetitorService competitorService,
                          TulospalveluService tulospalveluService,
                          UserSession userSession,
                          EmitCardReader emitCardReader,
                          Validator validator) {
        this.cardService = cardService;
        this.competitorService = competitorService;
        this.tulospalveluService = tulospalveluService;
        this.userSession = userSession;
        this.emitCardReader = emitCardReader;
        this.validator = validator;

        emitReaderButton = new Emit250ReaderButton(
                () -> Notification.show("Emit-lukija valmis!", 3000, Notification.Position.MIDDLE),
                ecard -> {
                    String number = String.valueOf(ecard.ecardNumber());
                    if (!number.equals(cardNumber.getValue())) {
                        cardNumber.setValue(number);
                    }
                    searchField.focus();
                }
        );
        emitReaderButton.getContent().setText("yhdistä lukija");
        emitReaderButton.setFilterUsbReaders(true);

        cardNumber.addValueChangeListener(e -> validateForm());
        competitorGrid.asSingleSelect().addValueChangeListener(e -> validateForm());
        searchField.addValueChangeListener(e -> searchCompetitors());
        saveButton.setEnabled(false);
        saveButton.addClickListener(e -> changeCard());
        competitorGrid.setItems(competitorService.getAllCompetitors());
        startEmitCardReading();

        var cardNumberRow = new HorizontalLayout(cardNumber, emitReaderButton) {{
            setWidthFull();
            setDefaultVerticalComponentAlignment(Alignment.END);
            expand(cardNumber);
        }};

        add(cardNumberRow, searchField);
        addAndExpand(competitorGrid);
        add(saveButton);
        setSizeFull();
    }

    private CardChangeRequest readForm() {
        var request = new CardChangeRequest();
        request.setCardNumber(cardNumber.getValue());
        request.setCompetitor(competitorGrid.asSingleSelect().getValue());
        return request;
    }

    private void validateForm() {
        if (clearing) return;
        var violations = validator.validate(readForm());
        showViolations(violations);
        saveButton.setEnabled(violations.isEmpty());
    }

    private void showViolations(Set<ConstraintViolation<CardChangeRequest>> violations) {
        String cardError = violations.stream()
                .filter(v -> "cardNumber".equals(v.getPropertyPath().toString()))
                .map(ConstraintViolation::getMessage)
                .findFirst()
                .orElse(null);
        cardNumber.setInvalid(cardError != null);
        cardNumber.setErrorMessage(cardError);
    }

    private void changeCard() {
        if (tulospalveluService.isPasswordRequired() && !userSession.isAuthenticated()) {
            Notification.show("Kirjaudu ensin etusivulla", 3000, Notification.Position.MIDDLE);
            getUI().ifPresent(ui -> ui.navigate(MainView.class));
            return;
        }

        var request = readForm();
        var violations = validator.validate(request);
        showViolations(violations);
        if (!violations.isEmpty()) return;

        boolean success = cardService.changeCard(
                request.getCompetitor().getId(),
                request.getCardNumber()
        );

        if (success) {
            Notification.show("Kilpailukortti vaihdettu onnistuneesti", 3000, Notification.Position.MIDDLE);
            clearForm();
        } else {
            Notification.show("Kilpailukortin vaihto epäonnistui", 3000, Notification.Position.MIDDLE);
        }
    }

    private void clearForm() {
        clearing = true;
        cardNumber.clear();
        searchField.clear();
        competitorGrid.asSingleSelect().clear();
        competitorGrid.setItems(competitorService.getAllCompetitors());
        saveButton.setEnabled(false);
        cardNumber.setInvalid(false);
        cardNumber.setErrorMessage(null);
        clearing = false;
        cardNumber.focus();
    }

    private void searchCompetitors() {
        String term = searchField.getValue().trim();
        List<Competitor> competitors = term.isEmpty()
                ? competitorService.getAllCompetitors()
                : competitorService.searchCompetitors(term);
        competitorGrid.setItems(competitors);
        if (competitors.isEmpty()) {
            Notification.show("Ei kilpailijoita löytynyt", 3000, Notification.Position.MIDDLE);
        }
    }

    private void startEmitCardReading() {
        emitCardReader.startReading(cardNo ->
                getUI().ifPresent(ui -> ui.access(() -> {
                    if (!cardNo.equals(cardNumber.getValue())) {
                        cardNumber.setValue(cardNo);
                    }
                    searchField.focus();
                }))
        );
    }

    @Override
    public void onDetach(DetachEvent detachEvent) {
        emitCardReader.stopReading();
        super.onDetach(detachEvent);
    }

    // Test accessors
    TextField getCardNumberField() { return cardNumber; }
    Grid<Competitor> getCompetitorGrid() { return competitorGrid; }
    DefaultButton getSaveButton() { return saveButton; }
}
