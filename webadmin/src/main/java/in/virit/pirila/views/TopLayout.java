package in.virit.pirila.views;

import com.vaadin.flow.component.dialog.Dialog;
import com.vaadin.flow.component.textfield.PasswordField;
import com.vaadin.flow.router.AfterNavigationEvent;
import com.vaadin.flow.router.AfterNavigationObserver;
import in.virit.pirila.service.TulospalveluService;
import in.virit.pirila.service.UserSession;
import org.vaadin.firitin.appframework.MainLayout;
import org.vaadin.firitin.components.button.DefaultButton;

public class TopLayout extends MainLayout implements AfterNavigationObserver {

    private final TulospalveluService tulospalveluService;
    private final UserSession userSession;
    private final ConnectionStatusIndicator indicator;

    public TopLayout(TulospalveluService tulospalveluService,
                     UserSession userSession,
                     ConnectionStatusIndicator connectionStatusIndicator) {
        this.tulospalveluService = tulospalveluService;
        this.userSession = userSession;
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

    @Override
    public void afterNavigation(AfterNavigationEvent event) {
        super.afterNavigation(event);
        if (tulospalveluService.isStarted()
                && tulospalveluService.isPasswordRequired()
                && !userSession.isAuthenticated()) {
            showPasswordDialog();
        }
    }

    private void showPasswordDialog() {
        var dialog = new Dialog();
        dialog.setHeaderTitle("Syötä salasana");
        dialog.setCloseOnEsc(false);
        dialog.setCloseOnOutsideClick(false);

        var pwField = new PasswordField();
        pwField.setWidthFull();
        pwField.setAutofocus(true);
        dialog.add(pwField);

        dialog.getFooter().add(new DefaultButton("Kirjaudu", ev -> {
            if (tulospalveluService.checkPassword(pwField.getValue())) {
                userSession.setAuthenticated(true);
                dialog.close();
            } else {
                pwField.setInvalid(true);
                pwField.setErrorMessage("Väärä salasana");
            }
        }));

        dialog.open();
    }
}
