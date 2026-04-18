package in.virit.pirila.views;

import com.vaadin.flow.component.AttachEvent;
import com.vaadin.flow.component.DetachEvent;
import com.vaadin.flow.component.button.ButtonVariant;
import com.vaadin.flow.component.dialog.Dialog;
import com.vaadin.flow.component.grid.Grid;
import com.vaadin.flow.component.html.Paragraph;
import com.vaadin.flow.component.icon.Icon;
import com.vaadin.flow.component.icon.VaadinIcon;
import com.vaadin.flow.component.notification.Notification;
import com.vaadin.flow.component.orderedlayout.HorizontalLayout;
import com.vaadin.flow.component.progressbar.ProgressBar;
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
import org.vaadin.firitin.util.style.AuraProps;
import org.vaadin.firitin.util.style.VaadinCssProps;

import java.util.List;
import java.util.Set;
import java.util.function.Consumer;

@Route(layout = TopLayout.class)
@MenuItem(icon = VaadinIcon.PENCIL)
public class CardChangeView extends VVerticalLayout implements Consumer<fi.pirila.tulospalvelu.Competitor> {

    private final CompetitionCardService cardService;
    private final CompetitorService competitorService;
    private final TulospalveluService tulospalveluService;
    private final UserSession userSession;
    private final Validator validator;
    private final EmitCardReader emitCardReader;

    private final TextField cardNumber = new VTextField("Uuden kilpailukortin numero") {{
        setPlaceholder("Skannaa tai syötä kortin numero");
        setClearButtonVisible(true);
        setAutofocus(true);
    }};

    private final Emit250ReaderButton emitReaderButton;

    private final TextField searchField = new TextField("Valitse kilpailija (numero/nimi)") {{
        setPlaceholder("Hae kilpailijoita numerolla tai nimellä...");
        setClearButtonVisible(true);
        setWidthFull();
        setValueChangeMode(ValueChangeMode.LAZY);
        setValueChangeTimeout(500);
        setPrefixComponent(new Icon("vaadin", "search"));
    }};

    private final Grid<Competitor> competitorGrid = new Grid<>() {{
        addColumn(Competitor::getCompetitionNumber).setHeader("Bib").setFlexGrow(0);
        addColumn(Competitor::getCardNumber).setHeader("Kortti").setFlexGrow(0);
        addColumn(Competitor::getName).setHeader("Nimi");
        addColumn(Competitor::getClub).setHeader("Seura");
        getColumns().forEach(c -> {
            c.setSortable(true);
            c.setAutoWidth(true);
        });
    }};

    private boolean clearing;

    private final DefaultButton saveButton = new DefaultButton("Vaihda kortti") {{
        addThemeVariants(ButtonVariant.LARGE);
        getStyle().setPadding(VaadinCssProps.PADDING_L.var());
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
        ){
            {
                getContent().setText("yhdistä lukija");
                setFilterUsbReaders(true);
                getContent().setTabIndex(-1);
            }};

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

        add(searchField);
        addAndExpand(competitorGrid);
        add(cardNumberRow);
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
            ui().navigate(MainView.class);
            return;
        }

        var request = readForm();
        var violations = validator.validate(request);
        showViolations(violations);
        if (!violations.isEmpty()) return;

        var dialog = new Dialog();
        dialog.setHeaderTitle("Vaihdetaan emit-korttia...");
        dialog.setCloseOnEsc(false);
        dialog.setCloseOnOutsideClick(false);
        var progress = new ProgressBar();
        progress.setIndeterminate(true);
        var statusText = new Paragraph("Lähetetään palvelimelle, yritetään uudelleen jos verkko on ruuhkainen...");
        dialog.add(progress, statusText);
        dialog.open();

        saveButton.setEnabled(false);

        var competitorId = request.getCompetitor().getId();
        var cardNum = request.getCardNumber();

        new Thread(() -> {
            boolean success = cardService.changeCard(competitorId, cardNum);
            ui().access(() -> {
                dialog.close();
                if (success) {
                    Notification.show("Kilpailukortti vaihdettu onnistuneesti", 3000, Notification.Position.MIDDLE);
                    clearForm();
                } else {
                    Notification.show("Kilpailukortin vaihto epäonnistui usean yrityksen jälkeen", 5000, Notification.Position.MIDDLE);
                    saveButton.setEnabled(true);
                }
            });
        }).start();
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
        if (competitors.size() == 1) {
            competitorGrid.asSingleSelect().setValue(competitors.getFirst());
        }
        if (competitors.isEmpty()) {
            Notification.show("Ei kilpailijoita löytynyt", 3000, Notification.Position.MIDDLE);
        }
    }

    private void startEmitCardReading() {
        emitCardReader.startReading(cardNo ->
                ui().access(()->{
                    if (!cardNo.equals(cardNumber.getValue())) {
                        cardNumber.setValue(cardNo);
                    }
                    searchField.focus();
                }));
    }

    @Override
    public void onDetach(DetachEvent detachEvent) {
        emitCardReader.stopReading();
        tulospalveluService.removeUpdateListener(this);
        super.onDetach(detachEvent);
    }

    @Override
    protected void onAttach(AttachEvent attachEvent) {
        super.onAttach(attachEvent);
        tulospalveluService.addUpdateListener(this);
    }

    // Test accessors
    TextField getCardNumberField() { return cardNumber; }
    Grid<Competitor> getCompetitorGrid() { return competitorGrid; }
    DefaultButton getSaveButton() { return saveButton; }

    @Override
    public void accept(fi.pirila.tulospalvelu.Competitor competitor) {
        ui().access(this::searchCompetitors);
    }
}
