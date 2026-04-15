package in.virit.pirila.views;

import com.vaadin.flow.component.html.Span;
import com.vaadin.flow.spring.annotation.SpringComponent;
import com.vaadin.flow.spring.annotation.UIScope;
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

    public void refresh() {
        boolean connected = tulospalveluService.isConnected();
        getStyle().setBackground(connected ? "#4CAF50" : "#F44336");
        getElement().setAttribute("title", connected ? "Yhdistetty" : "Ei yhteyttä");
        setText(connected ? "Yhdistetty" : "Ei yhteyttä");
    }
}
