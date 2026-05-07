package in.virit.pirila.views;

import com.vaadin.flow.component.AttachEvent;
import com.vaadin.flow.component.Component;
import com.vaadin.flow.component.DetachEvent;
import com.vaadin.flow.component.Key;
import com.vaadin.flow.component.KeyModifier;
import com.vaadin.flow.component.ShortcutRegistration;
import com.vaadin.flow.component.UI;
import com.vaadin.flow.component.combobox.ComboBox;
import com.vaadin.flow.component.formlayout.FormLayout;
import com.vaadin.flow.component.html.Emphasis;
import com.vaadin.flow.component.html.Span;
import com.vaadin.flow.component.textfield.TextField;
import in.virit.pirila.data.CompetitorEdit;
import in.virit.pirila.service.TulospalveluService;
import org.vaadin.firitin.components.textfield.VTextField;
import org.vaadin.firitin.form.BeanValidationForm;

import java.util.List;
import java.util.Map;

/**
 * BeanValidationForm subclass for editing a single competitor.
 * Field names match CompetitorEdit's property names so FormBinder auto-binds.
 *
 * Lähtöaika is a plain VTextField — Vaadin's TimePicker eats the change
 * unless another field is touched after it, so we parse "HH:MM[:SS]" by hand
 * in CompetitorListView.handleSave.
 */
public class CompetitorEditForm extends BeanValidationForm<CompetitorEdit> {

    private final Span kilpno = new Span();
    private final VTextField etunimi = new VTextField("Etunimi");
    private final TextField sukunimi = new VTextField("Sukunimi");
    private final ComboBox<String> seura = new ComboBox<>("Seura") {{
        setAllowCustomValue(true);
        setClearButtonVisible(true);
    }};
    private final ComboBox<Integer> sarja = new ComboBox<>("Sarja");
    private final TextField cardNumber = new VTextField("Kilpailukortti") {{
        setPlaceholder("emit-kortin numero");
    }};
    private final TextField startTime = new VTextField("Lähtöaika") {{
        setPlaceholder("HH:MM:SS");
        setHelperText("24h kellonaika, esim. 13:45:00");
    }};
    private ShortcutRegistration shiftClickReg;

    public CompetitorEditForm(TulospalveluService service) {
        super(CompetitorEdit.class);
        getStyle().setMinWidth("350px");

        Map<Integer, String> classes = service.getAllClasses();
        sarja.setItems(classes.keySet());
        sarja.setItemLabelGenerator(idx -> classes.getOrDefault(idx, String.valueOf(idx)));

        var seurat = service.getAllSeuras();
        // Mutable backing list so addCustomValueSetListener can append a
        // new entry — ComboBox.setValue silently drops values not in items.
        java.util.List<String> seuraItems = new java.util.ArrayList<>(seurat.keySet());
        seura.setItems(seuraItems);
        // Show "lyhenne — nimi" in the dropdown when we have a lyhenne, so
        // the typist can scan abbreviations alongside the full name.
        seura.setItemLabelGenerator(name -> {
            var s = seurat.get(name);
            return (s != null && !s.lyhenne().isBlank())
                    ? s.lyhenne() + " — " + name
                    : name;
        });
        // Custom entry: append to items, commit value, AND mirror into the
        // bound entity directly. Viritin's FormBinder reads form fields via
        // reflection at save time, but the round trip from ComboBox
        // setValue → ValueChangeEvent → entity bean is unreliable when the
        // user committed via Enter (no blur happened first), so we belt-and-
        // braces by writing entity.seura ourselves here.
        seura.addCustomValueSetListener(e -> {
            String typed = e.getDetail();
            if (typed == null || typed.isBlank()) return;
            if (!seuraItems.contains(typed)) {
                seuraItems.add(typed);
                seura.setItems(seuraItems);
            }
            seura.setValue(typed);
            if (getEntity() != null) {
                getEntity().setSeura(typed);
            }
        });

        setSaveCaption("Tallenna");
        setCancelCaption("Peruuta");
        getResetButton().addClickShortcut(Key.ESCAPE);

    }

    @Override
    protected Component createContent() {
        FormLayout fl = new FormLayout();
        fl.add(new Span("Bib: "), kilpno);
        fl.add(etunimi);
        fl.add(sukunimi);
        fl.add(seura);
        fl.add(sarja);
        fl.add(cardNumber);
        fl.add(startTime);
        fl.setColspan(kilpno, 2);
        return new com.vaadin.flow.component.orderedlayout.VerticalLayout(fl,
                getClassLevelViolationsDisplay(), getToolbar(), new Emphasis("Shift enter -> save and edit next."));
    }

    @Override
    protected List<Component> getFormComponents() {
        // Used by default createContent; we override createContent so this list
        // is what FormBinder sees for binding by-name.
        return List.of(etunimi, sukunimi, seura, sarja, cardNumber, startTime);
    }

    @Override
    public void setEntity(CompetitorEdit entity) {
        super.setEntity(entity);
        kilpno.setText(entity != null && entity.getKilpno() != null ? entity.getKilpno() : "");
        etunimi.selectAll();
    }

    @Override
    protected void onAttach(AttachEvent attachEvent) {
        super.onAttach(attachEvent);
        shiftClickReg = UI.getCurrent().addShortcutListener(() -> {
            findAncestor(CompetitorListView.class).focusNextOnSave();
            getSaveButton().focus();
            getSaveButton().getElement().executeJs("").then(a -> {
                getSaveButton().click();
            });
        }, Key.ENTER, KeyModifier.SHIFT);
    }

    @Override
    protected void onDetach(DetachEvent detachEvent) {
        shiftClickReg.remove();
        super.onDetach(detachEvent);
    }
}
