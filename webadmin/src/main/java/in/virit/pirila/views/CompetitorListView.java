package in.virit.pirila.views;

import com.vaadin.flow.component.AttachEvent;
import com.vaadin.flow.component.DetachEvent;
import com.vaadin.flow.component.icon.Icon;
import com.vaadin.flow.component.icon.VaadinIcon;
import com.vaadin.flow.component.textfield.TextField;
import com.vaadin.flow.data.value.ValueChangeMode;
import com.vaadin.flow.router.Route;
import in.virit.pirila.data.Competitor;
import in.virit.pirila.service.CompetitorService;
import in.virit.pirila.service.TulospalveluService;
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

    private final TextField searchField = new TextField("Hae (numero/nimi/seura/sarja)") {{
        setPlaceholder("Hae kilpailijoita...");
        setClearButtonVisible(true);
        setWidthFull();
        setValueChangeMode(ValueChangeMode.LAZY);
        setValueChangeTimeout(500);
        setPrefixComponent(new Icon("vaadin", "search"));
        setAutofocus(true);
    }};

    private final VGrid<Competitor> competitorGrid = new VGrid<>() {{
        addColumn(Competitor::getCompetitionNumber).setHeader("Bib").setSortable(true);
        addColumn(Competitor::getName).setHeader("Nimi").setSortable(true);
        addColumn(Competitor::getClub).setHeader("Seura").setSortable(true);
        addColumn(Competitor::getSarja).setHeader("Sarja").setSortable(true);
        addColumn(Competitor::getCardNumber).setHeader("Korttinro").setSortable(true);
        addColumn(Competitor::getResult).setHeader("Tulos")
                .setComparator(Competitor::getResultOrder)
                .setSortable(true);
        withColumnSelector();
        setSizeFull();
    }};

    public CompetitorListView(CompetitorService competitorService,
                              TulospalveluService tulospalveluService) {
        this.competitorService = competitorService;
        this.tulospalveluService = tulospalveluService;

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
}
