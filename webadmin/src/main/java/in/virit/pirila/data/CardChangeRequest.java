package in.virit.pirila.data;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;

public class CardChangeRequest {

    @NotBlank(message = "Syötä kilpailukortin numero")
    @CardNotInUse
    private String cardNumber;

    @NotNull(message = "Valitse kilpailija")
    private Competitor competitor;

    public String getCardNumber() {
        return cardNumber;
    }

    public void setCardNumber(String cardNumber) {
        this.cardNumber = cardNumber;
    }

    public Competitor getCompetitor() {
        return competitor;
    }

    public void setCompetitor(Competitor competitor) {
        this.competitor = competitor;
    }
}
