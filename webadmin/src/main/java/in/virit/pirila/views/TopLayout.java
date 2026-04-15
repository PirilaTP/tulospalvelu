package in.virit.pirila.views;

import com.vaadin.flow.component.dependency.StyleSheet;
import com.vaadin.flow.theme.lumo.Lumo;
import org.vaadin.firitin.appframework.MainLayout;

public class TopLayout extends MainLayout {
    private final ConnectionStatusIndicator indicator;

    public TopLayout(ConnectionStatusIndicator connectionStatusIndicator) {
        this.indicator = connectionStatusIndicator;
    }

    @Override
    protected Object getDrawerHeader() {
        return "Pirilä}>";
    }

    @Override
    protected void addDrawerContent() {
        super.addDrawerContent();
        addToDrawer(indicator);
    }
}
