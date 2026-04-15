package in.virit.pirila.views;

import com.vaadin.flow.component.html.H1;
import com.vaadin.flow.router.Route;
import org.vaadin.firitin.appframework.MenuItem;
import org.vaadin.firitin.components.orderedlayout.VVerticalLayout;

@Route(layout = TopLayout.class)
@MenuItem(order = MenuItem.BEGINNING)
public class MainView extends VVerticalLayout {
    public MainView(ConnectionStatusIndicator connectionStatusIndicator) {
        add(new H1("Pirilä web admin PoC"));

        add(connectionStatusIndicator);

    }
}
