package in.virit.pirila.views;

import com.vaadin.flow.component.grid.Grid;
import com.vaadin.flow.component.icon.Icon;
import com.vaadin.flow.component.icon.VaadinIcon;
import com.vaadin.flow.component.textfield.TextField;
import com.vaadin.flow.data.value.ValueChangeMode;
import com.vaadin.flow.router.Route;
import in.virit.pirila.data.Competitor;
import in.virit.pirila.service.CompetitorService;
import org.vaadin.firitin.appframework.MenuItem;
import org.vaadin.firitin.components.orderedlayout.VVerticalLayout;

import java.util.List;

@Route(layout = TopLayout.class)
@MenuItem(icon = VaadinIcon.GRID)
public class CompetitorListView extends VVerticalLayout {

    private final CompetitorService competitorService;

    private final TextField searchField = new TextField("Hae kilpailija (numero/nimi)") {{
        setPlaceholder("Hae kilpailijoita numerolla tai nimellä...");
        setClearButtonVisible(true);
        setWidthFull();
        setValueChangeMode(ValueChangeMode.LAZY);
        setValueChangeTimeout(500);
        setPrefixComponent(new Icon("vaadin", "search"));
        setAutofocus(true);
    }};

    private final Grid<Competitor> competitorGrid = new Grid<>() {{
        addColumn(Competitor::getCompetitionNumber).setHeader("Bib").setSortable(true);
        addColumn(Competitor::getName).setHeader("Nimi").setSortable(true);
        addColumn(Competitor::getClub).setHeader("Seura").setSortable(true);
        addColumn(Competitor::getCardNumber).setHeader("Korttinro").setSortable(true);
        setSizeFull();
    }};

    public CompetitorListView(CompetitorService competitorService) {
        this.competitorService = competitorService;

        searchField.addValueChangeListener(event -> search());
        competitorGrid.setItems(competitorService.getAllCompetitors());

        add(searchField, competitorGrid);
        setSizeFull();
    }

    private void search() {
        String term = searchField.getValue().trim();
        List<Competitor> competitors = term.isEmpty()
                ? competitorService.getAllCompetitors()
                : competitorService.searchCompetitors(term);
        competitorGrid.setItems(competitors);
    }
}
