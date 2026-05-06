package in.virit.pirila.views;

import com.vaadin.flow.component.Component;
import com.vaadin.flow.component.combobox.ComboBox;
import com.vaadin.flow.component.formlayout.FormLayout;
import com.vaadin.flow.component.html.Span;
import com.vaadin.flow.component.textfield.TextField;
import com.vaadin.flow.data.value.ValueChangeMode;
import in.virit.pirila.data.CompetitorEdit;
import in.virit.pirila.service.TulospalveluService;
import org.vaadin.firitin.form.BeanValidationForm;

import java.util.List;
import java.util.Map;

/**
 * BeanValidationForm subclass for editing a single competitor.
 * Field names match CompetitorEdit's property names so FormBinder auto-binds.
 */
public class CompetitorEditForm extends BeanValidationForm<CompetitorEdit> {

    private final Span kilpno = new Span();
    private final TextField etunimi = new TextField("Etunimi") {{
        setValueChangeMode(ValueChangeMode.LAZY);
    }};
    private final TextField sukunimi = new TextField("Sukunimi") {{
        setValueChangeMode(ValueChangeMode.LAZY);
    }};
    private final TextField seura = new TextField("Seura") {{
        setValueChangeMode(ValueChangeMode.LAZY);
    }};
    private final ComboBox<Integer> sarja = new ComboBox<>("Sarja");
    private final TextField cardNumber = new TextField("Kilpailukortti") {{
        setPlaceholder("emit-kortin numero");
        setValueChangeMode(ValueChangeMode.LAZY);
    }};

    public CompetitorEditForm(TulospalveluService service) {
        super(CompetitorEdit.class);

        Map<Integer, String> classes = service.getAllClasses();
        sarja.setItems(classes.keySet());
        sarja.setItemLabelGenerator(idx -> classes.getOrDefault(idx, String.valueOf(idx)));

        setSaveCaption("Tallenna");
        setCancelCaption("Peruuta");
    }

    @Override
    protected Component createContent() {
        kilpno.getElement().getStyle().setColor("var(--lumo-secondary-text-color)");
        FormLayout fl = new FormLayout();
        fl.add(new Span("Bib: "), kilpno);
        fl.add(etunimi);
        fl.add(sukunimi);
        fl.add(seura);
        fl.add(sarja);
        fl.add(cardNumber);
        fl.setColspan(kilpno, 2);
        return new com.vaadin.flow.component.orderedlayout.VerticalLayout(fl,
                getClassLevelViolationsDisplay(), getToolbar()) {{
            setPadding(false);
        }};
    }

    @Override
    protected List<Component> getFormComponents() {
        // Used by default createContent; we override createContent so this list
        // is what FormBinder sees for binding by-name.
        return List.of(etunimi, sukunimi, seura, sarja, cardNumber);
    }

    @Override
    public void setEntity(CompetitorEdit entity) {
        super.setEntity(entity);
        kilpno.setText(entity != null && entity.getKilpno() != null ? entity.getKilpno() : "");
    }
}
