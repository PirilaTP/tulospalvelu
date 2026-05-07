package in.virit.pirila.views;

import com.vaadin.flow.component.AttachEvent;
import com.vaadin.flow.component.Composite;
import com.vaadin.flow.component.DetachEvent;
import com.vaadin.flow.component.UI;
import com.vaadin.flow.component.button.Button;
import com.vaadin.flow.component.button.ButtonVariant;
import com.vaadin.flow.component.checkbox.Checkbox;
import com.vaadin.flow.component.dialog.Dialog;
import com.vaadin.flow.component.html.Paragraph;
import com.vaadin.flow.component.icon.Icon;
import com.vaadin.flow.component.icon.VaadinIcon;
import com.vaadin.flow.component.masterdetaillayout.MasterDetailLayout;
import com.vaadin.flow.component.menubar.MenuBar;
import com.vaadin.flow.component.menubar.MenuBarVariant;
import com.vaadin.flow.component.notification.Notification;
import com.vaadin.flow.component.orderedlayout.HorizontalLayout;
import com.vaadin.flow.component.orderedlayout.VerticalLayout;
import com.vaadin.flow.component.progressbar.ProgressBar;
import com.vaadin.flow.component.textfield.TextField;
import com.vaadin.flow.data.value.ValueChangeMode;
import com.vaadin.flow.router.Route;
import fi.pirila.tulospalvelu.TulospalveluProtocol;
import in.virit.pirila.data.Competitor;
import in.virit.pirila.data.CompetitorEdit;
import in.virit.pirila.service.CompetitorService;
import in.virit.pirila.service.TulospalveluService;
import in.virit.pirila.service.UserSession;
import org.vaadin.firitin.appframework.MenuItem;
import org.vaadin.firitin.components.grid.VGrid;
import org.vaadin.firitin.components.textfield.VTextField;

import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.function.Consumer;

@Route(layout = TopLayout.class)
@MenuItem(icon = VaadinIcon.GRID)
public class CompetitorListView extends Composite<MasterDetailLayout>
        implements Consumer<fi.pirila.tulospalvelu.Competitor> {

    private final CompetitorService competitorService;
    private final TulospalveluService tulospalveluService;
    private final UserSession userSession;

    private final VTextField searchField = new VTextField() {{
        setPlaceholder("Hae kilpailijoita...");
        setClearButtonVisible(true);
        setValueChangeMode(ValueChangeMode.LAZY);
        setValueChangeTimeout(500);
        setPrefixComponent(new Icon("vaadin", "search"));
        setAutofocus(true);
    }};

    private final Checkbox showVakantit = new Checkbox("Näytä vakantit");

    private final VGrid<Competitor> competitorGrid = new CompetitorGrid();

    private final CompetitorEditForm editForm;
    private UI ui;
    private boolean focusNextOnSave;
    private Competitor editedRow;

    public CompetitorListView(CompetitorService competitorService,
                              TulospalveluService tulospalveluService,
                              UserSession userSession) {
        this.competitorService = competitorService;
        this.tulospalveluService = tulospalveluService;
        this.userSession = userSession;
        this.editForm = new CompetitorEditForm(tulospalveluService);

        searchField.addValueChangeListener(e -> search());
        showVakantit.addValueChangeListener(e -> search());
        competitorGrid.asSingleSelect().addValueChangeListener(e -> openInDetail(e.getValue()));

        editForm.setSavedHandler(this::handleSave);
        editForm.setResetHandler(edited -> getContent().setDetail(null));

        HorizontalLayout toolbar = new HorizontalLayout(searchField, showVakantit) {{
            setWidthFull();
            setDefaultVerticalComponentAlignment(Alignment.CENTER);
            expand(searchField);
        }};

        VerticalLayout master = new VerticalLayout(toolbar, competitorGrid) {{
            setSizeFull();
            setPadding(false);
            setSpacing(false);
            expand(competitorGrid);
        }};

        MasterDetailLayout root = getContent();
        root.setSizeFull();
        root.setMaster(master);
        root.setExpandMaster(true);
        search();
    }

    @Override
    protected void onAttach(AttachEvent attachEvent) {
        super.onAttach(attachEvent);
        this.ui = attachEvent.getUI();
        tulospalveluService.addUpdateListener(this);
    }

    @Override
    public void onDetach(DetachEvent detachEvent) {
        tulospalveluService.removeUpdateListener(this);
        super.onDetach(detachEvent);
    }

    /** Server-initiated update from another peer or the C++ master. */
    @Override
    public void accept(fi.pirila.tulospalvelu.Competitor competitor) {
        ui.access(this::search);
    }

    private void search() {
        String term = searchField.getValue() == null ? "" : searchField.getValue().trim();
        boolean withVacant = showVakantit.getValue() != null && showVakantit.getValue();
        List<Competitor> competitors = (term.isEmpty()
                ? competitorService.getAllCompetitors()
                : competitorService.searchCompetitors(term))
                .stream()
                .filter(c -> withVacant
                        || !tulospalveluService.isVacantClass(sarjaIndexOf(c)))
                .toList();
        competitorGrid.setItems(competitors);
    }

    private int sarjaIndexOf(Competitor c) {
        // Need backing kilpno → recordIndex → sarja from the in-memory model
        var backing = tulospalveluService.getCompetitorByRecordIndex(c.getId().intValue());
        return backing != null ? backing.sarja : -1;
    }

    private void openInDetail(Competitor c) {
        if (c == null) {
            getContent().setDetail(null);
            return;
        }
        editedRow = c;
        var backing = tulospalveluService.getCompetitorByRecordIndex(c.getId().intValue());
        if (backing == null) return;
        CompetitorEdit edit = new CompetitorEdit();
        edit.setRecordIndex(backing.recordIndex);
        edit.setKilpno(String.valueOf(backing.kilpno));
        edit.setEtunimi(backing.etunimi);
        edit.setSukunimi(backing.sukunimi);
        edit.setSeura(backing.seura);
        edit.setSeuralyh(backing.seuralyh);
        edit.setPiiri(backing.piiri);
        edit.setSarja(backing.sarja);
        edit.setCardNumber(backing.badge > 0 ? String.valueOf(backing.badge) : "");
        edit.setStartTime(backing.formatStartTime());
        editForm.setEntity(edit);
        getContent().setDetail(editForm);
    }

    private void handleSave(CompetitorEdit edit) {
        if (tulospalveluService.isPasswordRequired() && !userSession.isAuthenticated()) {
            Notification.show("Kirjaudu ensin etusivulla", 3000, Notification.Position.MIDDLE);
            ui.navigate(MainView.class);
            return;
        }
        var backing = tulospalveluService.getCompetitorByRecordIndex(edit.getRecordIndex());
        if (backing == null) return;

        // If the user picked a known seura from the catalogue, derive lyhenne
        // and piiri from the catalogue entry. For custom (free-typed) values
        // the catalogue lookup misses, so we leave lyhenne blank and piiri 0
        // — same convention C++ uses when no district is known.
        var seuratCatalogue = tulospalveluService.getAllSeuras();
        fi.pirila.tulospalvelu.Seura known = edit.getSeura() == null
                ? null : seuratCatalogue.get(edit.getSeura());
        String resolvedLyh = known != null ? known.lyhenne() : "";
        int resolvedPiiri = known != null ? known.piiri() : 0;
        edit.setSeuralyh(resolvedLyh);
        edit.setPiiri(resolvedPiiri);

        boolean recordChanged = !Objects.equals(edit.getEtunimi(), backing.etunimi)
                || !Objects.equals(edit.getSukunimi(), backing.sukunimi)
                || !Objects.equals(edit.getSeura(), backing.seura)
                || !Objects.equals(resolvedLyh, backing.seuralyh == null ? "" : backing.seuralyh)
                || resolvedPiiri != backing.piiri
                || !Objects.equals(edit.getSarja(), backing.sarja);
        Integer newBadge = parseBadge(edit.getCardNumber());
        boolean badgeChanged = newBadge != null && newBadge != backing.badge;
        Integer parsedStart = parseStartTime(edit.getStartTime());
        if (parsedStart == null) {
            // Pattern violation already shows on the field; don't send anything.
            Notification.show("Lähtöajan muoto virheellinen", 3000, Notification.Position.MIDDLE);
            return;
        }
        int newStartMs = parsedStart;
        boolean startTimeChanged = newStartMs != backing.startTime;

        // If we're promoting a vakantti slot ('V') to a real competitor by
        // entering name/seura/sarja data, clear the V flag automatically — the
        // user shouldn't have to do it through the status menu separately.
        boolean clearVakantti = recordChanged && backing.keskhyl == 'V';

        if (!recordChanged && !badgeChanged && !startTimeChanged) {
            Notification.show("Ei muutoksia", 2000, Notification.Position.MIDDLE);
            return;
        }
        sendAsync(edit, recordChanged, badgeChanged, newBadge,
                startTimeChanged, newStartMs, clearVakantti);
    }

    private static Integer parseBadge(String s) {
        if (s == null || s.isBlank()) return null;
        try { return Integer.parseInt(s.trim()); }
        catch (NumberFormatException e) { return null; }
    }

    /**
     * Parse "HH:MM" or "HH:MM:SS" → tlahto (ms from noon).
     * Empty/blank → TLAHTO_NOT_SET. Returns null on malformed input.
     */
    private static Integer parseStartTime(String s) {
        if (s == null || s.isBlank()) return fi.pirila.tulospalvelu.Competitor.TLAHTO_NOT_SET;
        String[] parts = s.trim().split(":");
        if (parts.length < 2 || parts.length > 3) return null;
        try {
            int h = Integer.parseInt(parts[0]);
            int m = Integer.parseInt(parts[1]);
            int sec = parts.length == 3 ? Integer.parseInt(parts[2]) : 0;
            if (h < 0 || h > 23 || m < 0 || m > 59 || sec < 0 || sec > 59) return null;
            int secOfDay = h * 3600 + m * 60 + sec;
            return secOfDay * 1000 - 12 * 3_600_000;
        } catch (NumberFormatException e) {
            return null;
        }
    }

    private void sendAsync(CompetitorEdit edit, boolean recordChanged,
                           boolean badgeChanged, Integer newBadge,
                           boolean startTimeChanged, int newStartMs,
                           boolean clearVakantti) {
        Dialog dialog = new Dialog();
        dialog.setHeaderTitle("Tallennetaan...");
        dialog.setCloseOnEsc(false);
        dialog.setCloseOnOutsideClick(false);
        ProgressBar progress = new ProgressBar(); progress.setIndeterminate(true);
        Paragraph status = new Paragraph("Lähetetään palvelimelle...");
        dialog.add(progress, status);
        dialog.open();

        new Thread(() -> {
            boolean ok = true;
            if (recordChanged) {
                ok = tulospalveluService.sendCompetitorEdit(edit.getRecordIndex(),
                        edit.getSukunimi(), edit.getEtunimi(), edit.getSeura(),
                        edit.getSeuralyh() == null ? "" : edit.getSeuralyh(),
                        edit.getPiiri() == null ? 0 : edit.getPiiri(),
                        edit.getSarja());
            }
            if (ok && clearVakantti) {
                ok = tulospalveluService.sendStatusChange(edit.getRecordIndex(),
                        fi.pirila.tulospalvelu.TulospalveluProtocol.STATUS_OPEN);
            }
            if (ok && badgeChanged) {
                ok = tulospalveluService.sendCardChange(edit.getRecordIndex(), newBadge);
            }
            if (ok && startTimeChanged) {
                ok = tulospalveluService.sendStartTimeChange(edit.getRecordIndex(), newStartMs);
            }
            boolean success = ok;
            ui.access(() -> {
                dialog.close();
                if (success) {
                    Notification.show("Tallennettu", 3000, Notification.Position.MIDDLE);
                    if(focusNextOnSave && editedRow != null) {
                        List<Competitor> competitorList = competitorGrid.getListDataView().getItems().toList();
                        int index = competitorList.indexOf(editedRow) + 1;
                        if(index < competitorList.size()) {
                            competitorGrid.select(competitorList.get(index));
                        }
                    } else {
                        getContent().setDetail(null);
                        competitorGrid.asSingleSelect().clear();
                        search();
                        searchField.selectAll();
                    }
                } else {
                    Notification.show("Tallennus epäonnistui", 5000, Notification.Position.MIDDLE);
                }
                focusNextOnSave = false;
            });
        }).start();
    }

    public void focusNextOnSave() {
        focusNextOnSave = true;
    }

    // --- Grid ---

    private class CompetitorGrid extends VGrid<Competitor> {
        {
            addColumn(Competitor::getCompetitionNumber).setHeader("Bib")
                    .setComparator(java.util.Comparator.comparingInt(c -> parseBibSafe(c.getCompetitionNumber())))
                    .setSortable(true).setAutoWidth(true).setFlexGrow(0);
            addColumn(Competitor::getName).setHeader("Nimi").setSortable(true);
            addColumn(Competitor::getClub).setHeader("Seura").setSortable(true);
            addColumn(Competitor::getSarja).setHeader("Sarja").setSortable(true).setAutoWidth(true).setFlexGrow(0);
            addColumn(Competitor::getCardNumber).setHeader("Korttinro").setSortable(true);
            addColumn(Competitor::getStartTime).setHeader("Lähtöaika")
                    .setComparator(Competitor::getStartTimeMs)
                    .setSortable(true).setAutoWidth(true).setFlexGrow(0);
            addColumn(Competitor::getResult).setHeader("Tulos")
                    .setComparator(Competitor::getResultOrder).setSortable(true);
            addComponentColumn(StatusActionsMenu::new).setHeader("Tila").setWidth("26px");
            withColumnSelector();
            setSizeFull();
        }
    }

    private static int parseBibSafe(String s) {
        if (s == null || s.isBlank()) return Integer.MAX_VALUE;
        try { return Integer.parseInt(s.trim()); }
        catch (NumberFormatException e) { return Integer.MAX_VALUE; }
    }

    // --- Status menu (DNS/DNF/DSQ) — preserved from earlier work ---

    private class StatusActionsMenu extends MenuBar {
        StatusActionsMenu(Competitor competitor) {
            addThemeVariants(MenuBarVariant.LUMO_TERTIARY_INLINE, MenuBarVariant.LUMO_SMALL);
            addItem("Merkitse: Ei lähtenyt (DNS)",
                    e -> confirmStatus(competitor, TulospalveluProtocol.STATUS_DNS, "Ei lähtenyt"));
            addItem("Merkitse: Keskeyttänyt (DNF)",
                    e -> confirmStatus(competitor, TulospalveluProtocol.STATUS_DNF, "Keskeyttänyt"));
            addItem("Merkitse: Hylätty (DSQ)",
                    e -> confirmStatus(competitor, TulospalveluProtocol.STATUS_DSQ, "Hylätty"));
            addItem("Tyhjennä tila",
                    e -> confirmStatus(competitor, TulospalveluProtocol.STATUS_OPEN, "Avoinna"));
        }
    }

    private void confirmStatus(Competitor competitor, char status, String statusLabel) {
        if (tulospalveluService.isPasswordRequired() && !userSession.isAuthenticated()) {
            Notification.show("Kirjaudu ensin etusivulla", 3000, Notification.Position.MIDDLE);
            ui.navigate(MainView.class);
            return;
        }
        Dialog confirm = new Dialog();
        confirm.setHeaderTitle("Vahvista tilamuutos");
        confirm.add(new Paragraph(competitor.getName() + " (#" + competitor.getCompetitionNumber()
                + ") → " + statusLabel));
        Button cancel = new Button("Peruuta", e -> confirm.close());
        Button ok = new Button("Lähetä", e -> {
            confirm.close();
            sendStatusAsync(competitor, status, statusLabel);
        });
        ok.addThemeVariants(ButtonVariant.LUMO_PRIMARY);
        confirm.getFooter().add(cancel, ok);
        confirm.open();
    }

    private void sendStatusAsync(Competitor competitor, char status, String statusLabel) {
        Dialog dialog = new Dialog();
        dialog.setHeaderTitle("Lähetetään...");
        dialog.setCloseOnEsc(false);
        dialog.setCloseOnOutsideClick(false);
        ProgressBar progress = new ProgressBar(); progress.setIndeterminate(true);
        dialog.add(progress, new Paragraph("Päivitetään tilaa: " + statusLabel));
        dialog.open();

        int recordIndex = competitor.getId().intValue();
        new Thread(() -> {
            boolean ok = tulospalveluService.sendStatusChange(recordIndex, status);
            ui.access(() -> {
                dialog.close();
                if (ok) {
                    Notification.show(competitor.getName() + ": " + statusLabel,
                            3000, Notification.Position.MIDDLE);
                    search();
                } else {
                    Notification.show("Tilamuutos epäonnistui", 5000, Notification.Position.MIDDLE);
                }
            });
        }).start();
    }
}
