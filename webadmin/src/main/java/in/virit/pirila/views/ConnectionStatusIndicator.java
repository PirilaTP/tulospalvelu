package in.virit.pirila.views;

import com.vaadin.flow.component.AttachEvent;
import com.vaadin.flow.component.DetachEvent;
import com.vaadin.flow.component.UI;
import com.vaadin.flow.component.html.Span;
import com.vaadin.flow.spring.annotation.SpringComponent;
import in.virit.pirila.service.TulospalveluService;
import org.springframework.context.annotation.Scope;

@SpringComponent
@Scope("prototype")
public class ConnectionStatusIndicator extends Span {

    private final TulospalveluService tulospalveluService;

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
    }
}
