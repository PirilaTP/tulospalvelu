package in.virit.pirila.views;

import com.vaadin.flow.component.AttachEvent;
import com.vaadin.flow.component.DetachEvent;
import com.vaadin.flow.component.button.Button;
import com.vaadin.flow.component.button.ButtonVariant;
import com.vaadin.flow.component.dialog.Dialog;
import com.vaadin.flow.component.html.Paragraph;
import com.vaadin.flow.component.icon.Icon;
import com.vaadin.flow.component.icon.VaadinIcon;
import com.vaadin.flow.component.menubar.MenuBar;
import com.vaadin.flow.component.menubar.MenuBarVariant;
import com.vaadin.flow.component.notification.Notification;
import com.vaadin.flow.component.progressbar.ProgressBar;
import com.vaadin.flow.component.textfield.TextField;
import com.vaadin.flow.data.value.ValueChangeMode;
import com.vaadin.flow.router.Route;
import fi.pirila.tulospalvelu.TulospalveluProtocol;
import in.virit.pirila.data.Competitor;
import in.virit.pirila.service.CompetitorService;
import in.virit.pirila.service.TulospalveluService;
import in.virit.pirila.service.UserSession;
import org.vaadin.firitin.appframework.MenuItem;
import org.vaadin.firitin.components.grid.VGrid;
import org.vaadin.firitin.components.orderedlayout.VVerticalLayout;

import java.util.List;
import java.util.function.Consumer;

@Route(layout = TopLayout.class)
@MenuItem(icon = VaadinIcon.GRID)
public class CompetitorListView extends VVerticalLayout implements Consumer<fi.pirila.tulospalvelu.Competitor> {

    private final CompetitorService competitorService;
    private final TulospalveluService tulospalveluService;
    private final UserSession userSession;

    private final TextField searchField = new TextField("Hae (numero/nimi/seura/sarja)") {{
        setPlaceholder("Hae kilpailijoita...");
        setClearButtonVisible(true);
        setWidthFull();
        setValueChangeMode(ValueChangeMode.LAZY);
        setValueChangeTimeout(500);
        setPrefixComponent(new Icon("vaadin", "search"));
        setAutofocus(true);
    }};

    private final VGrid<Competitor> competitorGrid = new CompetitorGrid();

    public CompetitorListView(CompetitorService competitorService,
                              TulospalveluService tulospalveluService,
                              UserSession userSession) {
        this.competitorService = competitorService;
        this.tulospalveluService = tulospalveluService;
        this.userSession = userSession;

        searchField.addValueChangeListener(event -> search());
        competitorGrid.setItems(competitorService.getAllCompetitors());

        add(searchField);
        addAndExpand(competitorGrid);
        setSizeFull();
    }

    @Override
    protected void onAttach(AttachEvent attachEvent) {
        super.onAttach(attachEvent);
        tulospalveluService.addUpdateListener(this);
    }

    @Override
    public void onDetach(DetachEvent detachEvent) {
        tulospalveluService.removeUpdateListener(this);
        super.onDetach(detachEvent);
    }

    @Override
    public void accept(fi.pirila.tulospalvelu.Competitor competitor) {
        ui().access(this::search);
    }

    private void search() {
        String term = searchField.getValue().trim();
        List<Competitor> competitors = term.isEmpty()
                ? competitorService.getAllCompetitors()
                : competitorService.searchCompetitors(term);
        competitorGrid.setItems(competitors);
    }

    private class CompetitorGrid extends VGrid<Competitor> {
        {
            addColumn(Competitor::getCompetitionNumber).setHeader("Bib").setSortable(true);
            addColumn(Competitor::getName).setHeader("Nimi").setSortable(true);
            addColumn(Competitor::getClub).setHeader("Seura").setSortable(true);
            addColumn(Competitor::getSarja).setHeader("Sarja").setSortable(true);
            addColumn(Competitor::getCardNumber).setHeader("Korttinro").setSortable(true);
            addColumn(Competitor::getResult).setHeader("Tulos")
                    .setComparator(Competitor::getResultOrder)
                    .setSortable(true);
            addComponentColumn(StatusActionsMenu::new).setHeader("Toiminnot").setAutoWidth(true).setFlexGrow(0);
            withColumnSelector();
            setSizeFull();
        }
    }

    private class StatusActionsMenu extends MenuBar {
        StatusActionsMenu(Competitor competitor) {
            addThemeVariants(MenuBarVariant.LUMO_TERTIARY_INLINE, MenuBarVariant.LUMO_SMALL);
            var root = addItem(new Icon(VaadinIcon.MENU));
            var sub = root.getSubMenu();
            sub.addItem("Merkitse: Ei lähtenyt (DNS)",
                    e -> confirmAndSend(competitor, TulospalveluProtocol.STATUS_DNS, "Ei lähtenyt"));
            sub.addItem("Merkitse: Keskeyttänyt (DNF)",
                    e -> confirmAndSend(competitor, TulospalveluProtocol.STATUS_DNF, "Keskeyttänyt"));
            sub.addItem("Merkitse: Hylätty (DSQ)",
                    e -> confirmAndSend(competitor, TulospalveluProtocol.STATUS_DSQ, "Hylätty"));
            sub.addItem("Tyhjennä tila",
                    e -> confirmAndSend(competitor, TulospalveluProtocol.STATUS_OPEN, "Avoinna"));
        }
    }

    private void confirmAndSend(Competitor competitor, char status, String statusLabel) {
        if (tulospalveluService.isPasswordRequired() && !userSession.isAuthenticated()) {
            Notification.show("Kirjaudu ensin etusivulla", 3000, Notification.Position.MIDDLE);
            ui().navigate(MainView.class);
            return;
        }

        new ConfirmDialog(competitor, status, statusLabel).open();
    }

    private class ConfirmDialog extends Dialog {
        ConfirmDialog(Competitor competitor, char status, String statusLabel) {
            setHeaderTitle("Vahvista tilamuutos");
            add(new Paragraph(competitor.getName() + " (#" + competitor.getCompetitionNumber()
                    + ") → " + statusLabel));

            var cancel = new Button("Peruuta", e -> close());
            var confirm = new Button("Lähetä", e -> {
                close();
                sendAsync(competitor, status, statusLabel);
            });
            confirm.addThemeVariants(ButtonVariant.LUMO_PRIMARY);
            getFooter().add(cancel, confirm);
        }
    }

    private void sendAsync(Competitor competitor, char status, String statusLabel) {
        var dialog = new Dialog();
        dialog.setHeaderTitle("Lähetetään...");
        dialog.setCloseOnEsc(false);
        dialog.setCloseOnOutsideClick(false);
        var progress = new ProgressBar();
        progress.setIndeterminate(true);
        dialog.add(progress, new Paragraph("Päivitetään tilaa: " + statusLabel));
        dialog.open();

        int recordIndex = competitor.getId().intValue();
        new Thread(() -> {
            boolean ok = tulospalveluService.sendStatusChange(recordIndex, status);
            ui().access(() -> {
                dialog.close();
                if (ok) {
                    Notification.show(competitor.getName() + ": " + statusLabel,
                            3000, Notification.Position.MIDDLE);
                    search();
                } else {
                    Notification.show("Tilamuutos epäonnistui",
                            5000, Notification.Position.MIDDLE);
                }
            });
        }).start();
    }
}
