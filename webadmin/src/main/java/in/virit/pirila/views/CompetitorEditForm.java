package in.virit.pirila.views;

import com.vaadin.flow.component.Component;
import com.vaadin.flow.component.Key;
import com.vaadin.flow.component.combobox.ComboBox;
import com.vaadin.flow.component.formlayout.FormLayout;
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
    private final TextField seura = new VTextField("Seura");
    private final ComboBox<Integer> sarja = new ComboBox<>("Sarja");
    private final TextField cardNumber = new VTextField("Kilpailukortti") {{
        setPlaceholder("emit-kortin numero");
    }};
    private final TextField startTime = new VTextField("Lähtöaika") {{
        setPlaceholder("HH:MM:SS");
        setHelperText("24h kellonaika, esim. 13:45:00");
    }};

    public CompetitorEditForm(TulospalveluService service) {
        super(CompetitorEdit.class);

        Map<Integer, String> classes = service.getAllClasses();
        sarja.setItems(classes.keySet());
        sarja.setItemLabelGenerator(idx -> classes.getOrDefault(idx, String.valueOf(idx)));

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
                getClassLevelViolationsDisplay(), getToolbar());
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
}
