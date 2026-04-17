package in.virit.pirila.data;

import in.virit.pirila.service.CompetitionCardService;
import jakarta.validation.ConstraintValidator;
import jakarta.validation.ConstraintValidatorContext;

public class CardNotInUseValidator implements ConstraintValidator<CardNotInUse, String> {

    private final CompetitionCardService cardService;

    public CardNotInUseValidator(CompetitionCardService cardService) {
        this.cardService = cardService;
    }

    @Override
    public boolean isValid(String cardNumber, ConstraintValidatorContext context) {
        if (cardNumber == null || cardNumber.isBlank() || "0".equals(cardNumber.trim())) {
            return true;
        }

        var holder = cardService.findCardHolder(cardNumber);
        if (holder.isPresent()) {
            context.disableDefaultConstraintViolation();
            context.buildConstraintViolationWithTemplate(
                            "Kortti on jo käytössä: " + holder.get())
                    .addConstraintViolation();
            return false;
        }
        return true;
    }
}
