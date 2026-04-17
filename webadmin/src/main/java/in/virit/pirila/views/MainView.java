package in.virit.pirila.views;

import com.vaadin.flow.component.html.H1;
import com.vaadin.flow.component.html.Paragraph;
import com.vaadin.flow.component.html.Span;
import com.vaadin.flow.component.notification.Notification;
import com.vaadin.flow.component.textfield.PasswordField;
import com.vaadin.flow.data.value.ValueChangeMode;
import com.vaadin.flow.router.Route;
import in.virit.pirila.service.TulospalveluService;
import in.virit.pirila.service.UserSession;
import org.springframework.beans.factory.annotation.Value;
import org.vaadin.firitin.appframework.MenuItem;
import org.vaadin.firitin.components.button.DefaultButton;
import org.vaadin.firitin.components.orderedlayout.VVerticalLayout;
import org.vaadin.firitin.components.textfield.VTextField;

import java.nio.file.Files;
import java.nio.file.Path;

@Route(layout = TopLayout.class)
@MenuItem(order = MenuItem.BEGINNING)
public class MainView extends VVerticalLayout {

    public MainView(TulospalveluService tulospalveluService,
                    UserSession userSession,
                    @Value("${tulospalvelu.data-dir:}") String defaultDataDir) {

        if (tulospalveluService.isStarted()) {
            add(new H1("Pirilä web admin"));
            add(new ConnectionStatusIndicator(tulospalveluService));
            return;
        }

        add(new H1("Pirilä web admin — Asetukset"));

        var dataDir = new VTextField("Asetushakemisto (tyhjä -> ohjelman working dir)") {{
            setPlaceholder("Hakemisto jossa KILP.DAT ja laskenta.cfg");
            setValue(defaultDataDir != null && !defaultDataDir.isBlank() ? defaultDataDir : ".");
            setWidthFull();
            setValueChangeMode(ValueChangeMode.LAZY);
            setValueChangeTimeout(300);
        }};

        var dirStatus = new Span();
        dirStatus.getStyle()
                .setPadding("0.5em")
                .setBorderRadius("0.25em")
                .setFontSize("var(--lumo-font-size-s)");

        var password = new PasswordField("Salasana (valinnainen)") {{
            setPlaceholder("Tyhjä = ei vaadita salasanaa");
            setWidthFull();
            setClearButtonVisible(true);
        }};

        var info = new Paragraph("Syötä hakemistopolku jossa kilpailun datatiedostot sijaitsevat. "
                + "Polku voi olla suhteellinen (esim. '.') tai absoluuttinen. "
                + "Salasana vaaditaan myöhemmin korttien vaihdossa, jos se asetetaan.");
        info.getStyle().setColor("var(--lumo-secondary-text-color)");

        var startButton = new DefaultButton("Aloita", e -> {
            try {
                tulospalveluService.start(dataDir.getValue().trim(), password.getValue());
                userSession.setAuthenticated(true);
                getUI().ifPresent(ui -> ui.navigate(CardChangeView.class));
            } catch (Exception ex) {
                Notification.show("Käynnistys epäonnistui: " + ex.getMessage(),
                        5000, Notification.Position.MIDDLE);
            }
        });
        startButton.setWidthFull();

        dataDir.addValueChangeListener(e -> validateDataDir(e.getValue(), dirStatus, startButton));
        validateDataDir(dataDir.getValue(), dirStatus, startButton);

        add(dataDir, dirStatus, password, info, startButton);
    }

    private void validateDataDir(String value, Span status, DefaultButton startButton) {
        String dir = value != null ? value.trim() : "";
        if (dir.isEmpty()) dir = ".";

        Path path = Path.of(dir);
        boolean hasDir = Files.isDirectory(path);
        boolean hasKilp = hasDir && Files.exists(path.resolve("KILP.DAT"));
        boolean hasCfg = hasDir && Files.exists(path.resolve("laskenta.cfg"));

        if (!hasDir) {
            status.setText("Hakemistoa ei löydy: " + path.toAbsolutePath());
            status.getStyle().setBackground("var(--lumo-error-color-10pct)").setColor("var(--lumo-error-text-color)");
            startButton.setEnabled(false);
        } else if (!hasKilp) {
            status.setText("KILP.DAT puuttuu hakemistosta: " + path.toAbsolutePath());
            status.getStyle().setBackground("var(--lumo-error-color-10pct)").setColor("var(--lumo-error-text-color)");
            startButton.setEnabled(false);
        } else if (!hasCfg) {
            status.setText("KILP.DAT löytyi, mutta laskenta.cfg puuttuu (yhteys ei toimi)");
            status.getStyle().setBackground("var(--lumo-warning-color-10pct)").setColor("var(--lumo-warning-text-color)");
            startButton.setEnabled(true);
        } else {
            status.setText("OK — KILP.DAT ja laskenta.cfg löytyvät (" + path.toAbsolutePath() + ")");
            status.getStyle().setBackground("var(--lumo-success-color-10pct)").setColor("var(--lumo-success-text-color)");
            startButton.setEnabled(true);
        }
    }
}
