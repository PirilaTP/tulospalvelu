package in.virit.pirila.views;

import com.vaadin.flow.component.AttachEvent;
import com.vaadin.flow.component.DetachEvent;
import com.vaadin.flow.component.UI;
import com.vaadin.flow.component.html.Span;
import com.vaadin.flow.component.notification.Notification;
import com.vaadin.flow.component.notification.NotificationVariant;
import com.vaadin.flow.spring.annotation.SpringComponent;
import in.virit.pirila.service.TulospalveluService;
import org.springframework.context.annotation.Scope;

import java.util.Objects;

@SpringComponent
@Scope("prototype")
public class ConnectionStatusIndicator extends Span {

    private final TulospalveluService tulospalveluService;
    /** Last warning we showed, so the 3s poll doesn't repeat the same notification. */
    private String lastShownWarning;

    public ConnectionStatusIndicator(TulospalveluService tulospalveluService) {
        this.tulospalveluService = tulospalveluService;
        getStyle()
                .setBorderRadius("0.5em")
                .setPadding("0.5em")
                .setDisplay(com.vaadin.flow.dom.Style.Display.INLINE_BLOCK);
        refresh();
    }

    @Override
    protected void onAttach(AttachEvent attachEvent) {
        super.onAttach(attachEvent);
        UI ui = attachEvent.getUI();
        ui.setPollInterval(3000);
        ui.addPollListener(e -> refresh());
    }

    @Override
    protected void onDetach(DetachEvent detachEvent) {
        getUI().ifPresent(ui -> ui.setPollInterval(-1));
        super.onDetach(detachEvent);
    }

    public void refresh() {
        if (!tulospalveluService.isStarted()) {
            getStyle().setBackground("#9E9E9E");
            setText("Ei käynnistetty");
            return;
        }
        boolean connected = tulospalveluService.isConnected();
        getStyle().setBackground(connected ? "#4CAF50" : "#F44336");
        setText(connected ? "Yhdistetty" : "Ei yhteyttä");
        maybeWarn();
    }

    /** Surface a peer-handshake mismatch (e.g. different competition day) once. */
    private void maybeWarn() {
        String warning = tulospalveluService.getConnectionWarning();
        if (Objects.equals(warning, lastShownWarning)) {
            return;
        }
        lastShownWarning = warning;
        if (warning != null) {
            Notification n = Notification.show(warning, 10000, Notification.Position.TOP_CENTER);
            n.addThemeVariants(NotificationVariant.LUMO_WARNING);
        }
    }
}
